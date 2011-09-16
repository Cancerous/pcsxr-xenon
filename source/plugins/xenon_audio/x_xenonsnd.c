#include "stdafx.h"
#define _IN_OSS
#include "externals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xenon_sound/sound.h>
#include <xenon_soc/xenon_power.h>
#include <ppc/atomic.h>
#include <byteswap.h>
#include <time/time.h>
#define MAX_UNPLAYED 32768

#define BUFFER_SIZE 65536
static char buffer[BUFFER_SIZE];
static double freq_ratio;
static double inv_freq_ratio;

static enum {
    BUFFER_SIZE_32_60 = 2112, BUFFER_SIZE_48_60 = 3200,
    BUFFER_SIZE_32_50 = 2560, BUFFER_SIZE_48_50 = 3840
} buffer_size = 1024;

static s16 prevLastSample[2] = {0, 0};

static void inline play_buffer(void) {
    int i;
    for (i = 0; i < buffer_size / 4; ++i) 
        ((int*) buffer)[i] = bswap_32(((int*) buffer)[i]);
    
    xenon_sound_submit(buffer, buffer_size);
}

/*
 * resamples pStereoSamples 
 * (taken from http://pcsx2.googlecode.com/svn/trunk/plugins/zerospu2/zerospu2.cpp)
 */
void ResampleLinear(s16* pStereoSamples, s32 oldsamples, s16* pNewSamples, s32 newsamples) {
    s32 newsampL, newsampR;
    s32 i;
    for (i = 0; i < newsamples; ++i) {
        s32 io = i * oldsamples;
        s32 old = io / newsamples;
        s32 rem = io - old * newsamples;

        old *= 2;
        //printf("%d %d\n",old,oldsamples);
        if (old == 0) {
            newsampL = prevLastSample[0] * (newsamples - rem) + pStereoSamples[0] * rem;
            newsampR = prevLastSample[1] * (newsamples - rem) + pStereoSamples[1] * rem;
        } else {
            newsampL = pStereoSamples[old - 2] * (newsamples - rem) + pStereoSamples[old] * rem;
            newsampR = pStereoSamples[old - 1] * (newsamples - rem) + pStereoSamples[old + 1] * rem;
        }
        pNewSamples[2 * i] = newsampL / newsamples;
        pNewSamples[2 * i + 1] = newsampR / newsamples;
    }
    prevLastSample[0] = pStereoSamples[oldsamples * 2 - 2];
    prevLastSample[1] = pStereoSamples[oldsamples * 2 - 1];
}

static void inline add_to_buffer(void* stream, unsigned int length) {
    unsigned int lengthLeft;
    unsigned int rlengthLeft;
    
    lengthLeft = length >> 2;
    rlengthLeft = ceil(lengthLeft * freq_ratio);

//    copy_to_buffer((int *)buffer, stream , rlengthLeft, lengthLeft);
    ResampleLinear((s16 *) stream, lengthLeft, (s16 *) buffer, rlengthLeft);
    
    buffer_size = rlengthLeft << 2;
    play_buffer();

}

/*
 * SETUP SOUND
 */
void SetupSound(void) {
    freq_ratio = 48000.0f / 44100.0f;
    inv_freq_ratio = 48000.0f / 44100.0f;
    //xenon_run_thread_task(2, &thread_stack[sizeof (thread_stack) - 0x100], thread_loop);
}

/*
 * REMOVE SOUND
 */
void RemoveSound(void) {
}

/*
 * GET BYTES BUFFERED
 */
unsigned long SoundGetBytesBuffered(void) {
    //return 0;
    return xenon_sound_get_unplayed();
}

/*
 * FEED SOUND DATA
 */
void SoundFeedStreamData(unsigned char* pSound, long lBytes) {
    //printf("SoundFeedStreamData %d\r\n",lBytes);
    if (lBytes < 0)
        return;
    add_to_buffer(pSound, lBytes);
}

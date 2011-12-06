#include "stdafx.h"
#include "record.h"
#include "externals.h"
#include <stdio.h>
#include <xetypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <xenon_sound/sound.h>
#include <xenon_soc/xenon_power.h>
#include <ppc/timebase.h>
#include <byteswap.h>
#include <time/time.h>

unsigned long LastWrite = 0xffffffff;
unsigned long LastPlay = 0;
unsigned int LastPlayTotal;

int output_channels = 2;
int output_samplesize = 4;


unsigned char mixer_playbuf[ SOUNDSIZE ];


#define MAX_UNPLAYED 32768
#define BUFFER_SIZE 65536

static char buffer[BUFFER_SIZE];
static double freq_ratio;

static int buffer_size = 1024;

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
    LastPlayTotal = 0;
    LastPlay = 0;
    LastWrite = 0xffffffff;
    
    freq_ratio = 48000.0f / 44100.0f;
}

/*
 * REMOVE SOUND
 */
void RemoveSound(void) {
    LastWrite = 0xffffffff;
}

/*
 * GET BYTES BUFFERED
 */
unsigned long SoundGetBytesBuffered(void) {
    int size = xenon_sound_get_unplayed();
    // edge boundaries
    if( LastWrite == 0xffffffff ) return 0;
    
    if( LastPlay < LastWrite ) size = LastWrite - LastPlay;
    else size = ( SOUNDSIZE - LastPlay ) + LastWrite;
    
    if( size > FULLMAX ) {
        size = 0;
    }
    
    return size;
}

/*
 * FEED SOUND DATA
 */
void SoundFeedStreamData(unsigned char* pSound, long lBytes) {
    if (lBytes < 0)
        return;
    add_to_buffer(pSound, lBytes);
}

#if 0
unsigned long timeGetTime() {
    /*
    struct timeval tv;
    gettimeofday(&tv, 0); // well, maybe there are better ways
    return tv.tv_sec * 100000 + tv.tv_usec / 10; // to do that, but at least it works
     */
    return mftb()/(PPC_TIMEBASE_FREQ/100000);
}
#endif


int SoundBufferReady()
{
    return 1;
}

int SoundGetSamplesBuffered()
{
	return SoundGetBytesBuffered() / output_samplesize;
}


void SoundPhantomPad()
{

}

void SoundRecordStreamData(unsigned char* pSound,long lBytes)
{

}


void ResetSound()
{
    // fast-forward lag?
    CDDAPlay  = CDDAStart;
    CDDAFeed  = CDDAStart;

    XAPlay  = XAStart;
    XAFeed  = XAStart;


    LastWrite = 0xffffffff;
}

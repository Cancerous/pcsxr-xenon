#include <ppc/timebase.h>

void ShowFPS(){
    static unsigned long lastTick = 0;
    static int frames=0,frame_id=0;
    unsigned long nowTick;
    frame_id++;
    frames++;
    nowTick = mftb() / (PPC_TIMEBASE_FREQ / 1000);
    if (lastTick + 1000 <= nowTick) {

        printf("PSX GPU %d fps\r\n", frames);

        frames = 0;
        lastTick = nowTick;
    }
}
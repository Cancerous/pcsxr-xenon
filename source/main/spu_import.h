unsigned

#include "gamecube_plugins.h"
/*
short CALLBACK PEOPS_SPUgetOne(unsigned long val);
void CALLBACK PEOPS_SPUputOne(unsigned long val, unsigned short data);
void CALLBACK PEOPS_SPUplaySample(unsigned char ch);
void CALLBACK PEOPS_SPUsetAddr(unsigned char ch, unsigned short waddr);
void CALLBACK PEOPS_SPUsetPitch(unsigned char ch, unsigned short pitch);
void CALLBACK PEOPS_SPUsetVolumeL(unsigned char ch, short vol);
void CALLBACK PEOPS_SPUsetVolumeR(unsigned char ch, short vol);
void CALLBACK PEOPS_SPUstartChannels1(unsigned short channels);
void CALLBACK PEOPS_SPUstartChannels2(unsigned short channels);
void CALLBACK PEOPS_SPUstopChannels1(unsigned short channels);
void CALLBACK PEOPS_SPUstopChannels2(unsigned short channels);
void CALLBACK PEOPS_SPUplaySector(unsigned long mode, unsigned char * p);
 */ 
//void CALLBACK PEOPS_SPUsetframelimit( int option );

/*
PluginTable spu_repair_110_plugin = {
    .lib = "/SPU",
    .numSyms = 19,
    {
        { "SPUconfigure",
            PEOPS_SPUsetConfigFile},
        { "SPUabout",
            PEOPS_SPUabout},
        { "SPUinit",
            PEOPS_SPUinit},
        { "SPUshutdown",
            PEOPS_SPUshutdown},
        { "SPUtest",
            PEOPS_SPUtest},
        { "SPUopen",
            PEOPS_SPUopen},
        { "SPUclose",
            PEOPS_SPUclose},
        { "SPUplayADPCMchannel",
            PEOPS_SPUplayADPCMchannel},
        { "SPUwriteRegister",
            PEOPS_SPUwriteRegister},
        { "SPUreadRegister",
            PEOPS_SPUreadRegister},
        { "SPUregisterCallback",
            PEOPS_SPUregisterCallback},
        { "SPUgetOne",
            PEOPS_SPUgetOne},
        { "SPUputOne",
            PEOPS_SPUputOne},
        { "SPUplaySample",
            PEOPS_SPUplaySample},
        { "SPUsetAddr",
            PEOPS_SPUsetAddr},
        { "SPUsetPitch",
            PEOPS_SPUsetPitch},
        { "SPUsetVolumeL",
            PEOPS_SPUsetVolumeL},
        { "SPUsetVolumeR",
            PEOPS_SPUsetVolumeR},
        { "SPUstartChannels1",
            PEOPS_SPUstartChannels1},
        { "SPUstartChannels2",
            PEOPS_SPUstartChannels2},
        { "SPUstopChannels1",
            PEOPS_SPUstopChannels1},
        { "SPUstopChannels2",
            PEOPS_SPUstopChannels2},
        { "SPUplaySector",
            PEOPS_SPUplaySector},
        { "SPUwriteDMA",
            PEOPS_SPUwriteDMA},
        { "SPUreadDMA",
            PEOPS_SPUreadDMA},
        { "SPUregisterCDDAVolume",
            PEOPS_SPUregisterCDDAVolume},
        { "SPUwriteDMAMem",
            PEOPS_SPUwriteDMAMem},
        { "SPUreadDMAMem",
            PEOPS_SPUreadDMAMem},
        { "SPUfreeze",
            PEOPS_SPUfreeze},
        { "SPUasync",
            PEOPS_SPUasync},
        { "SPUplayCDDAchannel",
            PEOPS_SPUplayCDDAchannel},
        { "SPUsetframelimit",
            PEOPS_SPUsetframelimit},
    }
}
 * */
#if 0
#define SPU_REPAIR110_PLUGIN \
    { "/SPU",      \
    19,         \
    { \
    { "SPUconfigure", \
        PEOPS_SPUsetConfigFile}, \
    { "SPUabout", \
        PEOPS_SPUabout}, \
    { "SPUinit",  \
        PEOPS_SPUinit }, \
    { "SPUshutdown",	\
        PEOPS_SPUshutdown}, \
    { "SPUtest", \
        PEOPS_SPUtest}, \
    { "SPUopen", \
        PEOPS_SPUopen}, \
    { "SPUclose", \
        PEOPS_SPUclose}, \
    { "SPUplayADPCMchannel", \
        PEOPS_SPUplayADPCMchannel}, \
    { "SPUwriteRegister", \
        PEOPS_SPUwriteRegister}, \
    { "SPUreadRegister", \
        PEOPS_SPUreadRegister}, \
    { "SPUregisterCallback", \
        PEOPS_SPUregisterCallback}, \
    { "SPUgetOne", \
        PEOPS_SPUgetOne}, \
    { "SPUputOne", \
        PEOPS_SPUputOne}, \
    { "SPUplaySample", \
        PEOPS_SPUplaySample}, \
    { "SPUsetAddr", \
        PEOPS_SPUsetAddr}, \
    { "SPUsetPitch", \
        PEOPS_SPUsetPitch}, \
    { "SPUsetVolumeL", \
        PEOPS_SPUsetVolumeL}, \
    { "SPUsetVolumeR", \
        PEOPS_SPUsetVolumeR}, \
    { "SPUstartChannels1", \
        PEOPS_SPUstartChannels1}, \
    { "SPUstartChannels2", \
        PEOPS_SPUstartChannels2}, \
    { "SPUstopChannels1", \
        PEOPS_SPUstopChannels1}, \
    { "SPUstopChannels2", \
        PEOPS_SPUstopChannels2}, \
    { "SPUplaySector", \
        PEOPS_SPUplaySector}, \
    { "SPUwriteDMA", \
        PEOPS_SPUwriteDMA}, \
    { "SPUreadDMA", \
        PEOPS_SPUreadDMA}, \
    { "SPUregisterCDDAVolume", \
        PEOPS_SPUregisterCDDAVolume}, \
    { "SPUwriteDMAMem", \
        PEOPS_SPUwriteDMAMem}, \
    { "SPUreadDMAMem", \
        PEOPS_SPUreadDMAMem}, \
    { "SPUfreeze", \
        PEOPS_SPUfreeze}, \
    { "SPUasync", \
        PEOPS_SPUasync}, \
    { "SPUplayCDDAchannel", \
        PEOPS_SPUplayCDDAchannel}, \
    { "SPUsetframelimit", \
        PEOPS_SPUsetframelimit}, \
       }\
 }

#endif
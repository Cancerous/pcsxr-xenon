#include "config.h"

#include "psxcommon.h"
#include "debug.h"
#include "sio.h"
#include "misc.h"
#include "cheat.h"
#include <sys/time.h>
#include <time/time.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// #define cdfile "uda:/psxisos/BREATH_OF_FIRE3.BIN"
#define cdfile "uda:/psxisos/MEGAMAN8.BIN"
// #define cdfile "uda:/psxisos/RPG Maker (U).bin"
#define cdfile "uda:/psxisos/Vagrant Story[NTSC-U].iso"
// #define cdfile "uda:/psx.iso"
// #define cdfile "uda:/psxisos/Super Street Fighter Collection.bin"
#define cdfile "uda:/psxisos/Strider 2 (USA)/Strider 2 (USA).bin"
#define cdfile "uda:/psxisos/Street Fighter Alpha - Warriors' Dreams (USA)/Street Fighter Alpha - Warriors' Dreams (USA) (Track 01).bin"
// #define cdfile "uda:/psxisos/Street Fighter Alpha 2 (USA)/Street Fighter Alpha 2 (USA) (Track 1).bin"
#define cdfile "uda:/psxisos/odd.iso"
#define cdfile "uda:/psxisos/PSX_Tekken_3_(NTSC).Iso"
#define cdfile "uda:/psxisos/Final Fantasy VII (USA) (Disc 1).bin"
#define cdfile "uda:/psxisos/MEGAMAN8.BIN"

void WriteSocket(char * buffer, int len) {

}

// 22 nov 2005 
#define	RTC_BASE	1132614024UL//1005782400UL
 
int gettimeofday(struct timeval * tp, void * tzp){
    unsigned char msg[16] = { 0x04 };
    unsigned long msec;
    unsigned long sec;
    
    xenon_smc_send_message(msg);
    xenon_smc_receive_message(msg);   
    
    msec = msg[1] | (msg[2] << 8) | (msg[3] << 16) |(msg[4] << 24) | ((unsigned long)msg[5] << 32);
    
    printf("smc ret : %d\r\n",msec);
    
    sec = msec/1000;
  
    tp->tv_sec  = sec + RTC_BASE;   
    
    msec -= sec * 1000;
    
    tp->tv_usec  = msec*1000;

    printf("s:%d - %ms:%d\r\n",tp->tv_sec,tp->tv_usec);
    
    return 0;
}

void usleep(int s){
    udelay(s);
}

void main() {
    xenos_init(VIDEO_MODE_AUTO);
    //console_init();
    xenon_make_it_faster(XENON_SPEED_FULL);
    usb_init();
    usb_do_poll();
    xenon_ata_init();
    dvd_init();
/*    
    struct timeval tt;
    gettimeofday(&tt,0);
    printf("s:%d - %ms:%d\r\n",tt.tv_sec,tt.tv_usec);
*/    
    memset(&Config, 0, sizeof (PcsxConfig));
    strcpy(Config.Net, "Disabled");
    strcpy(Config.Cdr,"CDR");
    strcpy(Config.Gpu,"GPU");
    strcpy(Config.Spu,"SPU");
    strcpy(Config.Pad1,"PAD1");
    strcpy(Config.Pad2,"PAD2");
    
    //strcpy(Config.Bios, "scph7502.bin");
    strcpy(Config.Bios, "SCPH1001.BIN"); // Use actual BIOS
    //strcpy(Config.Bios, "HLE"); // Use HLE
    strcpy(Config.BiosDir, "uda:/bios");
    
    Config.HLE = 0;
    Config.Xa = 0;  //XA enabled
    Config.Cdda = 1;
    Config.PsxAuto = 1; //Autodetect
    Config.Cpu = CPU_DYNAREC;
    Config.SlowBoot = 0; // Active bios
    
    // write not implemented ....
    strcpy(Config.Mcd1,"uda:/memcards/card1.mcd");
    strcpy(Config.Mcd2,"uda:/memcards/card2.mcd");
    
    SysPrintf("Init ...");
    
    SetIsoFile(cdfile);
    LoadPlugins();
    
    if (SysInit() == -1) {
        printf("SysInit() Error!\n");
        return 0;
    }

    GPU_clearDynarec(clearDynarec);

    int ret = CDR_open();
    if (ret < 0) {
        SysMessage(_("Error Opening CDR Plugin"));
        return -1;
    }
    ret = GPU_open(NULL,NULL,NULL);
    if (ret < 0) {
        SysMessage(_("Error Opening GPU Plugin (%d)"), ret);
        return -1;
    }
    ret = SPU_open();
    if (ret < 0) {
        SysMessage(_("Error Opening SPU Plugin (%d)"), ret);
        return -1;
    }
    ret = PAD1_open(1);
    if (ret < 0) {
        SysMessage(_("Error Opening PAD1 Plugin (%d)"), ret);
        return -1;
    }

    CDR_init();
    GPU_init();
    SPU_init();
    PAD1_init(1);
    PAD2_init(2);

    SysReset();

    SysPrintf("CheckCdrom\r\n");
    ret = CheckCdrom();
    if (ret!=0)
        SysPrintf("CheckCdrom: %08x\r\n", ret);
    ret = LoadCdrom();
    if (ret!=0)
        SysPrintf("LoadCdrom: %08x\r\n", ret);

    SysPrintf("Execute\r\n");

    psxCpu->Execute();

}
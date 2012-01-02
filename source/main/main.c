#include "config.h"
#include "r3000a.h"
#include "psxcommon.h"
#include "debug.h"
#include "sio.h"
#include "misc.h"
//#include "cheat.h"
#include <stdio.h>
#include <console/console.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gamecube_plugins.h"

extern void httpd_start(void);

extern PluginTable plugins[];

#ifndef LZX_GUI
//#define cdfile "sda:/psx/psxiso.iso"
//#define cdfile "uda:/psx/psxiso.iso"
#include "testing.h"
#endif

void printConfigInfo() {

}

#ifdef PCSXDF
enum {
	CPU_DYNAREC = 0,
	CPU_INTERPRETER
};
#endif


#ifdef LZX_GUI
int GetXaGui();
int GetSpuIrqGui();
const char * GetBiosGui();
int GetSlowbootGui();
int GetCpuGui();
#endif

void buffer_dump(uint8_t * buf, int size){
    int i = 0;
    TR;
    for(i=0;i<size;i++){
        
        printf("%02x ",buf[i]);
    }
    printf("\r\n");
}

uint8_t * xtaf_buff();

void SetIso(const char * fname){
    FILE *fd = fopen(fname,"rb");
    if(fd==NULL){
        printf("Error loading %s\r\n",fname);
        return;
    }
    uint8_t header[0x10];
    int n = fread(header,0x10,1,fd);
    printf("n : %d\r\n",n);

    buffer_dump(header,0x10);
    
    if(header[0]==0x78 && header[1]==0xDA){
        printf("Use CDRCIMG for  %s\r\n",fname);
        strcpy(Config.Cdr, "CDRCIMG");
        cdrcimg_set_fname(fname);
    }
    else{
        SetIsoFile(fname);
    }
    
    fclose(fd);
}

#ifndef LZX_GUI
int main() {
    xenos_init(VIDEO_MODE_AUTO);
    xenon_make_it_faster(XENON_SPEED_FULL);
    
    xenon_sound_init();
    //xenos_init(VIDEO_MODE_YUV_720P);
    //console_init();
    usb_init();
    usb_do_poll();
    
/*
xenon_ata_init();
xenon_atapi_init();
*/
    memset(&Config, 0, sizeof (PcsxConfig));
    
#else
int pcsxmain(const char * cdfile) {
    static int initialised = 0;
    if(initialised==0){
        xenon_make_it_faster(XENON_SPEED_FULL);
        xenon_sound_init();
        initialised++;
    }
    Xe_SetClearColor(getLzxVideoDevice(),0xff000000);
#endif
    
    //network_init();
    //network_print_config();
    
    //console_close();
    
    xenon_smc_start_bootanim(); // tell me that telnet or http are ready
    
    // telnet_console_init();
    // mdelay(5000);
    
    //httpd_start();
    
    // uart speed patch 115200 - jtag/freeboot
    // *(volatile uint32_t*)(0xea001000+0x1c) = 0xe6010000;

    //memset(&Config, 0, sizeof (PcsxConfig));
    strcpy(Config.Net, "Disabled");
    strcpy(Config.Cdr, "CDR");
    strcpy(Config.Gpu, "GPU");
    strcpy(Config.Spu, "SPU");
    strcpy(Config.Pad1, "PAD1");
    strcpy(Config.Pad2, "PAD2");

    //strcpy(Config.Bios, "SCPH1001.BIN"); // Use actual BIOS
    //strcpy(Config.Bios, "HLE"); // Use HLE
    strcpy(Config.BiosDir, "uda:/pcsxr/bios");
/*
    strcpy(Config.BiosDir, "sda:/hdd1/xenon/bios");
*/

    strcpy(Config.Bios, "scph7502.bin");
    Config.PsxOut = 0; // Enable Console Output 
    Config.SpuIrq = 0; // Spu Irq Always Enabled
    //Config.HLE = 0; 
    Config.Xa = 0; // Disable Xa Decoding
    Config.Cdda = 0; // Disable Cd audio
    Config.PsxAuto = 1; // autodetect system
    //Config.PsxType = PSX_TYPE_NTSC;
    Config.Cpu = CPU_DYNAREC;
    
#ifdef LZX_GUI
    if(useGpuSoft()){
        PluginTable softGpu = GPU_PEOPS_PLUGIN;
        plugins[5] = softGpu;
    }
#endif
    
    strcpy(Config.Mcd1, "uda:/pcsxr/memcards/card1.mcd");
    strcpy(Config.Mcd2, "uda:/pcsxr/memcards/card2.mcd");

/*
    strcpy(Config.Mcd1, "sda:/hdd1/xenon/memcards/card1.mcd");
    strcpy(Config.Mcd2, "sda:/hdd1/xenon/memcards/card2.mcd");
*/

    SetIso(cdfile);
    if (LoadPlugins() == 0) {
        if (OpenPlugins() == 0) {
            if (SysInit() == -1) {
                printf("SysInit() Error!\n");
                return -1;
            }

            SysReset();
            // Check for hle ...
            if(Config.HLE ==1){
                printf("Can't continue ... bios not found ...\r\n");
#ifdef LZX_GUI
                void GuiAlert(const char *message);
                GuiAlert("Can't continue ... bios not found ...");
#endif                
                return -1;
            }

            int ret = CheckCdrom();
            if (CheckCdrom() != 0)
            {
#ifdef LZX_GUI
                void GuiAlert(const char *message);
                GuiAlert("Can't continue ... invalide cd-image detected ...");
                return -1;
#endif                  
            }
            ret = LoadCdrom();
            if (ret != 0)
            {
#ifdef LZX_GUI
                void GuiAlert(const char *message);
                GuiAlert("Can't continue ... no executable found ...");
                return -1;
#endif                  
            }
#ifndef LZX_GUI
             //console_close();
#endif
            psxCpu->Execute();
        }
    }

    printf("Pcsx exit ...\r\n");
    return 0;
}

void cpuReset() {
    EmuReset();
}


void systemPoll(){
    // network_poll();
}

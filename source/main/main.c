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
#define cdfile "uda:/psxisos/MEGAMAN8.BIN"
#define cdfile "uda:/psxisos/PSX_Tekken_3_(NTSC).Iso"

//#define cdfile "uda:/psxisos/Street Fighter Alpha - Warriors' Dreams (USA)/Street Fighter Alpha - Warriors' Dreams (USA) (Track 01).bin"
//#define cdfile "uda:/psxisos/Soul Blade (USA) (v1.1).bin"


//#define cdfile "uda:/psxisos/Strider 2 (USA)/Strider 2 (USA).bin"
//#define cdfile "uda:/psxisos/CTR - Crash Team Racing (USA).bin"
//#define cdfile "uda:/psxisos/Strider 2 (USA)/Strider 2 (USA).bin"
//#define cdfile "uda:/Tekken 3 (USA)/Tekken 3 (USA) (Track 1).bin"

/*
#define cdfile "uda:/psxisos/Strider 2 (USA)/Strider 2 (USA).bin"

#define cdfile "uda:/psxisos/Tekken 3 (USA)/Tekken 3 (USA) (Track 1).bin"

#define cdfile "uda:/psxisos/Soul Blade (USA) (v1.1).bin"
#define cdfile "uda:/psxisos/Final Fantasy VII (USA)/Final Fantasy VII (USA) (Disc 1).bin"


#define cdfile "uda:/psxisos/Crash Bandicoot - Warped (USA).bin"
#define cdfile  "uda:/psxisos/Final Fantasy VII (F)/Final Fantasy VII (F) (Disc 1) [SCES-00868].bin"

#define cdfile "uda:/psxisos/Tekken 3 (USA)/Tekken 3 (USA) (Track 1).bin"
 */
/*
#define cdfile "uda:/psxisos/CTR - Crash Team Racing (USA).bin"
 */
//#define cdfile "uda:/psxisos/Tekken 3 (USA)/Tekken 3 (USA) (Track 1).bin"
//#define cdfile "uda:/psxisos/CTR - Crash Team Racing (USA).bin"
#define cdfile "uda:/psxisos/Final Fantasy VII (USA)/Final Fantasy VII (USA) (Disc 1).bin"
//#define cdfile "uda:/psxisos/Resident Evil - Director's Cut - Dual Shock Ver. (USA)/Resident Evil - Director's Cut - Dual Shock Ver. (USA) (Track 1).bin"
//#define cdfile "uda:/psxiso.iso"
//#define cdfile "uda:/psxisos/Gran Turismo 2 (USA) (v1.0) (Simulation Mode)/Gran Turismo 2 (USA) (v1.0) (Simulation Mode).bin"

#define cdfile "uda:/psxisos/Street Fighter Alpha 2 (USA)/Street Fighter Alpha 2 (USA) (Track 1).bin"
//#define cdfile "uda:/psxiso.iso"
//#define cdfile "uda:/psxisos/Tekken 3 (USA)/Tekken 3 (USA) (Track 1).bin"
//#define cdfile  "uda:/psxisos/Final Fantasy VII (F)/Final Fantasy VII (F) (Disc 1) [SCES-00868].bin"
//#define cdfile "uda:/psxisos/CTR - Crash Team Racing (USA).bin"
//#define cdfile "uda:/psxisos/Crash Bandicoot - Warped (USA).bin"
//#define cdfile "uda:/psxisos/WipEout XL (USA)/WipEout XL (USA) (Track 01).bin"
//#define cdfile "uda:/psxisos/Tekken 3 (USA)/Tekken 3 (USA) (Track 1).bin"
//#define cdfile "uda:/psxiso.iso"
//#define cdfile "uda:/psxisos/Rayman (USA)/Rayman (USA) (Track 01).bin"
//#define cdfile "uda:/psxisos/Darkstalkers 3 (USA)/Darkstalkers 3 (USA).bin"
//#define cdfile "uda:/psxisos/Gran Turismo 2 (USA) (v1.0) (Simulation Mode)/Gran Turismo 2 (USA) (v1.0) (Simulation Mode).bin"

//#define cdfile "uda:/psxisos/Street Fighter Alpha - Warriors' Dreams (USA)/Street Fighter Alpha - Warriors' Dreams (USA) (Track 01).bin"
//#define cdfile "uda:/psxisos/Tekken 3 (USA)/Tekken 3 (USA) (Track 1).bin"


//#define cdfile "uda:/psxisos/CTR - Crash Team Racing (USA).bin"
//#define cdfile "uda:/psxisos/R4 - Ridge Racer Type 4 (USA)/R4 - Ridge Racer Type 4 (USA).bin"

// #define cdfile "uda:/psxisos/Metal Gear Solid (France) (Disc 1)/Metal Gear Solid (France) (Disc 1).bin"
//#define cdfile "uda:/psxisos/Gran Turismo 2 (USA) (v1.0) (Simulation Mode)/Gran Turismo 2 (USA) (v1.0) (Simulation Mode).bin"
//#define cdfile "sda:/hdd1/xenon/psx/CTR - Crash Team Racing (USA)/CTR - Crash Team Racing (USA).bin.Z"
//#define cdfile "sda:/hdd1/xenon/psx/Tekken 3 (USA)/Tekken 3 (USA) (Track 1).bin.Z"
//#define cdfile "sda:/hdd1/xenon/psx/R4 - Ridge Racer Type 4 (USA).bin"
#define cdfile "uda:/CTR - Crash Team Racing (USA).bin"
//#define cdfile "uda:/Street Fighter Alpha - Warriors' Dreams (USA) (Track 01).bin"
//#define cdfile "uda:/Tekken 3 (USA) (Track 1).bin.Z"
//
//#define cdfile "uda:/R4 - Ridge Racer Type 4 (USA).bin.Z"
#define cdfile "uda:/Final Fantasy VII (USA) (Disc 1).bin"

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

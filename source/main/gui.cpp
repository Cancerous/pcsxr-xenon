// lzx.cpp : Defines the entry point for the application.
//

//#ifdef LZX_GUI
#if 1

extern "C" {
    int pcsxmain(const char * cdfile);
    #include "psxcommon.h"
}
#include <iostream>
#include <string>
#include <zlx/Browser.h>
#include <zlx/Hw.h>
#include <xenon_smc/xenon_smc.h>

using namespace ZLX;

Browser App;

//--------------------------------------------------------------------------
//  SMC
//--------------------------------------------------------------------------

enum SMC_COMMAND {
    //GET INFO
    SMC_COMMAND_POWER_ON_TYPE = 0x01,
    SMC_COMMAND_REAL_TIME_CLOCK = 0x04,
    SMC_COMMAND_REQUEST_TEMPERATURE = 0x07,
    SMC_COMMAND_REQUEST_TRAY_STATE = 0x0a,
    SMC_COMMAND_REQUEST_AV_PACK = 0x0f,
    SMC_COMMAND_REQUEST_ANA = 0x11,
    SMC_COMMAND_REQUEST_SMC_VERSION = 0x12,
    SMC_COMMAND_QUERY_IR_ACCESS = 0x16,
    SMC_COMMAND_REQUEST_TILT = 0x17,
    //SET
    SMC_COMMAND_SET_POWER = 0x82,
    SMC_COMMAND_SET_REAL_TIME_CLOCK = 0x85,
    SMC_COMMAND_SET_CD_TRAY = 0x8b,
    SMC_COMMAND_SET_POWER_LED = 0x8c,
    //EVENT
    SMC_COMMAND_EVENT = 0x83
};

enum SMC_POWER_EVENT {
    SMC_POWER_EVENT_POWER_BUTTON = 0x11,
    SMC_POWER_EVENT_EJECT_BUTTON = 0x12,
    SMC_POWER_EVENT_RTC_WAKEUP = 0x15,
    SMC_POWER_EVENT_POWER_IR = 0x20,
    SMC_POWER_EVENT_EJECT_IR = 0x21,
    SMC_POWER_EVENT_XENON_IR = 0x22,
    SMC_POWER_EVENT_MCE_IR = 0x24,
    SMC_POWER_EVENT_POWER_CYCLE = 0x30,
    SMC_POWER_EVENT_RESET_CYCLE = 0x31,
    SMC_POWER_EVENT_UNK = 0x41,
    SMC_POWER_EVENT_KIOSK = 0x51,
    SMC_POWER_EVENT_ARGON = 0x55,
    SMC_POWER_EVENT_GAMEPORT_1 = 0x56,
    SMC_POWER_EVENT_GAMEPORT_2 = 0x57,
    SMC_POWER_EVENT_EXPANSION = 0x5a,
};

enum SMC_TRAY {
    SMC_TRAY_OPEN = 0x60,
    SMC_TRAY_OPENING = 0x61,
    SMC_TRAY_CLOSE = 0x62,
};


//--------------------------------------------------------------------------
//  Cd tray
//--------------------------------------------------------------------------
void openTray(){
unsigned char msg[16] = {SMC_COMMAND_SET_CD_TRAY, SMC_TRAY_OPEN};
xenon_smc_send_message(msg);
}

void closeTray(){
unsigned char msg[16] = {SMC_COMMAND_SET_CD_TRAY, SMC_TRAY_CLOSE};
xenon_smc_send_message(msg);
}


//--------------------------------------------------------------------------
//  Gui action
//--------------------------------------------------------------------------
lpBrowserActionEntry xa; // enable disable
lpBrowserActionEntry slowboot; // enable disable
lpBrowserActionEntry spuirq; // enable disable
lpBrowserActionEntry cdda; // enable disable

lpBrowserActionEntry bios; // scph7502.bin ...
lpBrowserActionEntry region; // auto pal ntsc


lpBrowserActionEntry gpuplugin; // soft hw
lpBrowserActionEntry spuplugin; // dfaudio shalma
lpBrowserActionEntry inputplugin; // dfaudio shalma

static int gpuplugins = 0;
static int spuplugins = 0;
static int inputplugins = 0;

static int use_soft_gpu_plugin = 0;

extern "C" int useGpuSoft(){
    return use_soft_gpu_plugin;
}

void setGpuPluginName(){
    if(use_soft_gpu_plugin)
        gpuplugin->name = "Soft Gpu plugin";
    else
        gpuplugin->name = "HW Gpu plugin (W.I.P)";
}

void gpu_action(void*p){
    use_soft_gpu_plugin=!use_soft_gpu_plugin;
    setGpuPluginName();
}


//xa
void setXaName(){
    if(Config.Xa)
        xa->name="Xa disabled";
    else
        xa->name="Xa enabled";
}
void xa_action(void* param){
    Config.Xa = !Config.Xa;
    setXaName();
}

// cdda
void setCdName(){
    if(Config.Cdda)
        cdda->name="Cdda disabled";
    else
        cdda->name="Cdda enabled";
}
void cdda_action(void*p){
    Config.Cdda = !Config.Cdda;
    setCdName();
}

// slowboot
void setSbName(){
    if(Config.SlowBoot)
        slowboot->name="Slowboot enabled";
    else
        slowboot->name="Slowboot disabled";
}
void slowboot_action(void*p){
    Config.SlowBoot = !Config.SlowBoot;
    setSbName();
}

// spuirq
void setSIRQName(){
    if(Config.SpuIrq)
        spuirq->name="Spu Irq enabled";
    else
        spuirq->name="Spu Irq disabled";
}
void spuirq_action(void*p){
    Config.SpuIrq = !Config.SpuIrq;
    setSIRQName();
}
// bios
static std::string bios_name[]={
    "scph1001.bin",
    "scph7502.bin" ,
    "HLE (not working)"
};

static int sbios = 0;

void setBios(){
    sbios++;
    
    if(sbios==3)
        sbios=0;
    
    switch(sbios){
        case 0:
            strcpy(Config.Bios,"scph1001.bin");
            Config.HLE=0;
            bios->name = "Bios: scph1001";
            break;

        case 1:
            strcpy(Config.Bios,"scph7502.bin");
            bios->name = "Bios: scph7502";
            Config.HLE=0;
            break;

        case 2:
            strcpy(Config.Bios,"");
            bios->name = "Bios: HLE (not working)";
            Config.HLE=1;
            break;
    }
}

void bios_action(void*p){
    setBios();
}

// Region
static int region_s=0;

void setRegion(){
    region_s++;
    if(region_s==3)
        region_s=0;
    
    if(region_s==0) // region auto..
    {
        Config.PsxAuto = 1;
    }
    else
    {
        Config.PsxAuto = 0;
        Config.PsxType = (region_s==1)?PSX_TYPE_NTSC:PSX_TYPE_PAL;
    }
    switch(region_s){
        case 0:
            region->name = "Region: Auto";
            break;
            
        case 1:
            region->name ="Region: Ntsc";
            break;
        case 2:
            region->name = "Region: Pal";
            break;
    }
}

void region_action(void*p){
    setRegion();
}

void InitActionEntry(){
    memset(&Config, 0, sizeof (PcsxConfig));
    {
        gpuplugin = new BrowserActionEntry;
        gpuplugin->action = gpu_action;
        setGpuPluginName();
        App.AddAction(gpuplugin);
    }
    
    {
        xa = new BrowserActionEntry;
        xa->action = xa_action;
        setXaName();
        App.AddAction(xa);
    }
    {
        cdda = new BrowserActionEntry;
        cdda->action = cdda_action;
        setCdName();
        App.AddAction(cdda);
    }
    {
        slowboot = new BrowserActionEntry;
        slowboot->action = slowboot_action;
        setSbName();
        App.AddAction(slowboot);
    }
    {
        spuirq = new BrowserActionEntry;
        spuirq->action = spuirq_action;
        setSIRQName();
        App.AddAction(spuirq);
    }
    
    {
        bios = new BrowserActionEntry;
        bios->action = bios_action;
        bios->name = "Bios: scph1001";
        strcpy(Config.Bios,"scph1001.bin");
        App.AddAction(bios);
    }
   
    {
        region = new BrowserActionEntry;
        region->action = region_action;
        region->name = "Region: Auto";
        Config.PsxAuto = 1;
        App.AddAction(region);
    }

}



extern "C" void GuiAlert(const char *message) {
    App.Alert(message);
}

int main() {
    xenos_init(VIDEO_MODE_AUTO);
    
    Hw::SystemInit(INIT_SOUND|INIT_VIDEO|INIT_USB);
    closeTray();
    App.SetLaunchAction((void (*)(char*))pcsxmain);
    
    
    
    InitActionEntry();
    
    
    App.Run("uda://pcsxr/");

    return 0;
}

extern "C" XenosDevice * getLzxVideoDevice() {
    return ZLX::g_pVideoDevice;
}

#endif
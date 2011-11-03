// lzx.cpp : Defines the entry point for the application.
//

#ifdef LZX_GUI

extern "C" {
    int pcsxmain(const char * cdfile);
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
static int xavalue = 0;
static int spuvalue = 0;
static char psxBios[256];
static int selected_bios = 0;
static int slowboot = 0;
static int cpu = 0;

extern "C" int GetXaGui() {
    return xavalue;
}

extern "C" int GetSpuIrqGui() {
    return spuvalue;
}

extern "C" const char * GetBiosGui() {
    return psxBios;
}

extern "C" int GetSlowbootGui() {
    return slowboot;
}

extern "C" int GetCpuGui() {
    return cpu;
}

std::string cpuName(){
    if(cpu)
        return "Cpu: Interpreter";
    else
        return "Cpu: Dynarec";
}

std::string xaName(){
    if(xavalue)
        return "XA: Disabled";
    else
        return "XA: Enabled";
}
std::string slowbootName(){
    if(slowboot)
        return "Slowboot: enabled";
    else
        return "Slowboot: Disabled";
}
std::string spuName(){
    if(spuvalue)
        return "Spu Irq not Always Enabled";
    else
        return "Spu Irq Always Enabled";
}


void ToggleCpu(void* param){
    lpBrowserActionEntry action = (lpBrowserActionEntry)param;
    cpu = !cpu;
    action->name = cpuName();
}

void ToggleBios(void* param){
    lpBrowserActionEntry action = (lpBrowserActionEntry)param;
    if (selected_bios == 0) {
        action->name = "Bios: scph1001.bin";
        strcpy(psxBios, "scph1001.bin");

        selected_bios++;
    } else {
        action->name = "Bios: scph7502.bin";
        strcpy(psxBios, "scph7502.bin");

        selected_bios = 0;
    }
}

void ToggleXa(void* param){
    lpBrowserActionEntry action = (lpBrowserActionEntry)param;
    xavalue = !xavalue;
    action->name = xaName();
}

void ToggleSpuIrq(void* param){
    lpBrowserActionEntry action = (lpBrowserActionEntry)param;
    spuvalue=!spuvalue;
    action->name = spuName();
}

void ToggleSlowBoot(void* param) {
    lpBrowserActionEntry action = (lpBrowserActionEntry)param;
    slowboot = !slowboot;
    action->name = slowbootName();
}

extern "C" void GuiAlert(const char *message) {
    App.Alert(message);
}

int main() {
    Hw::SystemInit(INIT_SOUND|INIT_VIDEO|INIT_USB);
    closeTray();
    App.SetLaunchAction((void (*)(char*))pcsxmain);
    strcpy(psxBios, "scph7502.bin");
    
    {
        lpBrowserActionEntry action = new BrowserActionEntry();
        action->name = "Bios: scph7502.bin";
        action->action = ToggleBios;
        action->param = NULL;
        App.AddAction(action);
    }
    {
        lpBrowserActionEntry action = new BrowserActionEntry();
        action->name = cpuName();
        action->action = ToggleCpu;
        action->param = NULL;
        App.AddAction(action);
    }
    {
        lpBrowserActionEntry action = new BrowserActionEntry();
        action->name = xaName();
        action->action = ToggleXa;
        action->param = NULL;
        App.AddAction(action);
    }
    {
        lpBrowserActionEntry action = new BrowserActionEntry();
        action->name = spuName();
        action->action = ToggleSpuIrq;
        action->param = NULL;
        App.AddAction(action);
    }
    {
        lpBrowserActionEntry action = new BrowserActionEntry();
        action->name = slowbootName();
        action->action = ToggleSlowBoot;
        action->param = NULL;
        App.AddAction(action);
    }
    
    
    App.Run("uda://pcsxr/");

    return 0;
}

extern "C" XenosDevice * getLzxVideoDevice() {
    return ZLX::g_pVideoDevice;
}

#endif
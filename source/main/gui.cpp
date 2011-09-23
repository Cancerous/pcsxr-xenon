#ifdef LZX_GUI
//#if 1
// lzx.cpp : Defines the entry point for the application.
//
extern "C" {
    int pcsxmain(const char * cdfile);
}

#include <zlx/Console.h>
#include <zlx/Draw.h>
#include <zlx/Bgshader.h>

#include <debug.h>
#include <xenos/xenos.h>

#include <diskio/ata.h>
extern "C" {
#include <input/input.h>
}
#include <console/console.h>
#include <diskio/diskio.h>
#include <usb/usbmain.h>
#include <time/time.h>
#include <xenon_soc/xenon_power.h>
#include <xenon_smc/xenon_smc.h>
#include <xenon_sound/sound.h>
#include <dirent.h>
#include <elf/elf.h>

#include <stdio.h>
#include <algorithm>
#include <vector>
#include <string>
#include <math.h>

#include "menu.h"

using namespace ZLX;

MyConsole cApp;

typedef ActionEntry * lpActionEntry;

lpActionEntry vAction[16];

static int nbAction = 0;

void addAction(lpActionEntry action) {
    if (action) {
        vAction[nbAction] = action;
        nbAction++;
    }
}

extern "C" void GuiAlert(const char *message) {
    cApp.Alert(message);
}

void append_dir_to_path(char * path, char * dir) {
    if (!strcmp(dir, "..")) {
        int i = strlen(path);
        int delimcount = 0;

        while (i >= 0 && delimcount < 2) {
            if (path[i] == '/') {
                ++delimcount;

                if (delimcount > 1) {
                    path[i + 1] = '\0';
                }
            }
            --i;
        }
    } else if (!strcmp(dir, ".")) {
        return;
    } else {
        strcat(path, dir);
        strcat(path, "/");
    }
}

bool vEntrySort(FileEntry i, FileEntry j) {
    if (i.type == j.type) {
        return i.name < j.name;
    } else {
        return (i.type > j.type);
    }
}

void MyConsole::RenderProgress() {

    float posX = -0.5f;
    float posY = -0.15f;

    float w = 1.f;
    float h = 0.3f;

    float borderSize = 0.03f;

    // white border
    Draw::DrawColoredRect(posX, posY, w, h, 0xFFFFFFFF);
    // black bg
    Draw::DrawColoredRect(
            posX + (borderSize / 2),
            posY + (borderSize / 2),
            w - borderSize,
            h - borderSize,
            0xFF000000
            );

    int PctColor = 0xFF00FF00;

    // progress things ...
    Draw::DrawColoredRect(
            posX + (borderSize / 2),
            posY + (borderSize / 2),
            (w - borderSize) * progressPct,
            h - borderSize,
            PctColor
            );
}

void MyConsole::RenderFileInfoPanel() {
    if (vEntry.size() == 0)
        return;

    unsigned int color = DefaultColor;
    float leftOffset = -1.f;
    float topOffset = 0.65f;

    if (panelSelected == PANEL_FILE_LIST) {
        color &= FocusColor;
    } else {
        color &= BlurColor;
    }

    char bottomInfo[256];
    sprintf(bottomInfo, "File %d/%d", (entrySelected + 1), vEntry.size());
    m_font.DrawTextF(bottomInfo, color, leftOffset, topOffset);
    m_font.DrawTextF("Some Info here ...", color, leftOffset, topOffset + 0.08f);
}

void MyConsole::RenderActionPanel() {
    float topOffset = -0.65f;
    float leftOffset = 0.2f;
    float lineHeight = 0.07f;

    for (int i = 0; i < nbAction; i++) {
        float posY = (topOffset)+(lineHeight * (i)); // + topOffset;
        unsigned int color = DefaultColor;
        if (actionSelected == i) {
            color = SelectedColor;
        }

        if (panelSelected == PANEL_ACTION) {
            color &= FocusColor;
        } else {
            color &= BlurColor;
        }

        if ((vAction[i]))
            m_font.DrawTextF(vAction[i]->name.c_str(), color, leftOffset, posY);
    }
}

void MyConsole::RenderTopPanel() {
    // m_font.DrawTextF("Everyday i'm browsin'", 0xFFFFFFFF, -1.f, -1.f);		
    m_font.Scale(0.8f);
    m_font.DrawTextF(currentPath, 0xFFFFFFFF, -1.f, -0.83f);
    m_font.Scale(1.0f);
}

void MyConsole::RenderFileListPanel() {
    int start, end;
    float topOffset = -0.75f;
    float leftOffset = -1.f;
    float lineHeight = 0.07f;

    int size = vEntry.size();

    if (size == 0)
        return;

    start = entrySelected - MAX_DISPLAYED_ENTRIES / 2;
    if (start < 0)
        start = 0;

    end = MIN(MAX_DISPLAYED_ENTRIES, vEntry.size() - start) + start;

    if (end > size)
        end = size;

    for (int k = start; k < end; k++) {

        float posY = (topOffset)+(lineHeight * (k - start)); // + topOffset;
        unsigned int color = DefaultColor;
        if (entrySelected == k) {
            color = SelectedColor;
        }

        if (panelSelected == PANEL_FILE_LIST) {
            color &= FocusColor;
        } else {
            color &= BlurColor;
        }

        //printf("%s - %d\r\n", vEntry.at(k).name.c_str(), vEntry.at(k).type);


        if (vEntry.at(k).type == 1) {
            m_font.DrawTextF("[Dir]", color, -0.2f, posY);
        }
        m_font.DrawTextF(vEntry.at(k).name.c_str(), color, leftOffset, posY);
    }

}

void MyConsole::RenderApp() {
    // Begin to draw
    Begin();

    // Draw bg
    if (bg == NULL)
        Bgshader::RenderBg();
    else
        Draw::DrawTexturedRect(-1.0f, -1.0f, 2.0f, 2.0f, bg);

    // Draw some text using font
    m_font.Begin();
    // need to fix that ..
    //m_font.DrawTextF("Error ?", SelectedColor, -0.55f, 0.3f);

    // Top part
    RenderTopPanel();

    // Render file listing
    RenderFileListPanel();

    // Render bottom part
    RenderFileInfoPanel();

    RenderActionPanel();

    if (panelSelected == PANEL_PROGRESS)
        RenderProgress();

    // Finished to draw font things ...
    m_font.End();

    // Draw all text + form
    Render();
    // Draw is finished
    End();
}

BOOL MyConsole::Warning(const char *message) {
    display_warning = 1;
    unsigned int color = 0xFF6f641a;

    memset(&old_ctrl, 0, sizeof (struct controller_data_s));
    memset(&ctrl, 0, sizeof (struct controller_data_s));

    while (display_warning == 1) {

        // Begin to draw
        Begin();

        // Draw bg

        if (bg == NULL)
            Bgshader::RenderBg();
        else
            Draw::DrawTexturedRect(-1.0f, -1.0f, 2.0f, 2.0f, bg);

        Draw::DrawColoredRect(-0.5f, -0.5f, 1.0f, 1.0f, color);

        m_font.Begin();

        // need to fix that ..
        //m_font.DrawTextF("Error ?", SelectedColor, -0.55f, 0.3f);


        m_font.DrawTextF(message, SelectedColor, -0.55f, -0.5f);

        m_font.DrawTextF("Press X to continue", SelectedColor, -0.55f, 0.3f);
        m_font.DrawTextF("Press B to cancel", SelectedColor, 0.00f, 0.3f);

        m_font.End();

        // Draw all text + form
        Render();

        // Draw is finished
        End();

        // Update code ...
        {
            usb_do_poll();
            get_controller_data(&ctrl, 0);

            if (ctrl.b & !old_ctrl.b) {
                display_warning = 0;
                return FALSE;
            }
            if (ctrl.x & !old_ctrl.x) {
                display_warning = 0;
                return TRUE;
            }
            // Save the old value
            old_ctrl = ctrl;
            memset(&ctrl, 0, sizeof (struct controller_data_s));
        }
    }
}

void MyConsole::Alert(const char *message) {
    display_alert = 1;
    unsigned int color = 0xFF1212b4;

    memset(&old_ctrl, 0, sizeof (struct controller_data_s));
    memset(&ctrl, 0, sizeof (struct controller_data_s));

    while (display_alert == 1) {
        // Begin to draw
        Begin();

        if (bg == NULL)
            Bgshader::RenderBg();
        else
            Draw::DrawTexturedRect(-1.0f, -1.0f, 2.0f, 2.0f, bg);
        Draw::DrawColoredRect(-0.5f, -0.5f, 1.0f, 1.0f, color);

        m_font.Begin();

        // need to fix that ..
        //m_font.DrawTextF("Error ?", SelectedColor, -0.55f, 0.3f);

        m_font.DrawTextF(message, SelectedColor, -0.55f, -0.5f);
        m_font.DrawTextF("Press B to continue", SelectedColor, -0.3f, 0.3f);

        m_font.End();

        // Draw all text + form
        Render();

        // Draw is finished
        End();

        // Update code ...
        {
            usb_do_poll();
            get_controller_data(&ctrl, 0);

            if (ctrl.b & !old_ctrl.b) {
                display_alert = 0;
                return;
            }
            // Save the old value
            old_ctrl = ctrl;
            memset(&ctrl, 0, sizeof (struct controller_data_s));
        }
    }
}

void MyConsole::SetProgressValue(float pct) {
    if (pct > 0.0f) {
        panelSelected = PANEL_PROGRESS;
        progressPct = pct;

        RenderApp();
    } else {
        panelSelected = PANEL_FILE_LIST;
    }
}

void MyConsole::Run() {
    Init();

    // Disable auto render on format
    SetAutoRender(0);
#ifdef WIN32
    strcpy(currentPath, "c:/");
#else
    //        strcpy(currentPath, "uda:/");
    handle = -1;
    const char * s;
    handle = bdev_enum(handle, &s);
    if (handle < 0)
        return;

    strcpy(currentPath, s);
    strcat(currentPath, ":/");
#endif
    ScanDir();

    while (1) {
        RenderApp();
        Update();
    }
}


void MyConsole::ScanDir() {
    entrySelected = 0;

    DIR * rep = opendir(currentPath);

    vEntry.clear();

    if (rep != NULL) {
        struct dirent * ent;

        while ((ent = readdir(rep)) != NULL) {
            // never display hidden files
            if (ent->d_name[0] == '.')
                continue;

            if (ent->d_type == FILE_TYPE) {
                // check for extentions
                int len = strlen(ent->d_name);
                char * ext = NULL;
                for(unsigned int i=len;i;i--){
                    if(ent->d_name[i]=='.'){
                        ext = ent->d_name+i;
                        break;
                    }
                }
                if(ext!=NULL){
                    if(stricmp(ext,".bin")==0){
                        
                    }
                    else if(stricmp(ext,".iso")==0){
                        
                    }
                    else if(stricmp(ext,".img")==0){
                       
                    }
                    else if(stricmp(ext,".mdf")==0){
                        
                    }
                    else if(stricmp(ext,".nrg")==0){
                        
                    }
                    else if(stricmp(ext,".Z")==0){
                        
                    }
                    else{
                        continue;
                    }
                }
                else
                {
                    continue;
                }
                
            }

            FileEntry currentEntry;
            currentEntry.name = ent->d_name;
            currentEntry.type = (ent->d_type == DIR_TYPE) ? 1 : 0;
            vEntry.push_back(currentEntry);
        }

        closedir(rep);
    }
    // Sort array
    std::sort(vEntry.begin(), vEntry.end(), vEntrySort);

    return;
}

void MyConsole::Update() {
#ifdef WIN32
    W32Update();
#endif
    usb_do_poll();

    get_controller_data(&ctrl, 0);
    {
        //if (get_controller_data(&ctrl, 0)) {
        if (ctrl.up && !old_ctrl.up) {
            switch (panelSelected) {
                case PANEL_FILE_LIST:
                    entrySelected--;
                    break;
                case PANEL_ACTION:
                    actionSelected--;
                    break;
            }
        }

        if (ctrl.down && !old_ctrl.down) {
            switch (panelSelected) {
                case PANEL_FILE_LIST:
                    entrySelected++;
                    break;
                case PANEL_ACTION:
                    actionSelected++;
                    break;
            }
        }
        if (ctrl.b && !old_ctrl.b) {
            switch (panelSelected) {
                case PANEL_FILE_LIST:
                    append_dir_to_path(currentPath, "..");
                    ScanDir();
                    break;
            }
        }
        if (ctrl.select && !old_ctrl.select) {
            switch (panelSelected) {
                case PANEL_FILE_LIST:
                    const char * s;
                    do {
                        handle = bdev_enum(handle, &s);
                    } while (handle < 0);

                    strcpy(currentPath, s);
                    strcat(currentPath, ":/");
                    ScanDir();
                    break;
            }
        }
        if
            (
                (ctrl.start && !old_ctrl.start) || (ctrl.a && !old_ctrl.a)
                ) {
            switch (panelSelected) {
                case PANEL_FILE_LIST:
                {
                    if (vEntry.size() == 0)
                        break;

                    // entrySelected++;
                    FileEntry currentEntry = vEntry.at(entrySelected);

                    if (currentEntry.type == 1) {
                        append_dir_to_path(currentPath, (char*) currentEntry.name.c_str());
                        ScanDir();
                    } else {
                        sprintf(currentFile, "%s/%s", currentPath, currentEntry.name.c_str());
                        pcsxmain(currentFile);
                    }
                    break;
                }
                case PANEL_ACTION:
                {
                    if (nbAction == 0)
                        break;

                    // Exec action ...
                    //FileEntry currentEntry = vEntry.at(entrySelected);
                    ActionEntry * currentAction = vAction[actionSelected];
                    if (currentAction->action != NULL) {
                        //sprintf((char*)currentAction.param, "%s/%s", currentPath, currentEntry.name.c_str());
                        currentAction->action(currentAction, currentPath);
                    }
                    break;
                }
            }
        }

        if (ctrl.left && !old_ctrl.left) {
            //if(panelSelected!=PANEL_PROGRESS)
            panelSelected--;
        }
        if (ctrl.right && !old_ctrl.right) {
            //if(panelSelected!=PANEL_PROGRESS)
            panelSelected++;
        }

        // clamp ...
        CLAMP(entrySelected, 0, vEntry.size() - 1);
        CLAMP(actionSelected, 0, nbAction - 1);
        CLAMP(panelSelected, 0, 1);


        // Save the old value
        old_ctrl = ctrl;

        // Erase for the big loop
        memset(&ctrl, 0, sizeof (struct controller_data_s));
    }


}

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

void ToggleCpu(ActionEntry * action, void * unused) {
    if (cpu == 0) {
        action->name = "Cpu: Interpreter";
        cpu++;
    } else {
        action->name = "Cpu: Dynarec";
        cpu = 0;
    }
}

void ToggleBios(ActionEntry * action, void * unused) {
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

void ToggleXa(ActionEntry * action, void * unused) {
    if (xavalue == 0) {
        action->name = "XA Disabled";
        xavalue++;
    } else {
        action->name = "XA Enabled";
        xavalue = 0;
    }
}

void ToggleSpuIrq(ActionEntry * action, void * unused) {
    if (spuvalue == 0) {
        action->name = "Spu Irq Always Enabled";
        spuvalue++;
    } else {
        action->name = "Spu Irq not Always Enabled";
        spuvalue = 0;
    }
}

void ToggleSlowBoot(ActionEntry * action, void * unused) {
    if (slowboot == 0) {
        action->name = "Slowboot enabled";
        slowboot++;
    } else {
        action->name = "Slowboot disabled";
        slowboot = 0;
    }
}

void MyConsole::InitActionEntry() {
    strcpy(psxBios, "scph7502.bin");
#if 1

    {
        lpActionEntry action = new ActionEntry();
        action->name = "Bios: scph7502.bin";
        action->action = ToggleBios;
        action->param = NULL;
        //vAction.push_back(action);
        addAction(action);
    }
    {
        lpActionEntry action = new ActionEntry();
        action->name = "Cpu: Dynarec";
        action->action = ToggleCpu;
        action->param = NULL;
        //vAction.push_back(action);
        addAction(action);
    }
    {
        lpActionEntry action = new ActionEntry();
        action->name = "XA Enabled";
        action->action = ToggleXa;
        action->param = NULL;
        //vAction.push_back(action);
        addAction(action);
    }
    {
        lpActionEntry action = new ActionEntry();
        action->name = "Spu Irq Always Enabled";
        action->action = ToggleSpuIrq;
        action->param = NULL;
        //vAction.push_back(action);
        addAction(action);
    }
    {
        lpActionEntry action = new ActionEntry();
        action->name = "Slowboot disabled";
        action->action = ToggleSlowBoot;
        action->param = NULL;
        //vAction.push_back(action);
        addAction(action);
    }
#endif
}

void MyConsole::Init() {
    entrySelected = 0;
    panelSelected = 0;
    actionSelected = 0;

    display_warning = 0;
    display_alert = 0;

    progressPct = 0.0f;

    InitActionEntry();

    Create("uda:/pcsxr/", 0x00000000, 0xFFFFFF00);
    LoadTextureFromFile(g_pVideoDevice, "uda:/pcsxr/bg.png", &bg);
};


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

int main() {

    usb_init();
    usb_do_poll();
    xenon_ata_init();
    xenon_atapi_init();
    xenos_init(VIDEO_MODE_AUTO);
    xenon_smc_start_bootanim();

    closeTray();

    cApp.Run();

    while (1) {
    }

    return 0;
}

extern "C" XenosDevice * getLzxVideoDevice() {
    return ZLX::g_pVideoDevice;
}

#endif
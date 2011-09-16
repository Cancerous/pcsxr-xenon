/* 
 * File:   menu.h
 * Author: cc
 *
 * Created on 22 juillet 2011, 20:39
 */

#ifndef MENU_H
#define	MENU_H


#define STICK_THRESHOLD 25000

#define MAX_PATH 512
#define MAX_FILES 1000
#define MAX_DISPLAYED_ENTRIES 20

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))
//#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define CLAMP(x,low,high) {if(x<low)x=low;if(x>high)x=high;}

#ifdef LIBXENON
#define FILE_TYPE	8
#define DIR_TYPE        4
#endif

enum {
    PANEL_FILE_LIST = 0,
    PANEL_ACTION,
    PANEL_PROGRESS
};

// Action
void ActionBootTFTP(void * unused);
void ActionShutdown(void * unused);
void ActionRestart(void * unused);
void ActionLaunchElf(const char * filename);
void ActionDumpNand(void * unused);
void ActionLaunchFile(const char * filename);

struct ActionEntry {
    std::string name;
    void * param;
    void (*action)(ActionEntry * _this,void * param);
};

struct FileEntry {
    std::string name;
    int type;
};

class MyConsole : public ZLX::Console {
private:
    static const unsigned int DefaultColor = 0x44FFFFFF;
    static const unsigned int SelectedColor = 0xFFFFFFFF;
    static const unsigned int FocusColor = 0xFFFFFFFF;
    static const unsigned int BlurColor = 0x44FFFFFF;


    ZLXTexture * bg;

    char currentPath[MAX_PATH];
    char currentFile[MAX_PATH];

    // Selected
    int entrySelected;
    int actionSelected;
    int panelSelected;

    std::vector<FileEntry> vEntry;
   // std::vector<ActionEntry*> vAction;

    float progressPct;

    // handle to bdev
    int handle;

    int display_alert;
    int display_warning;

    controller_data_s ctrl;
    controller_data_s old_ctrl;

    void RenderProgress();
    void RenderFileInfoPanel();
    void RenderActionPanel();
    void RenderTopPanel();
    void RenderFileListPanel();
    void RenderApp();

    void ScanDir();
    void Update();
    void InitActionEntry();
    void Init();
public:
    BOOL Warning(const char *message);
    void Alert(const char *message);
    void SetProgressValue(float pct);
    void Run();
};

extern MyConsole cApp;

#endif	/* MENU_H */


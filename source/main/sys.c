#include "config.h"

#include "psxcommon.h"
#include "debug.h"
#include "sio.h"
#include "misc.h"
//#include "cheat.h"

//#ifndef PCSXDF
#if 1
#define lpctr const char
#else
#define lpctr char
//int cdOpenCase = 0;
int NetOpened = 0;
long LoadCdBios = 0;
#endif

#include "gamecube_plugins.h"
PluginTable plugins[] ={
    PLUGIN_SLOT_0,
    PLUGIN_SLOT_1,
    PLUGIN_SLOT_2,
    PLUGIN_SLOT_3,
    PLUGIN_SLOT_4,
    PLUGIN_SLOT_5,
    PLUGIN_SLOT_6,
    PLUGIN_SLOT_7
};

int SysInit() {
#ifndef PCSXDF
    if (EmuInit() == -1) return -1;
#else
    if (psxInit() == -1) return -1;
#endif

    LoadMcds(Config.Mcd1, Config.Mcd2);

    return 0;
}

void SysReset() {
#ifndef PCSXDF
    EmuReset();
#else

#endif
}

void SysClose() {
#ifndef PCSXDF
    EmuShutdown();
#endif
    ReleasePlugins();
}

void SysPrintf(lpctr *fmt, ...) {
    va_list list;
    char msg[512];

    va_start(list, fmt);
    vsprintf(msg, fmt, list);
    va_end(list);

    printf("PCSX: %s\r\n", msg);

}

void SysMessage(lpctr *fmt, ...) {
    va_list list;
    char tmp[512];

    va_start(list, fmt);
    vsprintf(tmp, fmt, list);
    va_end(list);
    printf(tmp);
}

static char *err = N_("Error Loading Symbol");
static int errval;

void *SysLoadLibrary(lpctr *lib) {
    printf("SysLoadLibrary : %s\r\n",lib);
    int i;
    for (i = 0; i < NUM_PLUGINS; i++)
        if ((plugins[i].lib != NULL) && (!strcmp(lib, plugins[i].lib)))
            return (void*) i;
    return NULL;
}

void *cdrcimg_get_sym(const char *sym);

void *SysLoadSym(void *lib, lpctr *sym) {
    printf("SysLoadSym : %s\r\n",sym);
    PluginTable* plugin = plugins + (int) lib;
    int i;
    for (i = 0; i < plugin->numSyms; i++)
        if (plugin->syms[i].sym && !strcmp(sym, plugin->syms[i].sym)) {
            //printf("SysLoadSym : %s -> %p\r\n",sym,plugin->syms[i].pntr);
            return plugin->syms[i].pntr;
        }
    printf("SysLoadSym : %s not found\r\n",sym);
    return NULL;
}

const char *SysLibError() {
    //printf("SysLibError\r\n");
    if (errval) {
        errval = 0;
        return err;
    }
    return NULL;
}

void SysCloseLibrary(void *lib) {
    // FreeLibrary((HINSTANCE) lib);
}

void SysUpdate() {

}

void SysRunGui() {

}

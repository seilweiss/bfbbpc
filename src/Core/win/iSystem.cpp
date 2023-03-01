#include "iSystem.h"

#include "iFile.h"
#include "iTime.h"

#include "xDebug.h"
#include "xMemMgr.h"
#include "xPad.h"
#include "xSnd.h"
#include "xMath.h"
#include "xMath3.h"
#include "xShadow.h"
#include "xFX.h"
#include "xGlobals.h"
#include "xString.h"
#include "xstransvc.h"

#include <SDL.h>
#include <SDL_syswm.h>

#include <rwcore.h>
#include <rpworld.h>
#include <rpcollis.h>
#include <rpskin.h>
#include <rphanim.h>
#include <rpmatfx.h>
#include <rpusrdat.h>
#include <rpptank.h>

#include <stdio.h>
#include <stdlib.h>

#define RES_ARENA_SIZE 0x60000

RwVideoMode sVideoMode;

static SDL_Window* window;
static U32 shouldQuit = 0;

static RwEngineOpenParams openParams;

static RwDebugHandler oldDebugHandler;

static U32 RenderWareInit();
static void RenderWareExit();

static void psDebugMessageHandler(RwDebugType type, const RwChar* str)
{
    printf("%s\n", str);

    if (oldDebugHandler) {
        oldDebugHandler(type, str);
    }
}

void iVSync() WIP
{
    SDL_Delay(1000/VBLANKS_PER_SEC);
}

static void TRCInit() WIP
{
}

void iSystemInit(U32 options)
{
    shouldQuit = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL! SDL Error: %s\n", SDL_GetError());
        exit(-1);
    }

    window = SDL_CreateWindow(GAME_NAME,
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              FB_XRES, FB_YRES,
                              SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Failed to create SDL window! SDL Error: %s\n", SDL_GetError());
        exit(-1);
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;

    openParams.displayID = (void*)hwnd;

    xDebugInit();
    xMemInit();
    iFileInit();
    iTimeInit();
    xPadInit();
    xSndInit();
    TRCInit();
    RenderWareInit();
    xMathInit();
    xMath3Init();
}

void iSystemExit()
{
    xDebugExit();
    xMathExit();
    RenderWareExit();
    xSndExit();
    xPadKill();
    iFileExit();
    iTimeExit();
    xMemExit();

    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("(With apologies to Jim Morrison) This the end, my only friend, The End.");
    exit(0);
}

static U32 RWAttachPlugins()
{
    if (!RpWorldPluginAttach()) return 1;
    if (!RpCollisionPluginAttach()) return 1;
    if (!RpSkinPluginAttach()) return 1;
    if (!RpHAnimPluginAttach()) return 1;
    if (!RpMatFXPluginAttach()) return 1;
    if (!RpUserDataPluginAttach()) return 1;
    if (!RpPTankPluginAttach()) return 1;

    return 0;
}

static RwTexture* TextureRead(const char* name, const char* maskName);

static U32 RenderWareInit()
{
    if (!RwEngineInit(NULL, 0, RES_ARENA_SIZE)) {
        return 1;
    }

    RwResourcesSetArenaSize(RES_ARENA_SIZE);

#ifdef RWDEBUG
    oldDebugHandler = RwDebugSetHandler(psDebugMessageHandler);
    RwDebugSendMessage(rwDEBUGMESSAGE, GAME_NAME, "Debugging Initialized");
#endif

    if (RWAttachPlugins()) return 1;

    if (!RwEngineOpen(&openParams)) {
        RwEngineTerm();
        return 1;
    }

    RwEngineGetVideoModeInfo(&sVideoMode, RwEngineGetCurrentVideoMode());

    if (!RwEngineStart()) {
        RwEngineClose();
        RwEngineTerm();
        return 1;
    }

    RwTextureSetReadCallBack(TextureRead);

    RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)rwCULLMODECULLBACK);

    xShadowInit();
    xFXInit();

    RwTextureSetMipmapping(TRUE);
    RwTextureSetAutoMipmapping(TRUE);

    return 0;
}

static void RenderWareExit()
{
    RwEngineStop();
    RwEngineClose();
    RwEngineTerm();
}

static RwTexture* TextureRead(const RwChar* name, const RwChar* maskName)
{
    char tmpname[256];
    S32 npWidth = 0;
    S32 npHeight = 0;
    S32 npDepth = 0;
    S32 npFormat = 0;
    RwImage* img = NULL;
    RwRaster* rast = NULL;
    RwTexture* result;
#ifdef GAMECUBE
    RwGameCubeRasterExtension* ext = NULL;
#endif
    U32 assetid;
    U32 tmpsize;

    sprintf(tmpname, "%s.rw3", name);
    assetid = xStrHash(tmpname);
    result = (RwTexture*)xSTFindAsset(assetid, &tmpsize);

#ifdef GAMECUBE
    if (result && result->raster && result->raster->depth < 8) {
        ext = RwGameCubeRasterGetExtension(result->raster);
        if (!ext || ext->format != 14) {
            result = NULL;
        }
    }
#endif

    if (result) {
        strcpy(result->name, name);
        strcpy(result->mask, maskName);
    }

    return result;
}

void iSystemPollEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            shouldQuit = 1;
#if 1
            exit(0);
#endif
            break;
        }
    }
}

U32 iSystemShouldQuit()
{
    return shouldQuit;
}

U32 iSystemIsFullScreen()
{
    return 0;
}
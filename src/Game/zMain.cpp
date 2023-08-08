#include "zMain.h"

#include "iSystem.h"
#include "iTRC.h"

#include "xutil.h"
#include "xstransvc.h"
#include "xFont.h"
#include "xTRC.h"
#include "xScrFx.h"
#include "xFX.h"
#include "xParMgr.h"
#include "xsavegame.h"
#include "xDebug.h"
#include "xBehaveMgr.h"
#include "xserializer.h"

#include "zGlobals.h"
#include "zVar.h"
#include "zAssetTypes.h"
#include "zMenu.h"
#include "zDispatcher.h"
#include "zParCmd.h"
#include "zEntPickup.h"
#include "zCameraTweak.h"
#include "zShrapnel.h"
#include "zNPCMgr.h"
#include "zPickupTable.h"
#include "zCutsceneMgr.h"
#include "zGameState.h"
#include "zGame.h"

#include <SDL.h>

#define SHOW_COPYRIGHT_SCREEN 1

zGlobals globals;
xGlobals* xglobals = &globals;

static S32 sShowMenuOnBoot = 1;

F32 gSkipTimeCutscene = 1.0f;
F32 gSkipTimeFlythrough = 1.0f;

static st_SERIAL_PERCID_SIZE g_xser_sizeinfo[3] = {
    { 'PLYR', 328 },
    { 'CNTR', 480 },
    { 0 }
};

static S32 percentageDone = 0;

static void zMainOutputMgrSetup();
static void zMainInitGlobals();
static void zMainMemLvlChkCB();
static void zMainLoop();
static void zMainReadINI();
static void zMainLoadFontHIP();

extern "C" {
int main(int argc, char** argv) WIP
{
    memset(&globals, 0, sizeof(globals));
    globals.firstStartPressed = true;

    iSystemInit(0);
    zMainOutputMgrSetup();
    xMemRegisterBaseNotifyFunc(zMainMemLvlChkCB);
    zMainInitGlobals();
    var_init();
    zAssetStartup();
    zMainLoadFontHIP();
    xfont::init();

#if SHOW_COPYRIGHT_SCREEN
    zMainFirstScreen(1);
#endif

    zMainShowProgressBar();

    xTRCInit();
    zMainReadINI();
    iFuncProfileParse("scooby.elf", globals.profile);
    xUtilStartup();
    xSerialStartup(128, g_xser_sizeinfo);
    zDispatcher_Startup();
    xScrFxInit();
    xFXStartup();

    zMainShowProgressBar();

    xParMgrInit();
    zParCmdInit();
    iEnvStartup();
    zEntPickup_Startup();
    zCameraTweakGlobal_Init();

    globals.option_vibration = true;
    globals.pad0 = xPadEnable(globals.currentActivePad);
    globals.pad1 = NULL;
    gDebugPad = NULL;
    xPadRumbleEnable(globals.currentActivePad, globals.option_vibration);

    xSGStartup();
    xDebugTimestampScreen();
    zShrapnel_GameInit();

    zMainShowProgressBar();

    xBehaveMgr_Startup();
    zNPCMgr_Startup();

    // MAIN LOOP
    zMainLoop();

    zNPCMgr_Shutdown();
    xBehaveMgr_Shutdown();
    zAssetShutdown();
    xFXShutdown();
    zDispatcher_Shutdown();
    xSGShutdown();
    xSerialShutdown();
    xUtilShutdown();
    iSystemExit();

    return 0;
}
}

static void zMainOutputMgrSetup()
{
    //iTimeDiffSec(iTimeGet());
    //iTimeGet();
}

static void zMainInitGlobals()
{
    memset(&globals, 0, sizeof(globals));
    globals.sceneFirst = 1;

    //iTimeDiffSec(iTimeGet());
    //iTimeGet();
}

static void zMainMemLvlChkCB()
{
    zSceneMemLvlChkCB();
}

void zMainShowProgressBar()
{
    if (!zMenuIsFirstBoot()) return;

    if (percentageDone > 100) percentageDone = 100;
    
    S32 spotToInsertColor = percentageDone / 10;
    char textBuffer[64];
    char beforeColor[64];
    char afterColor[64];
    
    char loadingText[12];
    strcpy(loadingText, "Loading...");
    
    memset(textBuffer, 0, sizeof(textBuffer));
    memset(afterColor, 0, sizeof(afterColor));
    
    strcpy(beforeColor, loadingText);
    beforeColor[spotToInsertColor] = '\0';
    memcpy(afterColor, loadingText + spotToInsertColor, strlen(loadingText) - spotToInsertColor);
    
    sprintf(textBuffer, "{font=0}{h*2}{w*2}%s{color=FFFFFFFF}%s{~:c}", beforeColor, afterColor);

    zMainMemCardRenderText(textBuffer, true);

    percentageDone += 10;
}

static void zMainLoop() WIP
{
    zMainShowProgressBar();

    xMemPushBase();

    //iTime t;
    
    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('B','O','O','T'));
    //iTimeDiffSec(t);

    xSTPreLoadScene(IDTAG('B','O','O','T'), NULL, XST_OPTS_HIP);

    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('B','O','O','T'));
    //iTimeDiffSec(t);

    xSTQueueSceneAssets(IDTAG('B','O','O','T'), XST_OPTS_HIP);

    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('B','O','O','T'));
    //iTimeDiffSec(t);

    while (xSTLoadStep(IDTAG('B','O','O','T')) < 1.0f) {}

    xSTDisconnect(IDTAG('B','O','O','T'), XST_OPTS_HIP);

    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('B','O','O','T'));
    //iTimeDiffSec(t);

    zMainShowProgressBar();

    xSTPreLoadScene(IDTAG('P','L','A','T'), NULL, XST_OPTS_HIP);
    xSTQueueSceneAssets(IDTAG('P','L','A','T'), XST_OPTS_HIP);
    while (xSTLoadStep(IDTAG('P','L','A','T')) < 1.0f) {}
    xSTDisconnect(IDTAG('P','L','A','T'), XST_OPTS_HIP);

    zMainShowProgressBar();

    //iTimeGet();

    xShadowSimple_Init();

    globals.pickupTable = (zAssetPickupTable*)xSTFindAssetByType('PICK', 0, NULL);
    zPickupTableInit();

    xMemPushBase();

    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('M','N','U','4'));
    //iTimeDiffSec(t);

    xSTPreLoadScene(IDTAG('M','N','U','4'), NULL, XST_OPTS_HIP);

    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('M','N','U','4'));
    //iTimeDiffSec(t);

    xSTQueueSceneAssets(IDTAG('M','N','U','4'), XST_OPTS_HIP);

    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('M','N','U','4'));
    //iTimeDiffSec(t);

    while (xSTLoadStep(IDTAG('M','N','U','4')) < 1.0f) {}

    xSTDisconnect(IDTAG('M','N','U','4'), XST_OPTS_HIP);

    zMainShowProgressBar();

    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('M','N','U','4'));
    //iTimeDiffSec(t);

    xMemPushBase();

    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('M','N','U','5'));
    //iTimeDiffSec(t);

    xSTPreLoadScene(IDTAG('M','N','U','5'), NULL, XST_OPTS_HIP);

    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('M','N','U','5'));
    //iTimeDiffSec(t);

    xSTQueueSceneAssets(IDTAG('M','N','U','5'), XST_OPTS_HIP);

    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('M','N','U','5'));
    //iTimeDiffSec(t);

    while (xSTLoadStep(IDTAG('M','N','U','5')) < 1.0f) {}

    xSTDisconnect(IDTAG('M','N','U','5'), XST_OPTS_HIP);

    zMainShowProgressBar();

    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('M','N','U','5'));
    //iTimeDiffSec(t);

    xModelInit();
    xModelPoolInit(32, 64);
    xModelPoolInit(40, 8);
    xModelPoolInit(56, 1);

    static U32 preinit_bubble_matfx[] = {
        0xDBD033BC,
        0x452279A2,
        0xC17F4BCC,
        0x0CF9267A,
        0x5C009D14,
        0
    };

    for (U32* preinit = preinit_bubble_matfx; *preinit != 0; preinit++) {
        RpAtomic* modl = (RpAtomic*)xSTFindAsset(*preinit, NULL);
        if (modl) {
            xFXPreAllocMatFX(RpAtomicGetClump(modl));
        }
    }

    static U32 menuModeID = IDTAG('M','N','U','3');
    static U32 gameSceneID = (globals.sceneStart[0] << 24) |
                             (globals.sceneStart[1] << 16) |
                             (globals.sceneStart[2] << 8) |
                             (globals.sceneStart[3] << 0);

    U32 newGameSceneID = gameSceneID;

    //xUtil_idtag2string(gameSceneID);

    xMemPushBase();

    zMainShowProgressBar();

    zMainMemCardSpaceQuery();

    while (true) {
        zMainShowProgressBar();

        xSerialWipeMainBuffer();
        zCutSceneNamesTable_clearAll();

        zMainShowProgressBar();

        if (sShowMenuOnBoot) {
            zMenuInit(menuModeID);

            zMainShowProgressBar();
            zMainShowProgressBar();

            zMenuSetup();

#if ENABLE_WIP_CODE
            xFX_SceneEnter(globals.sceneCur->env->geom->world);
#endif

            newGameSceneID = zMenuLoop();

            zMenuExit();

            if (iSystemShouldQuit()) break;
        } else {
            sShowMenuOnBoot = 1;
            globals.firstStartPressed = true;

            zGameModeSwitch(eGameMode_Game);
            zGameStateSwitch(0);
        }

        if (newGameSceneID == 0) {
            zVarNewGame();
            iTimeSetGame(0.0f);

            gameSceneID = (globals.sceneStart[0] << 24) |
                          (globals.sceneStart[1] << 16) |
                          (globals.sceneStart[2] << 8) |
                          (globals.sceneStart[3] << 0);
        } else {
            gameSceneID = newGameSceneID;

            iTimeSetGame(0.0f);
        }

        zGameInit(gameSceneID);
        zGameSetup();

        iProfileClear(gameSceneID);

        zGameLoop();
        zGameExit();

        if (iSystemShouldQuit()) break;
    }
}

static void zMainReadINI() WIP
{
}

void zMainFirstScreen(S32 mode) NONMATCH("https://decomp.me/scratch/i0iOC")
{
    RwCamera* cam = iCameraCreate(FB_XRES, FB_YRES, 0);
    RwRGBA bg = { 0, 0, 0, 0 };

    for (S32 i = 0; i < 2; i++) {
        RwCameraClear(cam, &bg, rwCAMERACLEARIMAGE | rwCAMERACLEARZ);
        RwCameraBeginUpdate(cam);

        if (mode != 0) {
            char text[] = "Game and Software \xA9 2003 THQ Inc. \xA9 2003 Viacom "
                          "International Inc. All rights reserved.\n\n"
                          "Nickelodeon, SpongeBob SquarePants and all related "
                          "titles, logos, and characters are trademarks of Viacom "
                          "International Inc. Created by Stephen Hillenburg.\n\n"
                          "Exclusively published by THQ Inc. Developed by Heavy "
                          "Iron. Portions of this software are Copyright 1998 - "
                          "2003 Criterion Software Ltd. and its Licensors. THQ, "
                          "Heavy Iron and the THQ logo are trademarks and/or "
                          "registered trademarks of THQ Inc. All rights reserved.\n\n"
                          "All other trademarks, logos and copyrights are property "
                          "of their respective owners.\n\n"
                          "Unofficial PC port by Heavy Iron Modding";
            
            xColor color = { 255, 230, 0, 200 };
            xtextbox tb = xtextbox::create(
                xfont::create(1, NSCREENX(19.0f), NSCREENY(22.0f), 0.0f, color, screen_bounds),
                screen_bounds, 0x2, 0.0f, 0.0f, 0.0f, 0.0f
            );
            tb.set_text(text);
/*
            tb.set_text("It is a serious crime to copy video games according to copyright law. "
                        "Please refer to your Nintendo game instruction booklet for further information.\n\n"
                        "{r=1.0}{g=1.0}{b=1.0}{w=.5}{h=.5}{tex:sbjail;src=0,0.16,1,0.74}");
*/
            tb.bounds = screen_bounds;
            tb.bounds.contract(0.1f);
            tb.bounds.h = tb.yextent(true);
            tb.bounds.y = 0.5f - tb.bounds.h * 0.5f;
            tb.render(true);
        }

        RwCameraEndUpdate(cam);
        RwCameraShowRaster(cam, NULL, 0);
    }

    S32 vbl = VBLANKS_PER_SEC * 3;
    while (--vbl) {
        iSystemPollEvents();
        if (iSystemShouldQuit()) break;

        iTRCDisk::CheckDVDAndResetState();
        iVSync();
    }

    iCameraDestroy(cam);
}

void zMainMemCardSpaceQuery() WIP
{
}

void zMainMemCardRenderText(const char* text, bool enabled)
{
    RwRGBA bg = { 0, 0, 0, 0 };
    RwCamera* cam = iCameraCreate(FB_XRES, FB_YRES, 0);

    RwCameraClear(cam, &bg, rwCAMERACLEARIMAGE | rwCAMERACLEARZ);
    RwCameraBeginUpdate(cam);

    RenderText(text, enabled);

    RwCameraEndUpdate(cam);
    RwCameraShowRaster(cam, NULL, 0);

    iCameraDestroy(cam);
}

static void zMainLoadFontHIP()
{
    xMemPushBase();

    //iTime t;

    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('F','O','N','T'));
    //iTimeDiffSec(t);

    xSTPreLoadScene(IDTAG('F','O','N','T'), NULL, XST_OPTS_HIP);

    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('F','O','N','T'));
    //iTimeDiffSec(t);

    xSTQueueSceneAssets(IDTAG('F','O','N','T'), XST_OPTS_HIP);

    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('F','O','N','T'));
    //iTimeDiffSec(t);

    while (xSTLoadStep(IDTAG('F','O','N','T')) < 1.0f) {}

    xSTDisconnect(IDTAG('F','O','N','T'), XST_OPTS_HIP);

    //t = iTimeGet();
    //xUtil_idtag2string(IDTAG('F','O','N','T'));
    //iTimeDiffSec(t);
}
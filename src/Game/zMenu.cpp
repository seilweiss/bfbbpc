#include "zMenu.h"

#include "iSystem.h"
#include "iTime.h"

#include "xMath.h"

#include "zGlobals.h"
#include "zScene.h"
#include "zCamera.h"
#include "zMusic.h"

static S32 sFirstBoot = 1;
static S32 sInMenu = 0;

void zMenuInit(U32 theSceneID) WIP
{
    sInMenu = 1;

    xsrand((U32)iTimeGet());
    xrand();

    zSceneInit(theSceneID, 0);

    xCameraInit(&globals.camera, FB_XRES, FB_YRES);
    zCameraReset(&globals.camera);
    WIPBLOCK{
        xCameraSetScene(&globals.camera, globals.sceneCur);
    };

    zMusicInit();
}

void zMenuExit() WIP
{
}

void zMenuSetup() WIP
{
}

U32 zMenuLoop() WIP
{
    while (true) {
        iSystemPollEvents();
        if (iSystemShouldQuit()) break;
    }

    return 0;
}

S32 zMenuIsFirstBoot()
{
    return sFirstBoot;
}
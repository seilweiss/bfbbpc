#pragma once

#include "iTime.h"

#include "xCamera.h"
#include "xPad.h"
#include "xUpdateCull.h"

#include <rwcore.h>
#include <rpworld.h>

struct xGlobals
{
    xCamera camera;
    xPad* pad0;
    xPad* pad1;
    xPad* pad2;
    xPad* pad3;
    S32 profile;
    char profFunc[6][128];
    xUpdateCullMgr* updateMgr;
    S32 sceneFirst;
    char sceneStart[32];
    RpWorld* currWorld;
    iFogParams fog;
    iFogParams fogA;
    iFogParams fogB;
    iTime fog_t0;
    iTime fog_t1;
    S32 option_vibration;
    U32 QuarterSpeed;
    F32 update_dt;
    S32 useHIPHOP;
    bool NoMusic;
    char currentActivePad;
    bool firstStartPressed;
    U32 minVSyncCnt;
    bool dontShowPadMessageDuringLoadingOrCutScene;
    bool autoSaveFeature;
};

extern xGlobals* xglobals;
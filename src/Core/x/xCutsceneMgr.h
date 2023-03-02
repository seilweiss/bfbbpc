#pragma once

#include "xBase.h"
#include "xCutscene.h"

struct xCutsceneMgrAsset;

struct xCutsceneMgr : xBase
{
    xCutsceneMgrAsset* tasset;
    xCutscene* csn;
    U32 stop;
    xCutsceneZbufferHack* zhack;
    F32 oldfov;
};
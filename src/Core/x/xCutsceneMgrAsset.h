#pragma once

#include "xBaseAsset.h"

struct xCutsceneMgrAsset : xBaseAsset
{
    U32 cutsceneAssetID;
    U32 flags;
    F32 interpSpeed;
    F32 startTime[15];
    F32 endTime[15];
    U32 emitID[15];
};
#pragma once

#include "xBase.h"
#include "xMath3.h"

struct xSFXAsset : xBaseAsset
{
    U16 flagsSFX;
    U16 freq;
    F32 freqm;
    U32 soundAssetID;
    U32 attachID;
    U8 loopCount;
    U8 priority;
    U8 volume;
    U8 pad;
    xVec3 pos;
    F32 innerRadius;
    F32 outerRadius;
};
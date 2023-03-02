#pragma once

#include "xBaseAsset.h"
#include "xVec3.h"

struct xMovePointAsset : xBaseAsset
{
    xVec3 pos;
    U16 wt;
    U8 on;
    U8 bezIndex;
    U8 flg_props;
    U8 pad;
    U16 numPoints;
    F32 delay;
    F32 zoneRadius;
    F32 arenaRadius;
};
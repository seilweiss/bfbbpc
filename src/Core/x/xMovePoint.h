#pragma once

#include "xBase.h"
#include "xMath3.h"
#include "xSpline.h"

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

struct xMovePoint : xBase
{
    xMovePointAsset* asset;
    xVec3* pos;
    xMovePoint** nodes;
    xMovePoint* prev;
    U32 node_wt_sum;
    U8 on;
    U8 pad[2];
    F32 delay;
    xSpline3* spl;
};
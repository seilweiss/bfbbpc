#pragma once

#include "xBase.h"
#include "xSpline.h"

struct xMovePointAsset;

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
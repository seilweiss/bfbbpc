#pragma once

#include "xMovePointAsset.h"
#include "xBase.h"
#include "xSpline.h"

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

F32 xMovePointGetNext(const xMovePoint* m, const xMovePoint* prev, xMovePoint** next, xVec3* hdng);
const xVec3* xMovePointGetPos(const xMovePoint* m);

inline U16 xMovePointGetNumPoints(const xMovePoint* m)
{
    return m->asset->numPoints;
}

inline xMovePoint* xMovePointGetPoint(const xMovePoint* m, U16 n)
{
    return m->nodes[n];
}
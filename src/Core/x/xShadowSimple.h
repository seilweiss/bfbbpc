#pragma once

#include "xMath3.h"

struct xEnt;

struct xShadowSimplePoly
{
    xVec3 vert[3];
    xVec3 norm;
};

struct xShadowSimpleCache
{
    U16 flags;
    U8 alpha;
    U8 pad;
    U32 collPriority;
    xVec3 pos;
    xVec3 at;
    xEnt* castOnEnt;
    xShadowSimplePoly poly;
    F32 envHeight;
    F32 shadowHeight;
    U32 raster;
    F32 dydx;
    F32 dydz;
    xVec3 corner[4];
};

void xShadowSimple_Init();
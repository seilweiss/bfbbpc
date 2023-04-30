#pragma once

#include "xMath3.h"

struct zShrapnelAsset;
struct zFrag;
struct zFragAsset;
struct xModelInstance;

typedef void(*zShrapnelInitCallback)(zShrapnelAsset*, xModelInstance*, xVec3*, void(*)(zFrag*, zFragAsset*));

struct zShrapnelAsset
{
    S32 fassetCount;
    U32 shrapnelID;
    zShrapnelInitCallback initCB;
};
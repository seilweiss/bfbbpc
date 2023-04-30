#pragma once

#include "types.h"

struct xModelBucket;
struct xEnt;

struct zLODTable
{
    xModelBucket** baseBucket;
    F32 noRenderDist;
    xModelBucket** lodBucket[3];
    F32 lodDist[3];
};

zLODTable* zLOD_Get(xEnt* ent);
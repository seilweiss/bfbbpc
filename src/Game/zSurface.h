#pragma once

#include "xBase.h"
#include "xMath3.h"
#include "xSurface.h"

struct zSurfAssetBase;

struct zSurfacePropTexAnim
{
    U16 mode;
    F32 speed;
    F32 frame;
    U32 group;
    S32 group_idx;
    xBase* group_ptr;
};

struct zSurfacePropUVFX
{
    S32 mode;
    F32 rot;
    F32 rot_spd;
    F32 minmax_timer[2];
    xVec3 trans;
    xVec3 trans_spd;
    xVec3 scale;
    xVec3 scale_spd;
    xVec3 min;
    xVec3 max;
    xVec3 minmax_spd;
};

struct zSurfaceProps
{
    zSurfAssetBase* asset;
    U32 texanim_flags;
    zSurfacePropTexAnim texanim[2];
    U32 uvfx_flags;
    zSurfacePropUVFX uvfx[2];
};

#define SURF_TEXANIM_ON (1<<0)
#define SURF_TEXANIM_ON2 (1<<1)

#define UVANIM_FLAG_ON (1<<0)
#define UVANIM_FLAG_ON2 (1<<1)

void zSurfaceRegisterMapper(U32 assetId);
xSurface* zSurfaceGetSurface(U32 mat_id);
U32 zSurfaceGetStandOn(const xSurface* surf);
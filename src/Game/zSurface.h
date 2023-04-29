#pragma once

#include "xSurface.h"
#include "xMath3.h"

struct zSurfAssetBase;
struct xCollis;
struct xScene;

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

struct zMaterialMapAsset
{
    U32 id;
    U32 count;
};

struct zMaterialMapEntry
{
    U32 surfaceAssetID;
    U32 materialIndex;
};

void zSurfaceInit();
void zSurfaceRegisterMapper(U32 assetId);
void zSurfaceExit();
void zSurfaceResetSurface(xSurface* surf);
xSurface* zSurfaceGetSurface(U32 mat_id);
xSurface* zSurfaceGetSurface(const xCollis* coll);
U32 zSurfaceGetSlide(const xSurface* surf);
U32 zSurfaceGetStep(const xSurface* surf);
bool zSurfaceOutOfBounds(const xSurface& s);
F32 zSurfaceGetSlideStartAngle(const xSurface* surf);
F32 zSurfaceGetSlideStopAngle(const xSurface* surf);
U32 zSurfaceGetMatchOrient(const xSurface* surf);
S32 zSurfaceGetDamageType(const xSurface* surf);
U32 zSurfaceGetDamagePassthrough(const xSurface* surf);
U32 zSurfaceGetSticky(const xSurface* surf);
U32 zSurfaceGetStandOn(const xSurface* surf);
F32 zSurfaceGetFriction(const xSurface* surf);
F32 zSurfaceGetOutOfBoundsDelay(const xSurface& s);
S32 zSurfaceGetSlickness(const xSurface* surf);
F32 zSurfaceGetDamping(const xSurface* surf, F32 min_vel);
void zSurfaceSave(xSurface* ent, xSerial* s);
void zSurfaceLoad(xSurface* ent, xSerial* s);
void zSurfaceSetup(xSurface* s);
void zSurfaceUpdate(xBase* to, xScene* sc, F32 dt);
void zSurfaceGetName(S32 type, char* buffer);
xSurface& zSurfaceGetDefault();
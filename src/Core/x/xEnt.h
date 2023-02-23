#pragma once

#include "xBase.h"
#include "xMath3.h"
#include "xModel.h"
#include "xCollide.h"
#include "xGrid.h"
#include "xBound.h"
#include "xShadowSimple.h"

struct xScene;
struct xFFX;

struct xEntFrame;
struct xEntCollis;
struct xEntShadow;
struct xEntAsset;
struct xEnt;

typedef xEnt*(*xEntCallback)(xEnt*, xScene*, void*);
typedef void(*xEntCollisPostCallback)(xEnt*, xScene*, F32, xEntCollis*);
typedef U32(*xEntCollisDepenQueryCallback)(xEnt*, xEnt*, xScene*, F32, xCollis*);
typedef void(*xEntUpdateCallback)(xEnt*, xScene*, F32);
typedef void(*xEntEndUpdateCallback)(xEnt*, xScene*, F32);
typedef void(*xEntBoundUpdateCallback)(xEnt*, xVec3*);
typedef void(*xEntMoveCallback)(xEnt*, xScene*, F32, xEntFrame*);
typedef void(*xEntRenderCallback)(xEnt*);
typedef void(*xEntTranslateCallback)(xEnt*, xVec3*, xMat4x3*);

struct xEntFrame
{
    xMat4x3 mat;
    xMat4x3 oldmat;
    xVec3 oldvel;
    xRot oldrot;
    xRot drot;
    xRot rot;
    xVec3 dpos;
    xVec3 dvel;
    xVec3 vel;
    U32 mode;
};

// Frame modes (xEntFrame::mode)
#define k_XENT_MODE_0x2 ((U32)(1 << 1))
#define k_XENT_MODE_0x8 ((U32)(1 << 3))
#define k_XENT_SET_ROT ((U32)(1 << 4))
#define k_XENT_MODE_0x20 ((U32)(1 << 5))
#define k_XENT_MODE_0x400 ((U32)(1 << 10))
#define k_XENT_MODE_0x800 ((U32)(1 << 11))
#define k_XENT_MODE_0x1000 ((U32)(1 << 12))
#define k_XENT_MODE_0x10000 ((U32)(1 << 16))
#define k_XENT_MODE_0x20000 ((U32)(1 << 17))

struct xEntCollis
{
    U8 chk;
    U8 pen;
    U8 env_sidx;
    U8 env_eidx;
    U8 npc_sidx;
    U8 npc_eidx;
    U8 dyn_sidx;
    U8 dyn_eidx;
    U8 stat_sidx;
    U8 stat_eidx;
    U8 idx;
    xCollis colls[18];
    xEntCollisPostCallback post;
    xEntCollisDepenQueryCallback depenq;
};

struct xEntShadow
{
    xVec3 pos;
    xVec3 vec;
    RpAtomic* shadowModel;
    F32 dst_cast;
    F32 radius[2];
};

struct xEntAsset : xBaseAsset
{
    U8 flags;
    U8 subtype;
    U8 pflags;
    U8 moreFlags;
    U8 pad;
    U32 surfaceID;
    xVec3 ang;
    xVec3 pos;
    xVec3 scale;
    F32 redMult;
    F32 greenMult;
    F32 blueMult;
    F32 seeThru;
    F32 seeThruSpeed;
    U32 modelInfoID;
    U32 animListID;
};

struct xEnt : xBase
{
    struct anim_coll_data;

    xEntAsset* asset;
    U16 idx;
    U16 num_updates;
    U8 flags;
    U8 miscflags;
    U8 subType;
    U8 pflags;
    U8 moreFlags;
    U8 isCulled;
    U8 driving_count;
    U8 num_ffx;
    U8 collType;
    U8 collLev;
    U8 chkby;
    U8 penby;
    xModelInstance* model;
    xModelInstance* collModel;
    xModelInstance* camcollModel;
    xLightKit* lightKit;
    xEntUpdateCallback update;
    xEntEndUpdateCallback endUpdate;
    xEntBoundUpdateCallback bupdate;
    xEntMoveCallback move;
    xEntRenderCallback render;
    xEntFrame* frame;
    xEntCollis* collis;
    xGridBound gridb;
    xBound bound;
    xEntTranslateCallback transl;
    xFFX* ffx;
    xEnt* driver;
    S32 driveMode;
    xShadowSimpleCache* simpShadow;
    xEntShadow* entShadow;
    anim_coll_data* anim_coll;
    void* user_data;
};

// Ent flags (xEnt::flags)
#define k_XENT_IS_VISIBLE ((U8)(1 << 0))
#define k_XENT_IS_STACKED ((U8)(1 << 1))
#define k_XENT_0x40 ((U8)(1 << 6))
#define k_XENT_0x80 ((U8)(1 << 7))

// Physics flags (xEnt::pflags)
#define k_XENT_IS_MOVING ((U8)(1 << 0))
#define k_XENT_HAS_VELOCITY ((U8)(1 << 1))
#define k_XENT_HAS_GRAVITY ((U8)(1 << 2))
#define k_XENT_HAS_DRAG ((U8)(1 << 3))
#define k_XENT_HAS_FRICTION ((U8)(1 << 4))

// More ent flags (xEnt::moreFlags)
#define k_MORE_FLAGS_HITTABLE ((U8)1<<4)
#define k_MORE_FLAGS_ANIM_COLL ((U8)1<<5)

// Collision types (xEnt::collType)
#define k_XENT_COLLTYPE_TRIG ((U8)(1 << (0)))
#define k_XENT_COLLTYPE_STAT ((U8)(1 << (1)))
#define k_XENT_COLLTYPE_DYN ((U8)(1 << (2)))
#define k_XENT_COLLTYPE_NPC ((U8)(1 << (3)))
#define k_XENT_COLLTYPE_PC ((U8)(1 << (4)))
#define k_XENT_COLLTYPE_ENV ((U8)(1 << (5)))

static const U32 k_XENT_MAX_COLL = 18;

extern S32 sSetPipeline;
extern S32 xent_entent;

void xEntSetTimePassed(F32 sec);
void xEntSceneInit();
void xEntSceneExit();
void xEntInit(xEnt* ent, xEntAsset* asset);
void xEntInitForType(xEnt* ent);
void xEntSetup(xEnt* ent);
void xEntSave(xEnt* ent, xSerial* s);
void xEntLoad(xEnt* ent, xSerial* s);
void xEntReset(xEnt* ent);
xModelInstance* xEntLoadModel(xEnt* ent, RpAtomic* imodel);
void xEntSetupPipeline(xModelInstance* model);
void xEntSetupPipeline(xSurface* surf, RpAtomic* model);
void xEntRestorePipeline(xModelInstance* model);
void xEntRestorePipeline(xSurface* surf, RpAtomic* model);
void xEntRender(xEnt* ent);
void xEntUpdate(xEnt* ent, xScene* sc, F32 dt);
void xEntBeginUpdate(xEnt* ent, xScene* sc, F32 dt);
void xEntEndUpdate(xEnt* ent, xScene* sc, F32 dt);
void xEntDefaultBoundUpdate(xEnt* ent, xVec3* pos);
void xEntDefaultTranslate(xEnt* ent, xVec3* dpos, xMat4x3* dmat);
void xEntMotionToMatrix(xEnt* ent, xEntFrame* frame);
void xEntMove(xEnt* ent, xScene* sc, F32 dt);
void xEntApplyPhysics(xEnt* ent, xScene* sc, F32 dt);
void xEntCollide(xEnt* ent, xScene* sc, F32 dt);
void xEntBeginCollide(xEnt* ent, xScene* sc, F32 dt);
void xEntEndCollide(xEnt* ent, xScene* sc, F32 dt);
void xEntCollCheckEnv(xEnt* p, xScene* sc);
void xEntCollCheckByGrid(xEnt* p, xScene* sc, xEntCallback hitIt);
void xEntCollCheckNPCsByGrid(xEnt* p, xScene* sc, xEntCallback hitIt);
void xEntCollCheckStats(xEnt* p, xScene* sc, xEntCallback hitIt);
void xEntCollCheckDyns(xEnt* p, xScene* sc, xEntCallback hitIt);
void xEntCollCheckNPCs(xEnt* p, xScene* sc, xEntCallback hitIt);
xEnt* xEntCollCheckOneEntNoDepen(xEnt* ent, xScene* sc, void* data);
void xEntCollideFloor(xEnt* p, xScene* sc, F32 dt);
void xEntCollideCeiling(xEnt* p, xScene* sc, F32 dt);
void xEntCollideWalls(xEnt* p, xScene* sc, F32 dt);
void xEntSetNostepNormAngle(F32 angle);
xBox* xEntGetAllEntsBox();
void xEntAnimateCollision(xEnt& ent, bool on);
bool xEntValidType(U8 type);
void xEntReposition(xEnt& ent, const xMat4x3& mat);
void xEntInitShadow(xEnt& ent, xEntShadow& shadow);

inline xVec3* xEntGetCenter(const xEnt* ent)
{
    return (xVec3*)xBoundCenter(&ent->bound);
}

inline xVec3* xEntGetPos(const xEnt* ent)
{
    return &xModelGetFrame(ent->model)->pos;
}

inline xMat4x3* xEntGetFrame(const xEnt* ent)
{
    return xModelGetFrame(ent->model);
}

inline void xEntEnable(xEnt* ent)
{
    xBaseEnable(ent);
}

inline void xEntDisable(xEnt* ent)
{
    xBaseDisable(ent);
}

inline U32 xEntIsEnabled(const xEnt* ent)
{
    return xBaseIsEnabled(ent);
}

inline void xEntShow(xEnt* ent)
{
    ent->flags |= k_XENT_IS_VISIBLE;
}

inline void xEntHide(xEnt* ent)
{
    ent->flags &= ~k_XENT_IS_VISIBLE;
}

inline U32 xEntIsVisible(const xEnt* ent)
{
    return (ent->flags & (k_XENT_IS_VISIBLE | k_XENT_0x80)) == k_XENT_IS_VISIBLE;
}
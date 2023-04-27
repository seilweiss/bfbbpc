#pragma once

#include "xEntMotionAsset.h"
#include "xEnt.h"

struct xMovePoint;
struct xSpline3;

struct xEntERData
{
    xVec3 a;
    xVec3 b;
    xVec3 dir;
    F32 et;
    F32 wet;
    F32 rt;
    F32 wrt;
    F32 p;
    F32 brt;
    F32 ert;
    S32 state;
};

struct xEntOrbitData
{
    xVec3 orig;
    xVec3 c;
    F32 a;
    F32 b;
    F32 p;
    F32 w;
};

struct xEntSplineData
{
    S32 unknown;
};

struct xEntMPData
{
    F32 curdist;
    F32 speed;
    xMovePoint* dest;
    xMovePoint* src;
    xSpline3* spl;
    F32 dist;
    U32 padalign;
    xQuat aquat;
    xQuat bquat;
};

struct xEntMechData
{
    xVec3 apos;
    xVec3 bpos;
    xVec3 dir;
    F32 arot;
    F32 brot;
    F32 ss;
    F32 sr;
    S32 state;
    F32 tsfd;
    F32 trfd;
    F32 tsbd;
    F32 trbd;
    F32* rotptr;
};

struct xEntPenData
{
    xVec3 top;
    F32 w;
    xMat4x3 omat;
};

struct xEntMotion
{
    xEntMotionAsset* asset;
    U8 type;
    U8 pad;
    U16 flags;
    F32 t;
    F32 tmr;
    F32 d;
    union
    {
        xEntERData er;
        xEntOrbitData orb;
        xEntSplineData spl;
        xEntMPData mp;
        xEntMechData mech;
        xEntPenData pen;
    };
    xEnt* owner;
    xEnt* target;
};

void xEntMotionInit(xEntMotion* motion, xEnt* owner, xEntMotionAsset* asset);
void xEntMotionReset(xEntMotion* motion, xScene* sc);
void xEntMotionMove(xEntMotion* motion, xScene* sc, F32 dt, xEntFrame* frame);
void xEntMechForward(xEntMotion* motion);
void xEntMechReverse(xEntMotion* motion);
void xEntMotionTranslate(xEntMotion* motion, const xVec3* dpos, xMat4x3* dmat);
void xEntMotionDebugInit(U16 num_xems);
void xEntMotionDebugExit();

inline U32 xEntMotionIsStopped(const xEntMotion* motion)
{
    return motion->flags & k_XENTMOTION_STOPPED;
}

inline void xEntMotionStop(xEntMotion* motion)
{
    motion->flags |= k_XENTMOTION_STOPPED;
}

inline void xEntMotionRun(xEntMotion* motion)
{
    motion->flags &= (U16)~k_XENTMOTION_STOPPED;
}

inline U32 xEntERIsExtending(const xEntMotion* motion)
{
    return motion->t < motion->er.et;
}

inline U32 xEntERIsExtended(const xEntMotion* motion)
{
    return motion->t >= motion->er.et && motion->t < motion->er.brt;
}

inline U32 xEntERIsRetracting(const xEntMotion* motion)
{
    return motion->t >= motion->er.brt && motion->t < motion->er.ert;
}

inline U32 xEntERIsRetracted(const xEntMotion* motion)
{
    return motion->t >= motion->er.ert;
}

inline void xEntMPSetSpeed(xEntMotion* motion, F32 speed)
{
    motion->mp.speed = xmax(0.0f, speed);
}

inline void xEntMPAccelerate(xEntMotion* motion, F32 new_speed)
{
    motion->mp.speed = xmax(0.0f, motion->mp.speed + new_speed);
}
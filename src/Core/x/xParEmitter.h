#pragma once

#include "xPar.h"
#include "xParEmitterAsset.h"
#include "xBase.h"
#include "xModel.h"

struct xParGroup;
struct xParSys;

struct xParEmitter : xBase
{
    xParEmitterAsset* tasset;
    xParGroup* group;
    xParEmitterPropsAsset* prop;
    U8 rate_mode;
    F32 rate;
    F32 rate_time;
    F32 rate_fraction;
    F32 rate_fraction_cull;
    U8 emit_flags;
    U8 emit_pad[3];
    U8 rot[3];
    xModelTag tag;
    F32 oocull_distance_sqr;
    F32 distance_to_cull_sqr;
    void* attachTo;
    xParSys* parSys;
    void* emit_volume;
    xVec3 last_attach_loc;
};

struct xParEmitterCustomSettings : xParEmitterPropsAsset
{
    U32 custom_flags;
    U32 attachToID;
    xVec3 pos;
    xVec3 vel;
    F32 vel_angle_variation;
    U8 rot[3];
    U8 padding;
    F32 radius;
    F32 emit_interval_current;
    void* emit_volume;
};

xPar* xParEmitterEmitCustom(xParEmitter* p, F32 dt, xParEmitterCustomSettings* info);
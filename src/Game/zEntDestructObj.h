#pragma once

#include "xSFXAsset.h"

#include "zEnt.h"
#include "zParEmitter.h"
#include "zShrapnel.h"

struct zEntDestructObj;
struct zEntDestructObjAsset;

typedef void(*zEntDestructObjDestroyNotifyCallback)(zEntDestructObj&, void*);

struct zEntDestructObj : zEnt
{
    zEntDestructObjAsset* dasset;
    U32 state;
    U32 healthCnt;
    F32 fx_timer;
    zParEmitter* fx_emitter;
    F32 respawn_timer;
    U32 throw_target;
    zShrapnelAsset* shrapnel_destroy;
    zShrapnelAsset* shrapnel_hit;
    xModelInstance* base_model;
    xModelInstance* hit_model;
    xModelInstance* destroy_model;
    zEntDestructObjDestroyNotifyCallback destroy_notify;
    void* notify_context;
    xSFXAsset* sfx_destroy;
    xSFXAsset* sfx_hit;
};
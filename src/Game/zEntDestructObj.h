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

void zEntDestructObj_FindFX();
void zEntDestructObj_Init(void* ent, void* asset);
void zEntDestructObj_Init(zEntDestructObj* ent, xEntAsset* asset);
void zEntDestructObj_Move(zEntDestructObj*, xScene*, F32, xEntFrame*);
void zEntDestructObj_Update(zEntDestructObj* ent, xScene* sc, F32 dt);
void zEntDestructObj_Hit(zEntDestructObj* ent, U32 mask);
U32 zEntDestructObj_GetHit(zEntDestructObj* ent, U32 mask);
void zEntDestructObj_Save(zEntDestructObj* ent, xSerial* s);
void zEntDestructObj_Load(zEntDestructObj* ent, xSerial* s);
void zEntDestructObj_Setup(zEntDestructObj* ent);
void zEntDestructObj_Reset(zEntDestructObj* ent, xScene*);
U32 zEntDestructObj_isDestroyed(zEntDestructObj* ent);
void zEntDestructObj_DestroyFX(zEntDestructObj* o);
S32 zEntDestructObjEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget);
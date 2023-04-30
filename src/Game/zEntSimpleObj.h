#pragma once

#include "zEnt.h"

struct xSimpleObjAsset;

struct zEntSimpleObj : zEnt
{
    xSimpleObjAsset* sasset;
    U32 sflags;
    void* anim;
    F32 animTime;
};

struct zSimpleMgr
{
    xSphere worldBound;
    F32 lodDist[4];
    U16 entFlags;
    U8 lastlod;
    U8 padA;
    xModelBucket** lodBucket[4];
    RwMatrix* mat;
    zEntSimpleObj* ent;
    U32 padB;
};

void zEntSimpleObj_MgrInit(zEntSimpleObj** entList, U32 entCount);
void zEntSimpleObj_MgrUpdateRender(RpWorld*, F32 dt);
void zEntSimpleObj_MgrCustomUpdate(zScene* s, F32 dt);
void zEntSimpleObj_MgrCustomRender();
void zEntTrackPhysics_Init(void* ent, void* asset);
void zEntSimpleObj_Init(void* ent, void* asset);
void zEntSimpleObj_Init(zEntSimpleObj* ent, xEntAsset* asset, bool physparams);
void zEntSimpleObj_Update(zEntSimpleObj* ent, xScene* sc, F32 dt);
void zEntSimpleObj_Setup(zEntSimpleObj* ent);
void zEntSimpleObj_Save(zEntSimpleObj* ent, xSerial* s);
void zEntSimpleObj_Load(zEntSimpleObj* ent, xSerial* s);
void zEntSimpleObj_Reset(zEntSimpleObj* ent, xScene*);
S32 zEntSimpleObjEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget);
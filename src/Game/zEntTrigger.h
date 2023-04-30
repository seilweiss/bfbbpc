#pragma once

#include "zEnt.h"
#include "xEntAsset.h"

struct xTriggerAsset;

struct zEntTrigger : zEnt
{
    xMat4x3 triggerMatrix;
    xBox triggerBox;
    U32 entered;
};

enum eEntTriggerType
{
    eEntTriggerTypeBox,
    eEntTriggerTypeSphere,
    eEntTriggerTypeCylinder,
    eEntTriggerTypeCircle
};

void zEntTriggerInit(void* ent, void* asset);
void zEntTriggerInit(zEntTrigger* ent, xEntAsset* asset);
void zEntTriggerUpdate(zEntTrigger* trig, xScene* sc, F32 dt);
S32 zEntTriggerEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget);
void zEntTriggerSave(zEntTrigger* ent, xSerial* s);
void zEntTriggerLoad(zEntTrigger* ent, xSerial* s);
void zEntTriggerReset(zEntTrigger* ent);
bool zEntTriggerHitsSphere(const zEntTrigger& trig, const xSphere& o, const xVec3& dir);

inline const xTriggerAsset& zEntTriggerAsset(const zEntTrigger& trig)
{
    return *(xTriggerAsset*)(trig.asset + 1);
}
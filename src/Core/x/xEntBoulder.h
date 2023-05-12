#pragma once

#include "xEnt.h"

struct xEntBoulderAsset;
struct xBoulderGeneratorAsset;
struct xDynAsset;

struct xEntBoulder : xEnt
{
    xEntBoulderAsset* basset;
    xShadowSimpleCache simpShadow_embedded;
    xEntShadow entShadow_embedded;
    xVec3 localCenter;
    xVec3 vel;
    xVec3 rotVec;
    xVec3 force;
    xVec3 instForce;
    F32 angVel;
    F32 timeToLive;
    S32 hitpoints;
    F32 lastRolling;
    U32 rollingID;
    U8 collis_chk;
    U8 collis_pen;
    U8 pad1[2];
};

struct xBoulderGenerator : xBase
{
    xBoulderGeneratorAsset* bgasset;
    S32 numBoulders;
    S32 nextBoulder;
    xEntBoulder** boulderList;
    S32* boulderAges;
    U32 isMarker;
    void* objectPtr;
    F32 lengthOfInitVel;
    xVec3 perp1;
    xVec3 perp2;
};

void xEntBoulder_FitToModel(xEntBoulder* ent);
void xEntBoulder_Render(xEnt* ent);
void xEntBoulder_Init(void* ent, void* asset);
void xEntBoulder_Init(xEntBoulder* ent, xEntAsset* asset);
void xEntBoulder_ApplyForces(xEntCollis* collis);
void xEntBoulder_AddInstantForce(xEntBoulder* ent, xVec3* force);
void xEntBoulder_AddForce(xEntBoulder* ent, xVec3* force);
void xEntBoulder_BUpdate(xEnt*, xVec3*);
void xEntBoulder_RealBUpdate(xEnt* ent, xVec3* pos);
void xEntBoulder_Update(xEntBoulder* ent, xScene* sc, F32 dt);
S32 xEntBoulder_KilledBySurface(xEntBoulder* ent, xScene*, F32);
void xEntBoulder_Kill(xEntBoulder* ent);
void xEntBoulder_BubbleBowl(F32 multiplier);
void xEntBoulder_Setup(xEntBoulder* ent);
void xEntBoulder_Reset(xEntBoulder* ent, xScene*);
S32 xEntBoulderEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget);

void xBoulderGenerator_Init(xBase& data, xDynAsset& asset, size_t asset_size);
void xBoulderGenerator_Init(xBoulderGenerator* bg, xBoulderGeneratorAsset* asset);
void xBoulderGenerator_Reset(xBoulderGenerator* bg);
void xBoulderGenerator_Launch(xBoulderGenerator* bg, xVec3* pnt, F32 t);
S32 xBoulderGenerator_EventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget);
void xBoulderGenerator_GenBoulder(xBoulderGenerator* bg);
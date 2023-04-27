#pragma once

#include "xMovePoint.h"

struct zScene;

struct zMovePoint : xMovePoint
{
    S32 NumNodes() { return asset->numPoints; }
    zMovePoint* NodeByIndex(S32 idx) { return (zMovePoint*)nodes[idx]; }
    U32 IsOn() { return on; }
    const xVec3* PosGet() { return pos; }
    F32 Delay() { return asset->delay; }
    F32 RadiusZone() { return asset->zoneRadius; }
    F32 RadiusArena() { return asset->arenaRadius; }
    U32 HasSpline() { return spl != NULL; }
};

zMovePoint* zMovePoint_GetMemPool(S32 cnt);
void zMovePointInit(zMovePoint* m, xMovePointAsset* asset);
zMovePoint* zMovePoint_GetInst(S32 n);
void zMovePointSetup(zMovePoint* mvpt, zScene* scn);
zMovePoint* zMovePoint_From_xAssetID(U32 aid);
void zMovePointSave(zMovePoint* ent, xSerial* s);
void zMovePointLoad(zMovePoint* ent, xSerial* s);
void zMovePointReset(zMovePoint* ent);
S32 zMovePointEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget);
F32 zMovePointGetNext(const zMovePoint* current, const zMovePoint* prev, zMovePoint** next, xVec3* hdng);
const xVec3* zMovePointGetPos(const zMovePoint* m);
F32 zMovePointGetDelay(const zMovePoint* m);
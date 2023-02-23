#pragma once

#include "xEnt.h"
#include "xEnv.h"

typedef xBase* (*xSceneResolvIDCallback)(U32);
typedef char* (*xSceneBase2NameCallback)(xBase*);
typedef char* (*xSceneID2NameCallback)(U32);
typedef void(*xRayEntCallback)(xScene*, xRay3*, xQCData*, xEnt*, void*);

struct xScene
{
    U32 sceneID;
    U16 flags;
    U16 num_ents;
    U16 num_trigs;
    U16 num_stats;
    U16 num_dyns;
    U16 num_npcs;
    U16 num_act_ents;
    U16 num_nact_ents;
    F32 gravity;
    F32 drag;
    F32 friction;
    U16 num_ents_allocd;
    U16 num_trigs_allocd;
    U16 num_stats_allocd;
    U16 num_dyns_allocd;
    U16 num_npcs_allocd;
    xEnt** trigs;
    xEnt** stats;
    xEnt** dyns;
    xEnt** npcs;
    xEnt** act_ents;
    xEnt** nact_ents;
    xEnv* env;
    xMemPool mempool;
    xSceneResolvIDCallback resolvID;
    xSceneBase2NameCallback base2Name;
    xSceneID2NameCallback id2Name;
};

struct xNearFloorPoly
{
    xBox box;
    xVec3 center;
    xVec3 facingVec;
    F32 facingDot;
    F32 floorDot;
    xVec3 vert[3];
    F32 neardist;
    xVec3 nearpt;
    S32 nearvert;
    S32 nearedge;
    U32 oid;
    void* optr;
    xModelInstance* mptr;
};

#define k_XSCENE_HAS_GRAVITY ((U16)(1<<0))
#define k_XSCENE_HAS_FRICTION ((U16)(1<<1))
#define k_XSCENE_HAS_DRAG ((U16)(1<<2))

extern xScene* g_xSceneCur;

void xSceneInit(xScene* sc, U16 num_trigs, U16 num_stats, U16 num_dyns, U16 num_npcs);
void xSceneExit(xScene* sc);
void xSceneSave(xScene* ent, xSerial* s);
void xSceneLoad(xScene* ent, xSerial* s);
void xSceneSetup(xScene* sc);
void xSceneAddEnt(xScene* sc, xEnt* ent);
xBase* xSceneResolvID(xScene* sc, U32 id);
char* xSceneID2Name(xScene* sc, U32 id);
void xSceneForAllEnts(xScene* sc, xEntCallback func, void* data);
void xSceneForAllStatics(xScene* sc, xEntCallback func, void* data);
void xSceneForAllDynamics(xScene* sc, xEntCallback func, void* data);
void xSceneForAllNPCs(xScene* sc, xEntCallback func, void* data);
void xRayHitsGrid(xGrid* grid, xScene* sc, xRay3* r, xRayEntCallback rentcb, xQCData* qcr, void* data);
void xRayHitsTikiLandableEnt(xScene* sc, xRay3* r, xQCData* qcr, xEnt* ent, void* colldata);
void xRayHitsEnt(xScene* sc, xRay3* r, xQCData* qcr, xEnt* ent, void* colldata);
void xRayHitsTikiLandableScene(xScene* sc, xRay3* r, xCollis* coll);
void xRayHitsScene(xScene* sc, xRay3* r, xCollis* coll);
void xRayHitsSceneFlags(xScene* sc, xRay3* r, xCollis* coll, U8 collType, U8 chk);
U32 xSceneNearestFloorPoly(xScene* sc, xNearFloorPoly* nfpoly, U8 collType, U8 chk);
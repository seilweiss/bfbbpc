#pragma once

#include "xEnt.h"

struct zNPCCommon;

enum en_NPC_UI_WIDGETS
{
    NPC_WIDGE_TALK,
    NPC_WIDGE_NOMORE,
    NPC_WIDGE_FORCE
};

struct NPCWidget
{
    en_NPC_UI_WIDGETS idxID;
    xBase* base_widge;
    zNPCCommon* npc_ownerlock;

    S32 Off(const zNPCCommon* npc, S32 theman);
    S32 IsVisible();
};

void NPCSupport_Startup();
void NPCSupport_Shutdown();
void NPCSupport_ScenePrepare();
void NPCSupport_SceneFinish();
void NPCSupport_ScenePostInit();
void NPCSupport_SceneReset();
void NPCSupport_Timestep(F32 dt);
NPCWidget* NPCWidget_Find(en_NPC_UI_WIDGETS which);
F32 NPCC_dir_toXZAng(const xVec3* dir);
S32 NPCC_pos_ofBase(xBase* tgt, xVec3* pos);
void NPCC_xBoundAway(xBound* bnd);
void NPCC_xBoundBack(xBound* bnd);
F32 NPCC_ds2_toCam(const xVec3* pos_from, xVec3* delta);
S32 NPCC_LampStatus();
U32 NPCC_ForceTalkOk();

inline xVec3* NPCC_faceDir(xEnt* ent) { return (xVec3*)&ent->model->Mat->at; }
inline xVec3* NPCC_rightDir(xEnt* ent) { return (xVec3*)&ent->model->Mat->right; }
inline xVec3* NPCC_upDir(xEnt* ent) { return (xVec3*)&ent->model->Mat->up; }

inline F32 SQ(F32 x) { return x * x; }
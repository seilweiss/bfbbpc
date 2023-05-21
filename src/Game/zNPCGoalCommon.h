#pragma once

#include "xBehaviour.h"

struct NPCMsg;

enum en_NPC_GOAL_SPOT
{
    NPC_GSPOT_START = 32,
    NPC_GSPOT_RESUME,
    NPC_GSPOT_FINISH,
    NPC_GSPOT_STARTALT,
    NPC_GSPOT_ALTA,
    NPC_GSPOT_ALTB,
    NPC_GSPOT_PATROLPAUSE,
    NPC_GSPOT_NOMORE,
    NPC_GSPOT_FORCEINT = FORCEENUMSIZEINT
};

struct zNPCGoalCommon : xGoal
{
protected:
    S32 flg_npcgauto;
    S32 flg_npcgable;
    U32 anid_played;
    S32 flg_info : 16;
    S32 flg_user : 16;

public:
    zNPCGoalCommon(S32 myType) : xGoal(myType)
    {
        this->flg_npcgauto |= ~0x8;
    }

    const char* Name() { return NULL; }

    virtual void Clear()
    {
        this->flg_info = 0;
        xGoal::Clear();
    }

    virtual S32 Enter(F32 dt, void* updCtxt);
    virtual S32 Resume(F32 dt, void* updCtxt);
    virtual S32 PreCalc(F32 dt, void* updCtxt);
    virtual S32 NPCMessage(NPCMsg* mail) {}
    virtual S32 CollReview(void*) {}

    U32 DoAutoAnim(en_NPC_GOAL_SPOT gspot, S32 forceRestart);
    U32 DoExplicitAnim(U32 anid, S32 forceRestart);
};
#include "zNPCGoalCommon.h"

#include "xBehaveMgr.h"
#include "zNPCTypeCommon.h"

S32 zNPCGoalCommon::Enter(F32 dt, void* updCtxt)
{
    S32 gid = this->psyche->GIDOfPending();

    if ((this->flg_npcgauto & 0x2) && !gid || gid == this->GetID()) {
        this->DoAutoAnim(NPC_GSPOT_START, 0);
    }

    return xGoal::Enter(dt, updCtxt);
}

S32 zNPCGoalCommon::Resume(F32 dt, void* updCtxt)
{
    S32 gid = this->psyche->GIDOfPending();

    if ((this->flg_npcgauto & 0x2) && (this->flg_npcgauto & 0x4) && !gid || gid == this->GetID()) {
        this->DoAutoAnim(NPC_GSPOT_RESUME, 0);
    }

    return xGoal::Resume(dt, updCtxt);
}

S32 zNPCGoalCommon::PreCalc(F32 dt, void* updCtxt)
{
    zNPCCommon* npc = (zNPCCommon*)this->psyche->clt_owner;

    if (this->flg_npcgauto & 0x8) {
        if (!(this->flg_info |= 0x8)) {
            U32 curid = npc->AnimCurStateID();
            if (curid != this->anid_played) {
                this->Name();
                this->flg_info |= 0x8;
                if (this->psyche->cb_notice && this->psyche->cb_notice) {
                    this->psyche->cb_notice->Notice(PSY_NOTE_ANIMCHANGED, this, NULL);
                }
            }
        }
    }

    return xGoal::PreCalc(dt, updCtxt);
}

U32 zNPCGoalCommon::DoAutoAnim(en_NPC_GOAL_SPOT gspot, S32 forceRestart)
{
    U32 anid = ((zNPCCommon*)this->psyche->clt_owner)->AnimPick(this->goalID, gspot, this);
    if (anid) {
        this->DoExplicitAnim(anid, forceRestart);
    }

    return this->anid_played;
}

U32 zNPCGoalCommon::DoExplicitAnim(U32 anid, S32 forceRestart)
{
    S32 rc = ((zNPCCommon*)this->psyche->clt_owner)->AnimStart(anid, forceRestart);
    if (rc) {
        this->anid_played = anid;
    } else {
        this->Name();
        this->anid_played = 0;
    }

    if (this->flg_npcgauto & 0x8) {
        this->flg_info &= ~0x8;
    }

    return this->anid_played;
}
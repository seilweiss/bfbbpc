#pragma once

#include "xRMemData.h"
#include "xBase.h"
#include "xBehaviour.h"

enum en_psynote
{
    PSY_NOTE_HASRESUMED,
    PSY_NOTE_HASENTERED,
    PSY_NOTE_ANIMCHANGED,
    PSY_NOTE_NOMORE,
    PSY_NOTE_FORCE = FORCEENUMSIZEINT
};

struct xPSYNote
{
    xPSYNote() {}
    virtual void Notice(en_psynote note, xGoal* goal, void*) {}
};

enum en_pendtype
{
    PEND_TRAN_NONE,
    PEND_TRAN_SET,
    PEND_TRAN_PUSH,
    PEND_TRAN_POP,
    PEND_TRAN_POPTO,
    PEND_TRAN_POPALL,
    PEND_TRAN_SWAP,
    PEND_TRAN_INPROG,
    PEND_TRAN_NOMORE
};

enum PSY_BRAIN_STATUS
{
    PSY_STAT_BLANK,
    PSY_STAT_GROW,
    PSY_STAT_EXTEND,
    PSY_STAT_THINK,
    PSY_STAT_NOMORE,
    PSY_STAT_FORCE = FORCEENUMSIZEINT
};

enum en_xpsytime
{
    XPSY_TYMR_CURGOAL,
    XPSY_TYMR_NOMORE
};

struct xPsyche : RyzMemData
{
    xBase* clt_owner;
    xPSYNote* cb_notice;
    S32 flg_psyche;
    xGoal* goallist;
    xGoal* goalstak[5];
    F32 tmr_stack[5][1];
    S32 staktop;
    xGoal* pendgoal;
    en_pendtype pendtype;
    S32 gid_safegoal;
    void(*fun_remap)(S32*, en_trantype*);
    void* userContext;
    S32 cnt_transLastTimestep;
    PSY_BRAIN_STATUS psystat;
    xBase fakebase;

    void BrainBegin();
    void BrainExtend();
    void BrainEnd();
    xGoal* AddGoal(S32 gid, void* createData);
    void FreshWipe();
    xBase* GetClient() { return clt_owner; }
    void SetOwner(xBase* owner, void* ctxt);
    void SetNotify(xPSYNote* note) { cb_notice = note; }
    void KillBrain(xFactory*);
    void Lobotomy(xFactory*);
    void Amnesia(S32);
    S32 IndexInStack(S32 gid) const;
    S32 IndexInStack(const xGoal* goal) const { return IndexInStack(goal->GetID()); }
    xGoal* GetCurGoal() const;
    xGoal* GIDInStack(S32 gid) const;
    S32 GIDOfActive() const;
    S32 GIDOfPending() const;
    S32 GIDOfSafety() const { return gid_safegoal; }
    void SetSafety(S32 gid) { gid_safegoal = gid; }
    xGoal* GetPrevRecovery(S32 gid) const;
    S32 GoalSet(S32 gid, S32);
    S32 GoalPush(S32 gid, S32);
    S32 GoalPopToBase(S32 overpend);
    S32 GoalPopRecover(S32 overpend);
    S32 GoalPop(S32 gid_popto, S32);
    S32 GoalSwap(S32 gid, S32);
    S32 GoalNone(S32, S32 denyExplicit);
    void SetTopState(en_GOALSTATE state);
    xGoal* FindGoal(S32 gid);
    S32 HasGoal(S32 gid) { return FindGoal(gid) != NULL; }
    void ForceTran(F32 dt, void* updCtxt);
    S32 Timestep(F32 dt, void* updCtxt);
    S32 ParseTranRequest(en_trantype trantyp, S32 trangid);
    S32 TranGoal(F32 dt, void* updCtxt);
    F32 TimerGet(en_xpsytime tymr);
    void TimerClear();
    void TimerUpdate(F32);

    void ImmTranOn() { flg_psyche |= 0x1; }
    void ImmTranOff() { flg_psyche &= ~0x1; }
    S32 ImmTranIsOn() { return (flg_psyche & 0x1); }
    void ExpTranOn() { flg_psyche &= ~0x4; }
    void ExpTranOff() { flg_psyche |= 0x4; }
    S32 ExpTranIsOn() { return !(flg_psyche & 0x4); }

    void DBG_HistAdd(S32) {}
};

struct xBehaveMgr : RyzMemData
{
protected:
    xFactory* goalFactory;
    xPsyche* psypool;
    st_XORDEREDARRAY psylist;

public:
    xBehaveMgr() {}
    ~xBehaveMgr() {}

    xFactory* GetFactory() { return goalFactory; }
    void Startup(S32 maxPsyches, S32 maxTypes);
    void RegBuiltIn();
    xPsyche* Subscribe(xBase* owner, S32);
    void UnSubscribe(xPsyche* psy);
    void ScenePrepare();
    void SceneFinish();
    void SceneReset();
};

void xBehaveMgr_Startup();
void xBehaveMgr_Shutdown();
xBehaveMgr* xBehaveMgr_GetSelf();
xFactory* xBehaveMgr_GoalFactory();
void xBehaveMgr_ScenePrepare();
void xBehaveMgr_SceneFinish();
void xBehaveMgr_SceneReset();
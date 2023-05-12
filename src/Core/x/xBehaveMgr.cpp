#include "xBehaveMgr.h"

#include "xBehaveGoalSimple.h"
#include "xMemMgr.h"
#include "xutil.h"

#include <string.h>

static S32 g_modinit;
static xBehaveMgr* g_behavmgr;

void xBehaveMgr_Startup()
{
    if (!g_modinit++) {
        g_behavmgr = new ('BMGR', NULL) xBehaveMgr();
        g_behavmgr->Startup(250, 250);
    }
}

void xBehaveMgr_Shutdown()
{
    if (!--g_modinit) {
        if (g_behavmgr) delete g_behavmgr;
        g_behavmgr = NULL;
    }
}

xBehaveMgr* xBehaveMgr_GetSelf()
{
    return g_behavmgr;
}

xFactory* xBehaveMgr_GoalFactory()
{
    return g_behavmgr->GetFactory();
}

void xBehaveMgr_ScenePrepare()
{
    g_behavmgr->ScenePrepare();
}

void xBehaveMgr_SceneFinish()
{
    g_behavmgr->SceneFinish();
}

void xBehaveMgr_SceneReset()
{
    g_behavmgr->SceneReset();
}

void xBehaveMgr::Startup(S32 maxPsyches, S32 maxTypes)
{
    goalFactory = new ('BMGR', NULL) xFactory(maxTypes);

    g_behavmgr->RegBuiltIn();

    XOrdInit(&psylist, maxPsyches, 0);

    U32 amt = maxPsyches * sizeof(xPsyche);
    psypool = (xPsyche*)xMALLOC(amt);
    memset(psypool, 0, amt);
}

void xBehaveMgr::RegBuiltIn()
{
    xGoalSimple_RegisterTypes(goalFactory);
}

xPsyche* xBehaveMgr::Subscribe(xBase* owner, S32)
{
    xPsyche* psy = &psypool[psylist.cnt];

    XOrdAppend(&psylist, psy);

    psy->FreshWipe();
    psy->SetOwner(owner, NULL);

    return psy;
}

void xBehaveMgr::UnSubscribe(xPsyche* psy)
{
    psy->KillBrain(goalFactory);

    XOrdRemove(&psylist, psy, -1);
}

void xBehaveMgr::ScenePrepare()
{
}

void xBehaveMgr::SceneFinish()
{
    XOrdReset(&psylist);
}

void xBehaveMgr::SceneReset()
{
    for (S32 i = 0; i < psylist.cnt; i++) {
        xPsyche* psy = (xPsyche*)psylist.list[i];
        psy->Amnesia(0);
    }
}

void xPsyche::BrainBegin()
{
    xFactory* factory = xBehaveMgr_GoalFactory();

    psystat = PSY_STAT_GROW;
    factory->GrowDataEnable(&fakebase, 0);
}

void xPsyche::BrainExtend()
{
    xFactory* factory = xBehaveMgr_GoalFactory();

    psystat = PSY_STAT_EXTEND;
    factory->GrowDataEnable(&fakebase, 1);
}

void xPsyche::BrainEnd()
{
    xFactory* factory = xBehaveMgr_GoalFactory();

    factory->GrowDataDisable();
    psystat = PSY_STAT_THINK;
}

xGoal* xPsyche::AddGoal(S32 gid, void* createData)
{
    xFactory* factory = xBehaveMgr_GoalFactory();
    xGoal* goal = (xGoal*)factory->CreateItem(gid, createData, NULL);

    if (goal) {
        if (goallist) {
            goal->Insert(goallist);
        } else {
            goallist = goal;
        }

        goal->SetPsyche(this);
    } else {
        xUtil_idtag2string(gid);
    }

    return goal;
}

void xPsyche::FreshWipe() NONMATCH("https://decomp.me/scratch/m2kHE")
{
    S32 i;

    for (i = 0; i < 5; i++) {
        goalstak[i] = NULL;
    }

    staktop = -1;
    gid_safegoal = 0;
    pendgoal = NULL;
    pendtype = PEND_TRAN_NONE;

    for (i = 0; i < 5; i++) {
        tmr_stack[i][0] = 0.0f;
    }

    clt_owner = NULL;
    userContext = NULL;
    fun_remap = NULL;
    cb_notice = NULL;
    psystat = PSY_STAT_BLANK;
    flg_psyche |= 0x1;
}

void xPsyche::SetOwner(xBase* owner, void* ctxt)
{
    clt_owner = owner;
    userContext = ctxt;

    if (owner) {
        fakebase.id = owner->id;
        fakebase.baseType = owner->baseType + 128;
        fakebase.linkCount = 0;
        fakebase.baseFlags = 0;
        fakebase.link = NULL;
        fakebase.eventFunc = NULL;
    }
}

void xPsyche::KillBrain(xFactory* factory)
{
    Lobotomy(factory);

    fun_remap = NULL;
}

void xPsyche::Lobotomy(xFactory* factory)
{
    while (goallist) {
        xGoal* goal = goallist->RemHead(&goallist);
        factory->DestroyItem(goal);
    }
}

void xPsyche::Amnesia(S32 r29)
{
    xGoal* r31 = goallist;

    while (r31) {
        xGoal* r30 = r31;

        r31 = r31->Next();

        if (r29 || GIDInStack(r30->GetID()) == 0) {
            r30->Clear();
        }
    }
}

S32 xPsyche::IndexInStack(S32 gid) const NONMATCH("https://decomp.me/scratch/lsj2N")
{
    S32 top = staktop;
    S32 da_idx = -1;

    for (S32 i = 0; i <= top; i++) {
        if (gid == goalstak[i]->GetID()) {
            da_idx = i;
            break;
        }
    }

    return da_idx;
}

xGoal* xPsyche::GetCurGoal() const
{
    if (staktop < 0) return NULL;
    return goalstak[staktop];
}

xGoal* xPsyche::GIDInStack(S32 gid) const NONMATCH("https://decomp.me/scratch/pc2t1")
{
    S32 top = staktop;
    xGoal* da_goal = NULL;
    xGoal* tmpgoal;

    for (S32 i = 0; i <= top; i++) {
        tmpgoal = goalstak[i];
        if (gid == tmpgoal->GetID()) {
            da_goal = tmpgoal;
            break;
        }
    }

    return da_goal;
}

S32 xPsyche::GIDOfActive() const
{
    if (staktop < 0) return 0;
    return goalstak[staktop]->GetID();
}

S32 xPsyche::GIDOfPending() const
{
    if (pendgoal) return pendgoal->GetID();
    return 0;
}

xGoal* xPsyche::GetPrevRecovery(S32 gid) const
{
    S32 i;
    S32 idx_start = -1;
    xGoal* tmpgoal;
    xGoal* recgoal = NULL;

    if (gid == 0) {
        for (i = staktop; i >= 0; i--) {
            tmpgoal = goalstak[i];
            if (tmpgoal->GetFlags() & 0x8) {
                recgoal = tmpgoal;
                break;
            }
        }
    } else {
        for (i = staktop; i >= 0; i--) {
            tmpgoal = goalstak[i];
            if (gid == tmpgoal->GetID()) {
                idx_start = i - 1;
                break;
            }
        }

        if (idx_start > 0) {
            for (i = idx_start; i >= 0; i--) {
                tmpgoal = goalstak[i];
                if (tmpgoal->GetFlags() & 0x8) {
                    recgoal = tmpgoal;
                    break;
                }
            }
        }
    }

    return recgoal;
}

S32 xPsyche::GoalSet(S32 gid, S32 r30)
{
    S32 result = 0;
    xGoal* goal;
    en_trantype trantype;

    if (flg_psyche & 0x4) {
        return 0;
    }

    if (fun_remap) {
        trantype = GOAL_TRAN_SET;
        fun_remap(&gid, &trantype);

        if (trantype != GOAL_TRAN_SET) {
            return 0;
        }
    }

    if (gid_safegoal == 0) {
        SetSafety(gid);
    }

    if (pendgoal && !r30) {
        pendgoal->Name();
    }

    goal = FindGoal(gid);
    if (goal) {
        result = 1;
        pendgoal = goal;
        pendtype = PEND_TRAN_SET;
        SetTopState(GOAL_STAT_EXIT);
    }

    if (result && pendtype != PEND_TRAN_NONE && (flg_psyche & 0x1)) {
        ForceTran(0.01f, NULL);
    }

    return result;
}

S32 xPsyche::GoalPush(S32 gid, S32 r29)
{
    S32 result = 0;
    xGoal* goal;
    en_trantype trantype;

    if (flg_psyche & 0x4) {
        return 0;
    }

    if (fun_remap) {
        trantype = GOAL_TRAN_PUSH;
        fun_remap(&gid, &trantype);

        if (trantype != GOAL_TRAN_PUSH) {
            return 0;
        }
    }

    if (pendgoal && !r29) {
        pendgoal->Name();
    }

    if (staktop >= 4) {
        return 0;
    }

    if (staktop >= 0 && !(goalstak[staktop]->GetFlags() & 0x4)) {
        return 0;
    }

    goal = FindGoal(gid);
    if (goal && (goal->GetFlags() & 0x2)) {
        result = 1;
        pendgoal = goal;
        pendtype = PEND_TRAN_PUSH;
        SetTopState(GOAL_STAT_SUSPEND);
    }

    if (result && pendtype != PEND_TRAN_NONE && (flg_psyche & 0x1)) {
        ForceTran(0.01f, NULL);
    }

    return result;
}

S32 xPsyche::GoalPopToBase(S32 overpend)
{
    if (flg_psyche & 0x4) {
        return 0;
    }

    if (staktop < 1) {
        return 0;
    }

    GoalPop(goalstak[0]->GetID(), overpend);

    if (pendtype != PEND_TRAN_NONE && (flg_psyche & 0x1)) {
        ForceTran(0.01f, NULL);
    }

    return 1;
}

S32 xPsyche::GoalPopRecover(S32 overpend)
{
    S32 result = 0;
    S32 i;
    xGoal* tmpgoal;
    xGoal* destgoal = NULL;

    if (flg_psyche & 0x4) {
        return 0;
    }

    if (staktop < 1) {
        return 0;
    }

    for (i = staktop - 1; i >= 0; i--) {
        tmpgoal = goalstak[i];

        if (tmpgoal->GetFlags() & 0x8) {
            destgoal = tmpgoal;
            break;
        }
    }

    if (destgoal) {
        GoalPop(destgoal->GetID(), overpend);
        result = 1;
    }

    if (result && pendtype != PEND_TRAN_NONE && (flg_psyche & 0x1)) {
        ForceTran(0.01f, NULL);
    }

    return result;
}

S32 xPsyche::GoalPop(S32 gid_popto, S32 r5)
{
    S32 result = 0;
    xGoal* destgoal = NULL;
    xGoal* tmpgoal;
    S32 i;
    en_trantype trantype;

    if (flg_psyche & 0x4) {
        return 0;
    }

    if (pendgoal && !r5) {
        pendgoal->Name();
    }

    if (gid_popto != 0 && fun_remap) {
        trantype = GOAL_TRAN_POPTO;
        fun_remap(&gid_popto, &trantype);

        if (trantype != GOAL_TRAN_POPTO) {
            return 0;
        }
    }

    if (staktop < 0) {
        return 0;
    }

    if (staktop < 1) {
        result = 0;
    } else if (gid_popto == 0) {
        pendgoal = NULL;
        pendtype = PEND_TRAN_POP;
        SetTopState(GOAL_STAT_EXIT);
        result = 1;
    } else {
        if (gid_popto == goalstak[staktop]->GetID()) {
            destgoal = NULL;
        } else {
            for (i = 0; i < staktop; i++) {
                tmpgoal = goalstak[i];
                if (gid_popto == tmpgoal->GetID()) {
                    destgoal = tmpgoal;
                    break;
                }
            }
        }

        if (destgoal) {
            pendgoal = destgoal;
            pendtype = PEND_TRAN_POPTO;
            SetTopState(GOAL_STAT_EXIT);
            result = 1;
        }
    }

    if (result && pendtype != PEND_TRAN_NONE && (flg_psyche & 0x1)) {
        ForceTran(0.01f, NULL);
    }

    return result;
}

S32 xPsyche::GoalSwap(S32 gid, S32 r30)
{
    S32 result = 0;
    xGoal* goal;
    en_trantype trantype;

    if (flg_psyche & 0x4) {
        return 0;
    }

    if (fun_remap) {
        trantype = GOAL_TRAN_SWAP;
        fun_remap(&gid, &trantype);

        if (trantype != GOAL_TRAN_SWAP) {
            return 0;
        }
    }

    if (pendgoal && !r30) {
        pendgoal->Name();
    }

    goal = FindGoal(gid);
    if (goal) {
        result = 1;
        pendgoal = goal;
        pendtype = PEND_TRAN_SWAP;
        SetTopState(GOAL_STAT_EXIT);
    }

    if (result && pendtype != PEND_TRAN_NONE && (flg_psyche & 0x1)) {
        ForceTran(0.01f, NULL);
    }

    return result;
}

S32 xPsyche::GoalNone(S32 r4, S32 denyExplicit)
{
    if (flg_psyche & 0x4) {
        return 0;
    }

    if (!r4 && pendgoal) {
        pendgoal->Name();
    }

    pendgoal = NULL;

    if (staktop < 0) {
        pendtype = PEND_TRAN_NONE;
    } else {
        pendtype = PEND_TRAN_POPALL;
        SetTopState(GOAL_STAT_EXIT);
    }

    if (pendtype != PEND_TRAN_NONE && (flg_psyche & 0x1)) {
        if (denyExplicit) {
            S32 r31 = ExpTranIsOn();

            ExpTranOff();
            ForceTran(0.01f, NULL);

            if (r31) {
                ExpTranOn();
            }
        } else {
            ForceTran(0.01f, NULL);
        }
    }

    return 1;
}

void xPsyche::SetTopState(en_GOALSTATE state)
{
    if (staktop >= 0) {
        goalstak[staktop]->SetState(state);
    }
}

xGoal* xPsyche::FindGoal(S32 gid)
{
    xGoal* goal = goallist;
    xGoal* safe = NULL;

    while (goal) {
        if (gid_safegoal == goal->GetID()) {
            safe = goal;
        }

        if (gid == goal->GetID()) {
            break;
        }

        goal = goal->Next();
    }

    if (!goal) {
        xUtil_idtag2string(gid);

        if (safe) {
            safe->Name();
        } else {
            xUtil_idtag2string(gid_safegoal);
        }
    }

    if (!goal) {
        goal = safe;
    }

    return goal;
}

void xPsyche::ForceTran(F32 dt, void* updCtxt)
{
    flg_psyche |= 0x2;
    Timestep(dt, updCtxt ? updCtxt : userContext);
    flg_psyche &= ~0x2;
}

S32 xPsyche::Timestep(F32 dt, void* updCtxt)
{
    xGoal* curgoal;
    S32 moretodo;
    S32 trangid;
    en_trantype trantyp = GOAL_TRAN_NONE;
    S32 old_psyflags = flg_psyche;

    flg_psyche &= ~0x1;

    if (flg_psyche & 0x2) {
        cnt_transLastTimestep++;
    } else {
        cnt_transLastTimestep = 0;
    }

    TimerUpdate(dt);

    do {
        trantyp = GOAL_TRAN_NONE;
        moretodo = 0;

        if (pendtype != PEND_TRAN_NONE) {
            moretodo = TranGoal(dt, updCtxt);
        }

        if (moretodo && pendtype == PEND_TRAN_POPALL) break;
        if (moretodo) break;
        if (pendtype != PEND_TRAN_NONE) continue;
        if (staktop < 0) break;

        curgoal = goalstak[staktop];

        if (curgoal->GetFlags() & 0x10000) {
            curgoal->PreCalc(dt, updCtxt);
            if (pendtype != PEND_TRAN_NONE) continue;
        }

        if (curgoal->GetFlags() & 0x20000) {
            trangid = curgoal->EvalRules(&trantyp, dt, updCtxt);
            if (trantyp != GOAL_TRAN_NONE || trangid != 0) {
                ParseTranRequest(trantyp, trangid);
            }
            if (pendtype != PEND_TRAN_NONE) continue;
        }

        trantyp = GOAL_TRAN_NONE;
        trangid = curgoal->Process(&trantyp, dt, updCtxt, NULL);
        if (trantyp != GOAL_TRAN_NONE || trangid != 0) {
            ParseTranRequest(trantyp, trangid);
        }
    } while (pendtype != PEND_TRAN_NONE);

    flg_psyche |= (old_psyflags & 0x1);

    return 0;
}

S32 xPsyche::ParseTranRequest(en_trantype trantyp, S32 trangid)
{
    S32 rc = 0;

    if (trantyp == GOAL_TRAN_NONE && trangid == 0) {
        rc = 1;
    } else if (trantyp == GOAL_TRAN_POP) {
        rc = GoalPop(0, 0);
    } else if (trantyp == GOAL_TRAN_POPALL) {
        rc = GoalNone(0, 0);
    } else if (trantyp == GOAL_TRAN_POPBASE) {
        rc = GoalPopToBase(0);
    } else if (trantyp == GOAL_TRAN_POPSAFE) {
        rc = GoalPopRecover(0);
    } else if (trangid != 0) {
        if (trantyp == GOAL_TRAN_PUSH) {
            rc = GoalPush(trangid, 1);
        } else if (trantyp == GOAL_TRAN_POPTO) {
            rc = GoalPop(trangid, 1);
        } else if (trantyp == GOAL_TRAN_SWAP) {
            rc = GoalSwap(trangid, 1);
        } else if (trantyp == GOAL_TRAN_SET) {
            rc = GoalSet(trangid, 1);
        }
    }

    return rc;
}

S32 xPsyche::TranGoal(F32 dt, void* updCtxt)
{
    S32 halfway = 0;
    S32 just_switched = 0;
    xGoal* topgoal;

    if (staktop < 0 && pendgoal) {
        staktop = 0;
        goalstak[staktop] = pendgoal;

        pendgoal->SetState(GOAL_STAT_ENTER);
        DBG_HistAdd(pendgoal->GetID());

        pendgoal = NULL;
        pendtype = PEND_TRAN_INPROG;
    }

    if (staktop < 0) {
        return 0;
    }

    if (pendtype != PEND_TRAN_NONE && pendtype != PEND_TRAN_INPROG) {
        S32 moretodo = 0;
        
        topgoal = goalstak[staktop];

        switch (topgoal->GetState()) {
        case GOAL_STAT_SUSPEND:
            moretodo = topgoal->Suspend(dt, updCtxt);
            if (!moretodo) {
                topgoal->SetState(GOAL_STAT_PAUSED);
            }
            break;
        case GOAL_STAT_EXIT:
            moretodo = topgoal->Exit(dt, updCtxt);
            if (!moretodo) {
                topgoal->SetState(GOAL_STAT_DONE);
            }
            break;
        default:
            topgoal->GetState();
            break;
        }

        if (moretodo) {
            return moretodo;
        }

        halfway = 1;
    }

    if (halfway) {
        switch (pendtype) {
        case PEND_TRAN_SET:
            if (staktop >= 0) {
                goalstak[staktop] = NULL;
            }
            staktop--;
            if (staktop >= 0) {
                goalstak[staktop]->SetState(GOAL_STAT_EXIT);
                break;
            }
            // fallthrough
        case PEND_TRAN_PUSH:
            staktop++;
            // fallthrough
        case PEND_TRAN_SWAP:
            goalstak[staktop] = pendgoal;

            topgoal = goalstak[staktop];
            topgoal->SetState(GOAL_STAT_ENTER);
            DBG_HistAdd(topgoal->GetID());

            pendgoal = NULL;
            pendtype = PEND_TRAN_INPROG;
            break;
        case PEND_TRAN_POP:
            goalstak[staktop] = NULL;
            staktop--;

            topgoal = goalstak[staktop];
            topgoal->SetState(GOAL_STAT_RESUME);
            DBG_HistAdd(topgoal->GetID());

            pendtype = PEND_TRAN_INPROG;
            break;
        case PEND_TRAN_POPTO:
            goalstak[staktop] = NULL;
            staktop--;

            topgoal = goalstak[staktop];
            if (topgoal == pendgoal) {
                topgoal->SetState(GOAL_STAT_RESUME);
                DBG_HistAdd(topgoal->GetID());

                pendgoal = NULL;
                pendtype = PEND_TRAN_INPROG;
            } else {
                topgoal->SetState(GOAL_STAT_EXIT);
                DBG_HistAdd(topgoal->GetID());
            }
            break;
        case PEND_TRAN_POPALL:
            goalstak[staktop] = NULL;
            staktop--;

            if (staktop >= 0) {
                topgoal = goalstak[staktop];
                topgoal->SetState(GOAL_STAT_EXIT);
                DBG_HistAdd(topgoal->GetID());
            } else {
                pendgoal = NULL;
                pendtype = PEND_TRAN_INPROG;
            }
            break;
        }

        if (pendtype != PEND_TRAN_INPROG) {
            return 0;
        }

        just_switched = 1;
    }

    if (staktop >= 0) {
        S32 moretodo = 0;

        topgoal = goalstak[staktop];

        switch (topgoal->GetState()) {
        case GOAL_STAT_RESUME:
            moretodo = topgoal->Resume(dt, updCtxt);
            if (!moretodo) {
                topgoal->SetState(GOAL_STAT_PROCESS);
                if (cb_notice) {
                    cb_notice->Notice(PSY_NOTE_HASRESUMED, topgoal, NULL);
                }
            }
            break;
        case GOAL_STAT_ENTER:
            if (just_switched) {
                TimerClear();
            }
            
            moretodo = topgoal->Enter(dt, updCtxt);
            if (!moretodo) {
                topgoal->SetState(GOAL_STAT_PROCESS);
                if (cb_notice) {
                    cb_notice->Notice(PSY_NOTE_HASENTERED, topgoal, NULL);
                }
            }
            break;
        default:
            topgoal->GetState();
            break;
        }

        if (!moretodo) {
            pendtype = PEND_TRAN_NONE;
        }
    }

    if (pendtype != PEND_TRAN_NONE) {
        return 2;
    }

    if (staktop >= 0) {
        goalstak[staktop]->Name();
    }

    return 0;
}

F32 xPsyche::TimerGet(en_xpsytime tymr)
{
    if (staktop < 0) return -1.0f;
    return tmr_stack[staktop][tymr];
}

void xPsyche::TimerClear()
{
    if (staktop < 0) return;
    tmr_stack[staktop][0] = 0.0f;
}

void xPsyche::TimerUpdate(F32 dt) NONMATCH("https://decomp.me/scratch/znMnn")
{
    if (staktop < 0) return;
    tmr_stack[staktop][0] += dt;
}
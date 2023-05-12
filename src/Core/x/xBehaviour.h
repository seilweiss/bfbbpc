#pragma once

#include "xListItem.h"
#include "xFactory.h"

struct xGoal;
struct xPsyche;
struct xScene;

enum en_GOALSTATE
{
    GOAL_STAT_UNKNOWN,
    GOAL_STAT_PROCESS,
    GOAL_STAT_ENTER,
    GOAL_STAT_EXIT,
    GOAL_STAT_SUSPEND,
    GOAL_STAT_RESUME,
    GOAL_STAT_PAUSED,
    GOAL_STAT_DONE,
    GOAL_STAT_NOMORE,
    GOAL_STAT_FORCE = FORCEENUMSIZEINT
};

enum en_trantype
{
    GOAL_TRAN_NONE,
    GOAL_TRAN_SET,
    GOAL_TRAN_PUSH,
    GOAL_TRAN_POP,
    GOAL_TRAN_POPTO,
    GOAL_TRAN_POPALL,
    GOAL_TRAN_POPBASE,
    GOAL_TRAN_POPSAFE,
    GOAL_TRAN_SWAP,
    GOAL_TRAN_NOMORE,
    GOAL_TRAN_FORCE = FORCEENUMSIZEINT
};

typedef S32(*xGoalProcessCallback)(xGoal*, void*, en_trantype*, F32, void*);
typedef S32(*xGoalPreCalcCallback)(xGoal*, void*, F32, void*);
typedef S32(*xGoalEvalRuleCallback)(xGoal*, void*, en_trantype*, F32, void*);

struct xGoal : xListItem<xGoal>, xFactoryInst
{
protected:
    xPsyche* psyche;
    S32 goalID;
    en_GOALSTATE stat;
    S32 flg_able;
    xGoalProcessCallback fun_process;
    xGoalPreCalcCallback fun_precalc;
    xGoalEvalRuleCallback fun_chkRule;
    void* cbdata;

public:
    xGoal(S32 myType)
    {
        goalID = myType;
        flg_able = 0;
        stat = GOAL_STAT_UNKNOWN;
    }

    const char* Name() { return NULL; }
    xPsyche* GetPsyche() const { return psyche; }
    void SetPsyche(xPsyche* psy) { psyche = psy; }
    S32 GetID() const { return goalID; }
    en_GOALSTATE GetState() const { return stat; }
    void SetState(en_GOALSTATE state) { stat = state; }
    S32 GetFlags() const { return flg_able; }
    void SetFlags(S32 flags) { flg_able = flags; }
    void AddFlags(S32 flags) { flg_able |= flags; }

    void SetCallbacks(xGoalProcessCallback process, xGoalEvalRuleCallback chkRule, xGoalPreCalcCallback precalc, void* data)
    {
        fun_process = process;
        fun_chkRule = chkRule;
        fun_precalc = precalc;
        cbdata = data;
    }

    xBase* GetOwner() const;

    virtual void Clear() = 0;
    virtual S32 Enter(F32 dt, void* updCtxt) { return 0; }
    virtual S32 Exit(F32 dt, void* updCtxt) { return 0; }
    virtual S32 Suspend(F32 dt, void* updCtxt) { return 0; }
    virtual S32 Resume(F32 dt, void* updCtxt) { return 0; }
    virtual S32 PreCalc(F32 dt, void* updCtxt);
    virtual S32 EvalRules(en_trantype* trantype, F32 dt, void* updCtxt);
    virtual S32 Process(en_trantype* trantype, F32 dt, void* ctxt, xScene*);
    virtual S32 SysEvent(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget, S32* handled) { return 1; }
};
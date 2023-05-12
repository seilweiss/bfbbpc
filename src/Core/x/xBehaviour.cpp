#include "xBehaviour.h"

#include "xBehaveMgr.h"

xBase* xGoal::GetOwner() const
{
    return psyche->GetClient();
}

void xGoal::Clear()
{
    stat = GOAL_STAT_UNKNOWN;
}

S32 xGoal::PreCalc(F32 dt, void* updCtxt)
{
    if (fun_precalc) {
        return fun_precalc(this, cbdata, dt, updCtxt);
    }
    return 0;
}

S32 xGoal::EvalRules(en_trantype* trantype, F32 dt, void* updCtxt)
{
    if (fun_chkRule) {
        return fun_chkRule(this, cbdata, trantype, dt, updCtxt);
    }
    return 0;
}

S32 xGoal::Process(en_trantype* trantype, F32 dt, void* ctxt, xScene*)
{
    if (fun_process) {
        return fun_process(this, cbdata, trantype, dt, ctxt);
    }
    return 0;
}
#include "xBehaveGoalSimple.h"

static xFactoryInst* GOALCreate_Generic(S32 who, RyzMemGrow* growCtxt, void*);
static void GOALDestroy_Generic(xFactoryInst* item);

void xGoalSimple_RegisterTypes(xFactory* fac)
{
    fac->RegItemType('GSM\0', GOALCreate_Generic, GOALDestroy_Generic);
    fac->RegItemType('GSM\1', GOALCreate_Generic, GOALDestroy_Generic);
}

static xFactoryInst* GOALCreate_Generic(S32 who, RyzMemGrow* growCtxt, void*)
{
    xGoal* goal = NULL;

    switch (who) {
    case 'GSM\0':
        goal = new (who, growCtxt) xGoalGeneric(who);
        break;
    case 'GSM\1':
        goal = new (who, growCtxt) xGoalEmpty(who);
        break;
    }

    return goal;
}

static void GOALDestroy_Generic(xFactoryInst* item)
{
    delete item;
}

S32 xGoalGeneric::Enter(F32 dt, void* updCtxt)
{
    if (fun_enter) {
        return fun_enter(this, usrData, dt, updCtxt);
    }

    return xGoal::Enter(dt, updCtxt);
}

S32 xGoalGeneric::Exit(F32 dt, void* updCtxt)
{
    if (fun_exit) {
        return fun_exit(this, usrData, dt, updCtxt);
    }

    return xGoal::Exit(dt, updCtxt);
}

S32 xGoalGeneric::Suspend(F32 dt, void* updCtxt)
{
    if (fun_suspend) {
        return fun_suspend(this, usrData, dt, updCtxt);
    }
    return xGoal::Suspend(dt, updCtxt);
}

S32 xGoalGeneric::Resume(F32 dt, void* updCtxt)
{
    if (fun_resume) {
        return fun_resume(this, usrData, dt, updCtxt);
    }
    return xGoal::Resume(dt, updCtxt);
}

S32 xGoalGeneric::SysEvent(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget, S32* handled)
{
    if (fun_sysevent) {
        return fun_sysevent(this, usrData, from, to, toEvent, toParam, toParamWidget, handled);
    }
    return xGoal::SysEvent(from, to, toEvent, toParam, toParamWidget, handled);
}
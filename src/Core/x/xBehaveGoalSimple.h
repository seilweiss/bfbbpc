#pragma once

#include "xBehaviour.h"

struct xGoalGeneric : xGoal
{
protected:
    S32(*fun_enter)(xGoal*, void*, F32, void*);
    S32(*fun_exit)(xGoal*, void*, F32, void*);
    S32(*fun_suspend)(xGoal*, void*, F32, void*);
    S32(*fun_resume)(xGoal*, void*, F32, void*);
    S32(*fun_sysevent)(xGoal*, void*, xBase*, xBase*, U32, const F32*, xBase*, S32*);
    void* usrData;

public:
    xGoalGeneric(S32 myType) : xGoal(myType) {}

    virtual const char* Name() { return "xGoalGeneric"; }
    virtual S32 Enter(F32 dt, void* updCtxt);
    virtual S32 Exit(F32 dt, void* updCtxt);
    virtual S32 Suspend(F32 dt, void* updCtxt);
    virtual S32 Resume(F32 dt, void* updCtxt);
    virtual S32 SysEvent(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget, S32* handled);
    virtual void Clear() {}
};

struct xGoalEmpty : xGoal
{
public:
    xGoalEmpty(S32 myType) : xGoal(myType) {}

    virtual const char* Name() { return "xGoalEmpty"; }
    virtual void Clear() {}
};

void xGoalSimple_RegisterTypes(xFactory* fac);
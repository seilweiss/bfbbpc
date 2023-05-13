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

    const char* Name() { return "xGoalGeneric"; }
    S32 Enter(F32 dt, void* updCtxt);
    S32 Exit(F32 dt, void* updCtxt);
    S32 Suspend(F32 dt, void* updCtxt);
    S32 Resume(F32 dt, void* updCtxt);
    S32 SysEvent(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget, S32* handled);
    void Clear() {}
};

struct xGoalEmpty : xGoal
{
public:
    xGoalEmpty(S32 myType) : xGoal(myType) {}

    const char* Name() { return "xGoalEmpty"; }
    void Clear() {}
};

void xGoalSimple_RegisterTypes(xFactory* fac);
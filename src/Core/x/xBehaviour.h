#pragma once

#include "xListItem.h"
#include "xFactory.h"

struct xPsyche;

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
typedef enum en_GOALSTATE GOALSTATE;

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
typedef enum en_trantype trantype;

struct xGoal : xListItem<xGoal>, xFactoryInst
{
    xPsyche* psyche;
    S32 goalID;
    GOALSTATE stat;
    S32 flg_able;
    S32(*fun_process)(xGoal*, void*, trantype*, F32, void*);
    S32(*fun_precalc)(xGoal*, void*, F32, void*);
    S32(*fun_chkRule)(xGoal*, void*, trantype*, F32, void*);
    void* cbdata;
};
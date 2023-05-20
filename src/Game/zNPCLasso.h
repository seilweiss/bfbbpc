#pragma once

#include "xEnt.h"

enum en_LASSO_STATUS
{
    LASS_STAT_DONE,
    LASS_STAT_PENDING,
    LASS_STAT_GRABBING,
    LASS_STAT_TOSSING,
    LASS_STAT_NOMORE,
    LASS_STAT_FORCEINT = FORCEENUMSIZEINT
};

enum en_LASSO_EVENT
{
    LASS_EVNT_BEGIN,
    LASS_EVNT_ENDED,
    LASS_EVNT_GRABSTART,
    LASS_EVNT_GRABEND,
    LASS_EVNT_YANK,
    LASS_EVNT_ABORT,
    LASS_EVNT_NOMORE,
    LASS_EVNT_FORCEINT = FORCEENUMSIZEINT
};

enum en_lassanim
{
    LASS_ANIM_UNKNOWN,
    LASS_ANIM_GRAB,
    LASS_ANIM_HOLD,
    LASS_ANIM_NOMORE,
    LASS_ANIM_FORCEINT = FORCEENUMSIZEINT
};

struct zNPCLassoInfo
{
    en_LASSO_STATUS stage;
    xEnt* lassoee;
    xAnimState* holdGuideAnim;
    xModelInstance* holdGuideModel;
    xAnimState* grabGuideAnim;
    xModelInstance* grabGuideModel;
};
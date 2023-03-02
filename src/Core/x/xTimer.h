#pragma once

#include "xBase.h"

struct xTimerAsset;

struct xTimer : xBase
{
    xTimerAsset* tasset;
    U8 state;
    bool runsInPause;
    U16 flags;
    F32 secondsLeft;
};
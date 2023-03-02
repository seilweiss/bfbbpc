#pragma once

#include "xDynAsset.h"

struct CameraTweak_asset : xDynAsset
{
    S32 priority;
    F32 time;
    F32 pitch_adjust;
    F32 dist_adjust;
};
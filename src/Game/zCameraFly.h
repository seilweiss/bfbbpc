#pragma once

#include "types.h"

#define ZFLY_FPS 30.0f

struct zFlyKey
{
    S32 frame;
    F32 matrix[12];
    F32 aperture[2];
    F32 focal;
};

U32 zCameraFlyProcessStopEvent();
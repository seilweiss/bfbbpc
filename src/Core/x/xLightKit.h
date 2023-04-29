#pragma once

#include "types.h"

#include <rwcore.h>
#include <rpworld.h>

struct xLightKitLight
{
    U32 type;
    RwRGBAReal color;
    F32 matrix[16];
    F32 radius;
    F32 angle;
    RpLight* platLight;
};

struct xLightKit
{
    U32 tagID;
    U32 groupID;
    U32 lightCount;
    xLightKitLight* lightList;
};

extern xLightKit* gLastLightKit;

xLightKit* xLightKit_Prepare(void* data);
void xLightKit_Enable(xLightKit* lkit, RpWorld* world);
xLightKit* xLightKit_GetCurrent();
void xLightKit_Destroy(xLightKit* lkit);
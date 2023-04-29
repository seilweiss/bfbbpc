#pragma once

#include "xMath3.h"
#include "xColor.h"

#include <rwcore.h>
#include <rpworld.h>

struct iLight
{
    U32 type;
    RpLight* hw;
    xSphere sph;
    F32 radius_sq;
    xFColor color;
    xVec3 dir;
    F32 coneangle;
};

extern RpWorld* gLightWorld;

void iLightInit(RpWorld* world);
iLight* iLightCreate(iLight* light, U32 type);
void iLightModify(iLight* light, U32 flags);
void iLightSetColor(iLight* light, xFColor* col);
void iLightSetPos(iLight* light, xVec3* pos);
void iLightDestroy(iLight* light);
void iLightEnv(iLight* light, S32 env);
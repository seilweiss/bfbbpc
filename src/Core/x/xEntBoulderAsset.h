#pragma once

#include "xDynAsset.h"
#include "xVec3.h"

struct xEntBoulderAsset
{
    F32 gravity;
    F32 mass;
    F32 bounce;
    F32 friction;
    F32 statFric;
    F32 maxVel;
    F32 maxAngVel;
    F32 stickiness;
    F32 bounceDamp;
    U32 flags;
    F32 killtimer;
    U32 hitpoints;
    U32 soundID;
    F32 volume;
    F32 minSoundVel;
    F32 maxSoundVel;
    F32 innerRadius;
    F32 outerRadius;
};

struct xBoulderGeneratorAsset : xDynAsset
{
    U32 object;
    xVec3 offset;
    F32 offsetRand;
    xVec3 initvel;
    F32 velAngleRand;
    F32 velMagRand;
    xVec3 initaxis;
    F32 angvel;
};
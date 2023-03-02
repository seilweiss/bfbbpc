#pragma once

#include "types.h"

struct zEntDestructObjAsset
{
    F32 animSpeed;
    U32 initAnimState;
    U32 health;
    U32 spawnItemID;
    U32 dflags;
    U8 collType;
    U8 fxType;
    U8 pad[2];
    F32 blast_radius;
    F32 blast_strength;
    U32 shrapnelID_destroy;
    U32 shrapnelID_hit;
    U32 sfx_destroy;
    U32 sfx_hit;
    U32 hitModel;
    U32 destroyModel;
};
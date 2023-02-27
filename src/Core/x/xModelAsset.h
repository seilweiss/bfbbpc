#pragma once

#include "types.h"

struct xModelAssetInst
{
    U32 ModelID;
    U16 Flags;
    U8 Parent;
    U8 Bone;
    F32 MatRight[3];
    F32 MatUp[3];
    F32 MatAt[3];
    F32 MatPos[3];
};

struct xModelAssetParam
{
    U32 HashID;
    U8 WordLength;
    U8 String[3];
};

struct xModelAssetInfo
{
    U32 Magic;
    U32 NumModelInst;
    U32 AnimTableID;
    U32 CombatID;
    U32 BrainID;
};
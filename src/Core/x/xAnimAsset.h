#pragma once

#include "types.h"

struct xAnimAssetFile
{
    U32 FileFlags;
    F32 Duration;
    F32 TimeOffset;
    U16 NumAnims[2];
    void** RawData;
    S32 Physics;
    S32 StartPose;
    S32 EndPose;
};

struct xAnimAssetState
{
    U32 StateID;
    U32 FileIndex;
    U32 EffectCount;
    U32 EffectOffset;
    F32 Speed;
    U32 SubStateID;
    U32 SubStateCount;
};

struct xAnimAssetEffect
{
    U32 StateID;
    F32 StartTime;
    F32 EndTime;
    U32 Flags;
    U32 EffectType;
    U32 UserDataSize;
};

struct xAnimAssetTable
{
    U32 Magic;
    U32 NumRaw;
    U32 NumFiles;
    U32 NumStates;
    U32 ConstructFunc;
};
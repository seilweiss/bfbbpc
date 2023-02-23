#pragma once

#include "xMath3.h"

struct iAnimSKBHeader
{
    U32 Magic;
    U32 Flags;
    U16 BoneCount;
    U16 TimeCount;
    U32 KeyCount;
    F32 Scale[3];
};

struct iAnimSKBKey
{
    U16 TimeIndex;
    S16 Quat[4];
    S16 Tran[3];
};

void iAnimEvalSKB(iAnimSKBHeader* data, F32 time, U32 flags, xVec3* tran, xQuat* quat);
F32 iAnimDurationSKB(iAnimSKBHeader* data);
void _iAnimSKBAdjustTranslate(iAnimSKBHeader* data, U32 bone, F32* starttran, F32* endtran);
S32 _iAnimSKBExtractTranslate(iAnimSKBHeader* data, U32 bone, xVec3* tranArray, S32 tranCount);
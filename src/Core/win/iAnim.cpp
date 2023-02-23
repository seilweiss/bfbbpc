#include "iAnim.h"

#include "iAnimSKB.h"

static U8 scratchBuffer[0x23A0];
U8* giAnimScratch = scratchBuffer;

void iAnimInit() {}

void iAnimEval(void* RawData, F32 time, U32 flags, xVec3* tran, xQuat* quat)
{
    iAnimEvalSKB((iAnimSKBHeader*)RawData, time, flags, tran, quat);
}

F32 iAnimDuration(void* RawData)
{
    return iAnimDurationSKB((iAnimSKBHeader*)RawData);
}

U32 iAnimBoneCount(void* RawData)
{
    if (*(U32*)RawData == '1BKS') {
        return ((iAnimSKBHeader*)RawData)->BoneCount;
    }
    return 0;
}

void iAnimBlend(F32 BlendFactor, F32 BlendRecip, U16* BlendTimeOffset, F32* BoneTable, U32 BoneCount,
                xVec3* Tran1, xQuat* Quat1, xVec3* Tran2, xQuat* Quat2, xVec3* TranDest, xQuat* QuatDest) WIP
{
}
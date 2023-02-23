#pragma once

#include "xModel.h"

struct xModelBucket
{
    RpAtomic* Data;
    RpAtomic* OriginalData;
    xModelInstance* List;
    S32 ClipFlags;
    U32 PipeFlags;
};

struct xModelAlphaBucket
{
    RpAtomic* Data;
    xModelInstance* MInst;
    F32 AlphaFade;
    F32 SortValue;
    U32 Layer;
};

extern S32 iModelHack_DisablePrelight;
extern S32 xModelBucketEnabled;

void xModelBucket_PreCountReset();
void xModelBucket_PreCountBucket(RpAtomic* data, U32 pipeFlags, U32 subObjects);
void xModelBucket_PreCountAlloc(S32 maxAlphaModels);
void xModelBucket_InsertBucket(RpAtomic* data, U32 pipeFlags, U32 subObjects);
void xModelBucket_Init();
xModelBucket** xModelBucket_GetBuckets(RpAtomic* data);
void xModelBucket_Begin();
void xModelBucket_Add(xModelInstance* minst);
void xModelBucket_RenderOpaque();
void xModelBucket_RenderAlphaBegin();
void xModelBucket_RenderAlphaLayer(S32 maxLayer);
void xModelBucket_RenderAlphaEnd();
void xModelBucket_Deinit();
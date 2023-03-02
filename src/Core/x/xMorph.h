#pragma once

#include "types.h"

#include <rwcore.h>

struct xMorphSeqFile
{
    U32 Magic;
    U32 Flags;
    U32 TimeCount;
    U32 ModelCount;
};

typedef void*(*xMorphFindAssetCallback)(U32, char*);

xMorphSeqFile* xMorphSeqSetup(void* data, xMorphFindAssetCallback FindAssetCB);
void xMorphRender(xMorphSeqFile* seq, RwMatrix* mat, F32 time);
F32 xMorphSeqDuration(xMorphSeqFile* seq);
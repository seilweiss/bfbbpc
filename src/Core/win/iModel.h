#pragma once

#include "xMath3.h"

#include <rwcore.h>
#include <rpworld.h>

struct xModelTag;
struct xModelTagWithNormal;

void iModelInit();
RpAtomic* iModelFileNew(void* buffer, U32 size);
void iModelUnload(RpAtomic* userdata);
RpAtomic* iModelFile_RWMultiAtomic(RpAtomic* model);
U32 iModelNumBones(RpAtomic* model);
void iModelQuatToMat(xQuat* quat, xVec3* tran, RwMatrix* mat);
void iModelAnimMatrices(RpAtomic* model, xQuat* quat, xVec3* tran, RwMatrix* mat);
void iModelRender(RpAtomic* model, RwMatrix* mat);
S32 iModelCull(RpAtomic* model, RwMatrix* mat);
S32 iModelSphereCull(xSphere* sphere);
S32 iModelCullPlusShadow(RpAtomic* model, RwMatrix* mat, xVec3* shadowVec, S32* shadowOutside);
U32 iModelVertCount(RpAtomic* model);
U32 iModelVertEval(RpAtomic* model, U32 index, U32 count, RwMatrix* mat, xVec3* vert, xVec3* dest);
U32 iModelNormalEval(xVec3* out, const RpAtomic& m, const RwMatrix* mat, size_t index, S32 size, const xVec3* in);
U32 iModelTagSetup(xModelTag* tag, RpAtomic* model, F32 x, F32 y, F32 z);
U32 iModelTagSetup(xModelTagWithNormal* tag, RpAtomic* model, F32 x, F32 y, F32 z);
void iModelTagEval(RpAtomic* model, const xModelTag* tag, RwMatrix* mat, xVec3* dest);
void iModelTagEval(RpAtomic* model, const xModelTagWithNormal* tag, RwMatrix* mat, xVec3* dest, xVec3* normal);
void iModelSetMaterialAlpha(RpAtomic* model, U8 alpha);
void iModelResetMaterial(RpAtomic* model);
void iModelSetMaterialTexture(RpAtomic* model, void* texture);
void iModelMaterialMul(RpAtomic* model, F32 rm, F32 gm, F32 bm);
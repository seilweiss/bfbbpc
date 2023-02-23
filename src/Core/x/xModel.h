#pragma once

#include "iModel.h"

#include "xAnim.h"
#include "xLightKit.h"
#include "xMath2.h"

#include <rwcore.h>
#include <rpworld.h>

struct xSurface;

struct xModelInstance;
struct xModelPool;
struct xModelBucket;

struct xModelInstance
{
    xModelInstance* Next;
    xModelInstance* Parent;
    xModelPool* Pool;
    xAnimPlay* Anim;
    RpAtomic* Data;
    U32 PipeFlags;
    F32 RedMultiplier;
    F32 GreenMultiplier;
    F32 BlueMultiplier;
    F32 Alpha;
    F32 FadeStart;
    F32 FadeEnd;
    xSurface* Surf;
    xModelBucket** Bucket;
    xModelInstance* BucketNext;
    xLightKit* LightKit;
    void* Object;
    U16 Flags;
    U8 BoneCount;
    U8 BoneIndex;
    U8* BoneRemap;
    RwMatrix* Mat;
    xVec3 Scale;
    U32 modelID;
    U32 shadowID;
    RpAtomic* shadowmapAtomic;
    struct
    {
        xVec3* verts;
    } anim_coll;
};

enum
{
    // Guessed based on context
    ModelInstance_0x4 = 0x4,
    ModelInstance_0x80 = 0x80,
    ModelInstance_0x100 = 0x100
};

struct xModelPool
{
    xModelPool* Next;
    U32 NumMatrices;
    xModelInstance* List;
};

struct xModelTag
{
    xVec3 v;
    U32 matidx;
    F32 wt[4];
};

struct xModelTagWithNormal : xModelTag
{
    xVec3 normal;
};

struct xModelPipeInfo
{
    U32 ModelHashID;
    U32 SubObjectBits;
    U32 PipeFlags;
};

struct xModelPipeLookup
{
    RpAtomic* model;
    U32 PipeFlags;
};

extern RpMorphTarget anim_coll_old_mt;
extern S32 xModelPipeNumTables;
extern S32 xModelPipeCount[16];
extern xModelPipeInfo* xModelPipeData[16];
extern S32 xModelLookupCount;
extern xModelPipeLookup* xModelLookupList;
extern S32 xModelInstStaticAlloc;

U32 xModelGetPipeFlags(RpAtomic* model);
void xModelInit();
void xModelPoolInit(U32 count, U32 numMatrices);
xModelInstance* xModelInstanceAlloc(RpAtomic* data, void* object, U16 flags, U8 boneIndex, U8* boneRemap);
void xModelInstanceFree(xModelInstance* modelInst);
void xModelInstanceAttach(xModelInstance* inst, xModelInstance* parent);
void xModelInstanceUpgradeBrotherShared(xModelInstance* inst, U32 flags);
void xModelUpdate(xModelInstance* modelInst, F32 timeDelta);
void xModelEvalSingle(xModelInstance* modelInst);
void xModelEval(xModelInstance* modelInst);
void xModelRenderSingle(xModelInstance* modelInst);
void xModelRender(xModelInstance* modelInst);
void xModelRender2D(const xModelInstance& model, const basic_rect<F32>& r, const xVec3& from, const xVec3& to);
void xModelSetMaterialAlpha(xModelInstance* modelInst, U8 alpha);
void xModelMaterialMul(xModelInstance* modelInst, F32 rm, F32 gm, F32 bm);
void xModelResetMaterial(xModelInstance* modelInst);
void xModel_SceneEnter(RpWorld* world);
void xModel_SceneExit(RpWorld* world);
void xModelAnimCollStart(xModelInstance& m);
void xModelAnimCollRefresh(const xModelInstance& cm);
xVec3 xModelGetBoneLocation(const xModelInstance& model, size_t index);
void xModelGetBoneMat(xMat4x3& mat, const xModelInstance& model, size_t index);

inline xMat4x3* xModelGetFrame(xModelInstance* model)
{
    return (xMat4x3*)model->Mat;
}

inline void xModelSetFrame(xModelInstance* model, const xMat4x3* frame)
{
    xMat4x3Copy((xMat4x3*)model->Mat, frame);
}

inline xSphere* xModelGetLocalSBound(xModelInstance* model)
{
    return (xSphere*)RpAtomicGetBoundingSphere(model->Data);
}

inline void xModelSetScale(xModelInstance* model, const xVec3& scale)
{
    while (model) {
        model->Scale = scale;
        model = model->Next;
    }
}

inline void xModelAnimCollStop(xModelInstance& m)
{
    m.Flags &= (U16)~0x1800;
}

inline bool xModelAnimCollDirty(const xModelInstance& m)
{
    return (m.Flags & 0x1800) == 0x800;
}

inline void xModelAnimCollApply(const xModelInstance& cm)
{
    xModelInstance& m = (xModelInstance&)cm;
    if (xModelAnimCollDirty(m)) {
        xModelAnimCollRefresh(m);
    }
    RpGeometry* geom = RpAtomicGetGeometry(m.Data);
    RpMorphTarget* mt = RpGeometryGetMorphTarget(geom, 0);
    anim_coll_old_mt.verts = mt->verts;
    mt->verts = (RwV3d*)m.anim_coll.verts;
}

inline void xModelAnimCollRestore(const xModelInstance& cm)
{
    xModelInstance& m = (xModelInstance&)cm;
    RpGeometry* geom = RpAtomicGetGeometry(m.Data);
    RpMorphTarget* mt = RpGeometryGetMorphTarget(geom, 0);
    mt->verts = anim_coll_old_mt.verts;
}
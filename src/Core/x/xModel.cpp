#include "xModel.h"

#include "xModelBucket.h"

#include "rwframesync.h"

RpMorphTarget anim_coll_old_mt;

static xModelPool* sxModelPoolList;
static RwCamera* subcamera;

S32 xModelPipeNumTables;
S32 xModelPipeCount[16];
xModelPipeInfo* xModelPipeData[16];
S32 xModelLookupCount;
xModelPipeLookup* xModelLookupList;
S32 xModelInstStaticAlloc;

static RwCamera* CameraCreate(S32 width, S32 height, S32 zBuffer);
static void CameraDestroy(RwCamera* camera);

U32 xModelGetPipeFlags(RpAtomic* model)
{
    for (S32 i = 0; i < xModelLookupCount; i++) {
        if (xModelLookupList[i].model == model) {
            return xModelLookupList[i].PipeFlags;
        }
    }
    return 0;
}

void xModelInit() NONMATCH("https://decomp.me/scratch/dt5cn")
{
    iModelInit();
    
    sxModelPoolList = NULL;
    
    if (!subcamera) {
        subcamera = CameraCreate(0, 0, 1);
    }

    xModelPipeNumTables = 0;
}

void xModelPoolInit(U32 count, U32 numMatrices)
{
    S32 i;
    U8* buffer;
    RwMatrix* mat;
    xModelPool* pool, *curr, **prev;
    xModelInstance* inst;

    if (numMatrices < 1) numMatrices = 1;

    buffer = (U8*)xMALLOCALIGN(count * numMatrices * sizeof(RwMatrix) +
                               sizeof(xModelPool) +
                               count * sizeof(xModelInstance), 16);
    
    mat = (RwMatrix*)buffer;
    buffer += count * numMatrices * sizeof(RwMatrix);

    pool = (xModelPool*)buffer;
    buffer += sizeof(xModelPool);

    inst = (xModelInstance*)buffer;

    for (i = 0; i < (S32)count; i++) {
        inst[i].Next = &inst[i+1];
        inst[i].Pool = pool;
        inst[i].Mat = mat;
        mat += numMatrices;
    }
    inst[count-1].Next = NULL;

    pool->NumMatrices = numMatrices;
    pool->List = inst;

    curr = sxModelPoolList;
    prev = &sxModelPoolList;
    while (curr && numMatrices < curr->NumMatrices) {
        prev = &curr->Next;
        curr = curr->Next;
    }
    pool->Next = curr;
    *prev = pool;
}

xModelInstance* xModelInstanceAlloc(RpAtomic* data, void* object, U16 flags, U8 boneIndex, U8* boneRemap) NONMATCH("https://decomp.me/scratch/uzoct")
{
    S32 i;
    U32 boneCount, matCount;
    xModelPool* curr, *found;
    xModelInstance* dude;
    RwMatrix* allocmats;

    found = NULL;
    boneCount = iModelNumBones(data);
    matCount = 1 + boneCount + ((flags >> 6) & 0x1);

    if (xModelInstStaticAlloc) {
        if (flags & 0x2000) {
            dude = (xModelInstance*)xMALLOC(sizeof(xModelInstance));
            dude->Pool = NULL;
            dude->Mat = NULL;
        } else {
            allocmats = (RwMatrix*)xMALLOCALIGN(matCount * sizeof(RwMatrix) + sizeof(xModelInstance), 16);
            dude = (xModelInstance*)(allocmats + matCount);
            dude->Pool = NULL;
            dude->Mat = allocmats;
        }
    } else {
        if (flags & 0x2000) {
            flags &= (U16)~0x2000;
            flags |= 0x8;
        }

        curr = sxModelPoolList;
        while (curr) {
            if (curr->List && matCount <= curr->NumMatrices) {
                found = curr;
            }
            curr = curr->Next;
        }

        if (!found) {
            return NULL;
        }

        dude = found->List;
        found->List = dude->Next;
    }

    dude->Next = NULL;
    dude->Parent = NULL;
    dude->Anim = NULL;
    dude->Data = data;
    dude->Object = object;
    dude->Flags = flags | 0x3;
    dude->BoneCount = boneCount;
    dude->BoneIndex = boneIndex;
    dude->BoneRemap = boneRemap;
    dude->modelID = 0;
    dude->shadowID = 0xDEADBEEF;
    dude->Scale.x = 0.0f;
    dude->Scale.y = 0.0f;
    dude->Scale.z = 0.0f;
    dude->RedMultiplier = 1.0f;
    dude->GreenMultiplier = 1.0f;
    dude->BlueMultiplier = 1.0f;
    dude->Alpha = data->geometry->matList.materials[0]->color.alpha / 255.0f;
    dude->Surf = NULL;
    dude->FadeStart = 9e37f;
    dude->FadeEnd = 1e38f;

    if (dude->Mat) {
        for (i = 0; i < (S32)matCount; i++) {
            xMat4x3Identity((xMat4x3*)&dude->Mat[i]);
        }
    }

    dude->Bucket = xModelBucket_GetBuckets(dude->Data);
    dude->LightKit = NULL;

    if (dude->Bucket) {
        dude->PipeFlags = dude->Bucket[0]->PipeFlags;
    } else {
        dude->PipeFlags = xModelGetPipeFlags(dude->Data);
    }

    if (!(dude->PipeFlags & 0xF80000)) {
        dude->PipeFlags |= 0x980000;
    }

    dude->anim_coll.verts = NULL;

    return dude;
}

static xModelInstance* FindChild(xModelInstance* inst)
{
    xModelInstance* curr = inst->Next;
    while (curr) {
        if (curr->Parent == inst) {
            return curr;
        }
        curr = curr->Next;
    }
    return NULL;
}

void xModelInstanceFree(xModelInstance* modelInst)
{
    while (xModelInstance* child = FindChild(modelInst)) {
        xModelInstanceFree(child);
    }

    if (modelInst->Parent) {
        xModelInstance* curr = modelInst->Parent->Next;
        xModelInstance** prev = &modelInst->Parent->Next;
        while (curr && curr != modelInst) {
            prev = &curr->Next;
            curr = curr->Next;
        }
        *prev = curr->Next;
    }

    if (modelInst->Anim) {
        xAnimPoolFree(modelInst->Anim);
        modelInst->Anim = NULL;
    }

    if (modelInst->Pool) {
        modelInst->Next = modelInst->Pool->List;
        modelInst->Pool->List = modelInst;
    }
}

void xModelInstanceAttach(xModelInstance* inst, xModelInstance* parent)
{
    xModelInstance* curr = parent;
    while (curr->Next) {
        curr = curr->Next;
    }
    curr->Next = inst;
    
    inst->Parent = parent;
    if (inst->Flags & 0x2000) {
        inst->Mat = parent->Mat;
    }
}

void xModelInstanceUpgradeBrotherShared(xModelInstance* inst, U32 flags)
{
    if ((inst->Flags & 0x2000) && !(flags & 0x2000)) {
        U32 boneCount = iModelNumBones(inst->Data);
        RwMatrix* allocmats = (RwMatrix*)xMALLOCALIGN(((1 + boneCount + ((flags >> 6) & 0x1))) * sizeof(RwMatrix), 16);
        inst->Mat = allocmats;
        inst->Flags = flags;
    }
}

void xModelUpdate(xModelInstance* modelInst, F32 timeDelta)
{
    while (modelInst) {
        if (modelInst->Anim && (modelInst->Flags & 0x4)) {
            if (modelInst->Flags & 0x100) {
                xAnimPlayChooseTransition(modelInst->Anim);
            }
            xAnimPlayUpdate(modelInst->Anim, timeDelta);
        }
        modelInst = modelInst->Next;
    }
}

void xModelEvalSingle(xModelInstance* modelInst) NONMATCH("https://decomp.me/scratch/JhHRs")
{
    S32 i;
    U16 flags = modelInst->Flags;
    if (flags & 0x2) {
        modelInst->Flags = flags & (U16)~0x1000;
    
        if (modelInst->Anim) {
            xAnimPlayEval(modelInst->Anim);
        }
    
        xModelInstance* dad = modelInst->Parent;
        if (dad) {
            if (flags & 0x8) {
                memcpy(modelInst->Mat, dad->Mat, (modelInst->BoneCount + 1) * sizeof(RwMatrix));
            } else if (flags & 0x10) {
                U8* remap = modelInst->BoneRemap;
                memcpy(modelInst->Mat, dad->Mat, sizeof(RwMatrix));
                for (i = 0; i < modelInst->BoneCount; i++) {
                    memcpy(&modelInst->Mat[i+1], &dad->Mat[remap[i]+1], sizeof(RwMatrix));
                }
            } else if (flags & 0x20) {
                if (dad->BoneCount && modelInst->BoneIndex) {
                    xMat4x3Mul((xMat4x3*)modelInst->Mat,
                               (xMat4x3*)&dad->Mat[modelInst->BoneIndex+1],
                               (xMat4x3*)dad->Mat);
                } else {
                    memcpy(modelInst->Mat, dad->Mat, sizeof(RwMatrix));
                }

                if (flags & 0x40) {
                    xMat4x3Mul((xMat4x3*)modelInst->Mat,
                               (xMat4x3*)&modelInst->Mat[modelInst->BoneCount+1],
                               (xMat4x3*)modelInst->Mat);
                }
            }
        }
    }
}

void xModelEval(xModelInstance* modelInst)
{
    while (modelInst) {
        xModelEvalSingle(modelInst);
        modelInst = modelInst->Next;
    }
}

void xModelRenderSingle(xModelInstance* modelInst)
{
    if ((modelInst->Flags & 0x401) != 0x1) return;
    
    bool reset = false;
    xMat3x3 tmpmat;
    
    if (modelInst->Scale.x) {
        tmpmat = *(xMat3x3*)modelInst->Mat;

        modelInst->Mat->right.x *= modelInst->Scale.x;
        modelInst->Mat->right.y *= modelInst->Scale.x;
        modelInst->Mat->right.z *= modelInst->Scale.x;
        modelInst->Mat->up.x *= modelInst->Scale.y;
        modelInst->Mat->up.y *= modelInst->Scale.y;
        modelInst->Mat->up.z *= modelInst->Scale.y;
        modelInst->Mat->at.x *= modelInst->Scale.z;
        modelInst->Mat->at.y *= modelInst->Scale.z;
        modelInst->Mat->at.z *= modelInst->Scale.z;
    }

    if (!iModelCull(modelInst->Data, modelInst->Mat)) {
        if (modelInst->RedMultiplier != 1.0f ||
            modelInst->GreenMultiplier != 1.0f ||
            modelInst->BlueMultiplier != 1.0f) {
            xModelMaterialMul(modelInst,
                              modelInst->RedMultiplier,
                              modelInst->GreenMultiplier,
                              modelInst->BlueMultiplier);
        }

        reset = true;
        xModelSetMaterialAlpha(modelInst, (U8)(modelInst->Alpha * 255.0f) & 0xFF);

        if (modelInst->Flags & 0x80) {
            xAnimPlay* a = modelInst->Anim;
            for (U16 i = 0; i < a->NumSingle; i++) {
                if (a->Single[i].SingleFlags & 0x8000) {
                    if (a->Single[i].State) {
                        xMorphRender((xMorphSeqFile*)a->Single[i].State->Data->RawData[0],
                                     modelInst->Mat,
                                     xAnimFileRawTime(a->Single[i].State->Data, a->Single[i].Time));
                    }
                    break;
                }
            }
        } else {
            iModelRender(modelInst->Data, modelInst->Mat);
        }
    }
    
    if (reset) {
        xModelResetMaterial(modelInst);
    }

    if (modelInst->Scale.x) {
        *(xMat3x3*)modelInst->Mat = tmpmat;
    }
}

void xModelRender(xModelInstance* modelInst)
{
    while (modelInst) {
        if (xModelBucketEnabled) {
            xModelBucket_Add(modelInst);
        } else {
            xModelRenderSingle(modelInst);
        }
        modelInst = modelInst->Next;
    }
}

void xModelRender2D(const xModelInstance& model, const basic_rect<F32>& r, const xVec3& from, const xVec3& to) NONMATCH("https://decomp.me/scratch/6vVy4")
{
    if (r.w <= 0.0f ||
        r.h <= 0.0f ||
        r.x + r.w < 0.0f ||
        r.x > 1.0f ||
        r.y + r.h < 0.0f ||
        r.y > 1.0f) {
        return;
    }
    
    RwCamera* camera = RwCameraGetCurrentCamera();
    RwFrame* frame = RwCameraGetFrame(camera);
    RwMatrix* cammat = RwFrameGetLTM(frame);
    
    xMat4x3 objmat, shearmat, temp1, temp2;
    xMat3x3LookAt(&objmat, &to, &from);
    xMat3x3Transpose(&objmat, &objmat);
    objmat.pos.x = 0.0f;
    objmat.pos.y = 0.0f;
    objmat.pos.z = xVec3Dist(&to, &from);
    objmat.flags = 0;
    
    const RwV2d* camvw = RwCameraGetViewWindow(camera);
    F32 viewscale = camvw->x * r.w * 2.0f;
    F32 shearX = camvw->x * -(r.x * 2.0f + r.w - 1.0f);
    F32 shearY = camvw->y * -(r.y * 2.0f + r.h - 1.0f);

    shearmat.right.x = viewscale;
    shearmat.right.y = 0.0f;
    shearmat.right.z = 0.0f;
    shearmat.up.x = 0.0f;
    shearmat.up.y = viewscale;
    shearmat.up.z = 0.0f;
    shearmat.at.x = shearX;
    shearmat.at.y = shearY;
    shearmat.at.z = 1.0f;
    shearmat.pos.x = 0.0f;
    shearmat.pos.y = 0.0f;
    shearmat.pos.z = 0.0f;
    shearmat.flags = 0;

    xMat4x3Mul(&temp1, &objmat, &shearmat);
    xMat4x3Mul(&temp2, &temp1, (xMat4x3*)cammat);
    xMat4x3Mul(&objmat, (xMat4x3*)model.Mat, &temp2);
    
    temp1 = *(xMat4x3*)model.Mat;
    *model.Mat = *(RwMatrix*)&objmat;

    iModelRender(model.Data, model.Mat);

    *model.Mat = *(RwMatrix*)&temp1;
}

static RwCamera* CameraCreate(S32 width, S32 height, S32 zBuffer)
{
    RwCamera* camera;
    
    camera = RwCameraCreate();
    if (camera) {
        RwCameraSetFrame(camera, RwFrameCreate());
        RwCameraSetRaster(camera, RwRasterCreate(width, height, 0, rwRASTERTYPECAMERA));
        if (zBuffer) {
            RwCameraSetZRaster(camera, RwRasterCreate(width, height, 0, rwRASTERTYPEZBUFFER));
        }
        if (RwCameraGetFrame(camera) &&
            RwCameraGetRaster(camera) &&
            (!zBuffer || RwCameraGetZRaster(camera))) {
            return camera;
        }
    }
    
    CameraDestroy(camera);
    return NULL;
}

static void CameraDestroy(RwCamera* camera)
{
    RwRaster* raster;
    RwFrame* frame;

    if (camera) {
        _rwFrameSyncDirty();

        frame = RwCameraGetFrame(camera);
        if (frame) {
            RwCameraSetFrame(camera, NULL);
            RwFrameDestroy(frame);
        }

        raster = RwCameraGetRaster(camera);
        if (raster) {
            RwRasterDestroy(raster);
            RwCameraSetRaster(camera, NULL);
        }

        raster = RwCameraGetZRaster(camera);
        if (raster) {
            RwRasterDestroy(raster);
            RwCameraSetZRaster(camera, NULL);
        }

        RwCameraDestroy(camera);
    }
}

void xModelSetMaterialAlpha(xModelInstance* modelInst, U8 alpha)
{
    iModelSetMaterialAlpha(modelInst->Data, alpha);
}

void xModelMaterialMul(xModelInstance* modelInst, F32 rm, F32 gm, F32 bm)
{
    iModelMaterialMul(modelInst->Data, rm, gm, bm);
}

void xModelResetMaterial(xModelInstance* modelInst)
{
    iModelResetMaterial(modelInst->Data);
}

void xModel_SceneEnter(RpWorld* world)
{
    RpWorldAddCamera(world, subcamera);
}

void xModel_SceneExit(RpWorld* world)
{
    RpWorldRemoveCamera(world, subcamera);
}

void xModelAnimCollStart(xModelInstance& m)
{
    m.Flags = (m.Flags & ~0x1000) | 0x800;
    if (!m.anim_coll.verts) {
        U32 size = iModelVertCount(m.Data);
        if (size) {
            m.anim_coll.verts = (xVec3*)xMALLOC(size * sizeof(xVec3));
        }
    }
}

void xModelAnimCollRefresh(const xModelInstance& cm)
{
    xModelInstance& m = (xModelInstance&)cm;
    U32 size = iModelVertCount(m.Data);
    xMat4x3& mat = *(xMat4x3*)m.Mat;
    xMat4x3 old_mat = mat;
    
    mat = g_I3;
    iModelVertEval(m.Data, 0, size, m.Mat, NULL, m.anim_coll.verts);
    mat = old_mat;

    m.Flags |= 0x1000;
}

xVec3 xModelGetBoneLocation(const xModelInstance& model, size_t index)
{
    xMat4x3& root_mat = *(xMat4x3*)model.Mat;
    xMat4x3& anim_mat = *(xMat4x3*)(model.Mat + index);
    if (index == 0) {
        return root_mat.pos;
    }
    xVec3 ret;
    xMat4x3Toworld(&ret, &root_mat, &anim_mat.pos);
    return ret;
}

void xModelGetBoneMat(xMat4x3& mat, const xModelInstance& model, size_t index)
{
    const xMat4x3& root_mat = *(xMat4x3*)model.Mat;
    if (index == 0) {
        mat = root_mat;
        return;
    }
    const xMat4x3& anim_mat = *(xMat4x3*)(model.Mat + index);
    xMat4x3Mul(&mat, &anim_mat, &root_mat);
}
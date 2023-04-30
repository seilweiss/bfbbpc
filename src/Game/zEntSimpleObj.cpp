#include "zEntSimpleObj.h"

#include "xSimpleObjAsset.h"
#include "xEntAsset.h"
#include "xString.h"
#include "xModelBucket.h"
#include "xstransvc.h"
#include "xSkyDome.h"
#include "zBase.h"
#include "zLOD.h"
#include "zGlobals.h"
#include "zGame.h"
#include "zFX.h"
#include "zEvent.h"
#include "zCollGeom.h"
#include "zGoo.h"
#include "zShrapnelAsset.h"

static U32 sMgrCount;
static zSimpleMgr* sMgrList;
static U32 sSimpleCustomCount;
static xEnt** sSimpleCustomList;

static void zEntSimpleObj_Render(xEnt* ent);
static void zEntSimpleObj_Move(xEnt*, xScene*, F32, xEntFrame*);

void zEntSimpleObj_MgrInit(zEntSimpleObj** entList, U32 entCount) NONMATCH("https://decomp.me/scratch/gzNVg")
{
    U32 i;
    
    sMgrCount = 0;
    sMgrList = NULL;
    sSimpleCustomCount = 0;
    sSimpleCustomList = NULL;

    if (!entCount) return;

    zEntSimpleObj** tempEntList = (zEntSimpleObj**)RwMalloc(entCount * sizeof(zEntSimpleObj*));
    U32 tempEntCount = 0;
    U32 custEntCount = 0;
    U32 trailerHash = xStrHash("trailer_hitch");
    
    for (i = 0; i < entCount; i++) {
        if (entList[i]->sflags & 0x10) continue;

        if (entList[i]->update != (xEntUpdateCallback)zEntSimpleObj_Update ||
                entList[i]->render != zEntSimpleObj_Render ||
                entList[i]->eventFunc != zEntSimpleObjEventCB ||
                entList[i]->move ||
                (entList[i]->moreFlags & k_MORE_FLAGS_0x8) ||
                (entList[i]->moreFlags & k_MORE_FLAGS_ANIM_COLL) ||
                (entList[i]->miscflags & 0x1) ||
                entList[i]->atbl ||
                (entList[i]->sflags & 0x4) ||
                (entList[i]->sflags & 0x8) ||
                trailerHash == entList[i]->asset->modelInfoID ||
                entList[i]->baseType == eBaseTypeTrackPhysics ||
                entList[i]->driver) {
            tempEntList[entCount - 1 - custEntCount] = entList[i];
            custEntCount++;

            if (entList[i]->driver && !entList[i]->move) {
                entList[i]->move = zEntSimpleObj_Move;
                entList[i]->pflags |= k_XENT_IS_MOVING;
                entList[i]->frame = (xEntFrame*)xMALLOC(sizeof(xEntFrame));
            }
        } else {
            entList[i]->baseFlags |= k_XBASE_0x80;
            
            tempEntList[tempEntCount] = entList[i];
            tempEntCount++;
        }
    }
    
    if (custEntCount) {
        sSimpleCustomCount = custEntCount;
        sSimpleCustomList = (xEnt**)xMALLOC(custEntCount * sizeof(xEnt*));

        for (i = 0; i < custEntCount; i++) {
            sSimpleCustomList[i] = tempEntList[entCount - 1 - i];
        }
    } else {
        sSimpleCustomCount = 0;
        sSimpleCustomList = NULL;
    }

    entCount = tempEntCount;
    entList = tempEntList;

    if (!entCount) {
        RwFree(tempEntList);
        return;
    }

    sMgrCount = entCount;
    sMgrList = (zSimpleMgr*)xMALLOCALIGN(sMgrCount * sizeof(zSimpleMgr), 64);
    
    zSimpleMgr* smgr = sMgrList;
    for (i = 0; i < entCount; i++) {
        RpAtomic* model = entList[i]->model->Data;
        
        RwSphere oldbound = model->boundingSphere;
        model->boundingSphere.radius *= 1.1f;
        iModelCull(model, entList[i]->model->Mat);
        model->boundingSphere = oldbound;

        smgr->worldBound.center.x = model->worldBoundingSphere.center.x;
        smgr->worldBound.center.y = model->worldBoundingSphere.center.y;
        smgr->worldBound.center.z = model->worldBoundingSphere.center.z;
        smgr->worldBound.r = model->worldBoundingSphere.radius;

        zLODTable* lod = zLOD_Get(entList[i]);
        if (lod) {
            F32 distscale = xsqr(entList[i]->model->Mat->right.x) +
                            xsqr(entList[i]->model->Mat->right.y) +
                            xsqr(entList[i]->model->Mat->right.z);
            if (distscale < 0.0001f) distscale = 1.0f;

            smgr->lodDist[0] = lod->lodDist[0] ? lod->lodDist[0] * distscale : HUGE;
            smgr->lodDist[1] = lod->lodDist[1] ? lod->lodDist[1] * distscale : HUGE;
            smgr->lodDist[2] = lod->lodDist[2] ? lod->lodDist[2] * distscale : HUGE;
            smgr->lodDist[3] = lod->noRenderDist ? lod->noRenderDist * distscale : HUGE;
            smgr->lodBucket[0] = lod->baseBucket;
            smgr->lodBucket[1] = lod->lodBucket[0];
            smgr->lodBucket[2] = lod->lodBucket[1];
            smgr->lodBucket[3] = lod->lodBucket[2];

            if (!smgr->lodBucket[1]) smgr->lodDist[0] = HUGE;
            if (!smgr->lodBucket[2]) smgr->lodDist[1] = HUGE;
            if (!smgr->lodBucket[3]) smgr->lodDist[2] = HUGE;
        } else {
            smgr->lodDist[0] = HUGE;
            smgr->lodDist[1] = HUGE;
            smgr->lodDist[2] = HUGE;
            smgr->lodDist[3] = HUGE;
            smgr->lodBucket[0] = entList[i]->model->Bucket;
            smgr->lodBucket[1] = NULL;
            smgr->lodBucket[2] = NULL;
            smgr->lodBucket[3] = NULL;
        }

        smgr->entFlags = entList[i]->flags;
        smgr->mat = entList[i]->model->Mat;
        smgr->ent = entList[i];
        smgr->lastlod = 0xFF;

        xEntUpdate(entList[i], globals.sceneCur, 0.0f);

        smgr++;
    }

    RwFree(entList);
}

void zEntSimpleObj_MgrUpdateRender(RpWorld*, F32 dt)
{
    U32 i;
    xVec3* campos = &globals.camera.mat.pos;
    
    zSimpleMgr* smgr = sMgrList;
    for (i = 0; i < sMgrCount; i++, smgr++) {
        zEntSimpleObj* ent = smgr->ent;
        if (!xEntIsVisible(ent)) continue;

        F32 camdist2 = xsqr(campos->x - smgr->worldBound.center.x) +
                       xsqr(campos->y - smgr->worldBound.center.y) +
                       xsqr(campos->z - smgr->worldBound.center.z);
        if (camdist2 > smgr->lodDist[3]) continue;

        if (iModelSphereCull(&smgr->worldBound)) continue;

        U8 picklod = 0;
        if (camdist2 > smgr->lodDist[0]) {
            picklod = 1;
            if (camdist2 > smgr->lodDist[1]) {
                picklod = 2;
                if (camdist2 > smgr->lodDist[2]) {
                    picklod = 3;
                }
            }
        }

        xModelInstance* model = ent->model;
        
        model->Flags &= (U16)~0x400;
        smgr->lastlod = picklod;
        model->Bucket = smgr->lodBucket[picklod];
        model->Data = model->Bucket[0]->OriginalData;
        
        if (picklod == 0) {
            model = model->Next;
            while (model) {
                model->Flags &= (U16)~0x400;
                model = model->Next;
            }
        } else {
            model = model->Next;
            while (model) {
                model->Flags |= 0x400;
                model = model->Next;
            }
        }

        if (ent->anim && !zGameIsPaused()) {
            F32 duration = iAnimDuration(ent->anim);

            ent->animTime += dt;
            if (ent->animTime >= duration) {
                ent->animTime -= duration;
            }

            xQuat* q0 = (xQuat*)giAnimScratch;
            xVec3* t0 = (xVec3*)(q0 + 65);

            iAnimEval(ent->anim, ent->animTime, 0, t0, q0);
            iModelAnimMatrices(ent->model->Data, q0, t0, ent->model->Mat + 1);
        }

        xLightKit_Enable(ent->lightKit, globals.currWorld);
        
        zEntSimpleObj_Render(ent);

        if (picklod == 0 && (xrand() & 0xFFFF) < 85) {
            xVec3 blob_posrnd = { 0.25f, 1.0f, 0.25f };
            xVec3 pos;

            xVec3Copy(&pos, (xVec3*)&ent->model->Mat->pos);
            pos.y += 0.25f * xurand() + 0.25f;

            zFX_SpawnBubbleTrail(&pos, (xrand() & 0x7) + 1, &blob_posrnd, NULL);
        }
    }
}

void zEntSimpleObj_MgrCustomUpdate(zScene* s, F32 dt)
{
    for (U32 i = 0; i < sSimpleCustomCount; i++) {
        if (!(sSimpleCustomList[i]->baseFlags & k_XBASE_0x40)) {
            sSimpleCustomList[i]->update(sSimpleCustomList[i], s, dt);
        }
    }
}

void zEntSimpleObj_MgrCustomRender()
{
    for (U32 i = 0; i < sSimpleCustomCount; i++) {
        xLightKit_Enable(sSimpleCustomList[i]->lightKit, globals.currWorld);
        sSimpleCustomList[i]->render(sSimpleCustomList[i]);
    }
}

static void zEntSimpleObj_Render(xEnt* ent)
{
    if (!ent->model || !xEntIsVisible(ent)) return;

    xModelRender(ent->model);
}

void zEntTrackPhysics_Init(void* ent, void* asset)
{
    zEntSimpleObj_Init((zEntSimpleObj*)ent, (xEntAsset*)asset, true);
}

void zEntSimpleObj_Init(void* ent, void* asset)
{
    zEntSimpleObj_Init((zEntSimpleObj*)ent, (xEntAsset*)asset, false);
}

void zEntSimpleObj_Init(zEntSimpleObj* ent, xEntAsset* asset, bool physparams)
{
    zEntInit(ent, asset, 'SIMP');

    if (physparams) {
        ent->baseType = eBaseTypeTrackPhysics;
    }
    
    xSimpleObjAsset* sasset;
    if (physparams) {
        sasset = (xSimpleObjAsset*)(asset + 1);
    } else {
        sasset = (xSimpleObjAsset*)(asset + 1);
    }

    ent->sasset = sasset;
    ent->sflags = 0;
    ent->pflags = 0;
    ent->penby |= k_XENT_COLLTYPE_PC;

    if (ent->sasset->collType & k_XENT_COLLTYPE_STAT) {
        ent->chkby = (k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC);
    } else {
        ent->chkby = 0;
    }

    ent->move = NULL;
    ent->update = (xEntUpdateCallback)zEntSimpleObj_Update;
    ent->eventFunc = zEntSimpleObjEventCB;
    ent->render = zEntSimpleObj_Render;

    if (ent->linkCount) {
        if (physparams) {
            ent->link = (xLinkAsset*)((U8*)ent->asset + sizeof(xEntAsset) + sizeof(xSimpleObjAsset) + 0x3C); // TODO: figure out what 0x3C is
        } else {
            ent->link = (xLinkAsset*)((U8*)ent->asset + sizeof(xEntAsset) + sizeof(xSimpleObjAsset));
        }
    } else {
        ent->link = NULL;
    }

    ent->eventFunc = zEntSimpleObjEventCB; // redundant
    
    U32 tmpsize;
    RpAtomic* modelData = (RpAtomic*)xSTFindAsset(asset->modelInfoID, &tmpsize);
    void* animData = NULL;

    if (!(ent->miscflags & 0x1) &&
            ent->asset->modelInfoID &&
            ent->model &&
            ent->model->Anim &&
            ent->model->Anim->Table &&
            !strcmp("xEntAutoEventSimple", ent->model->Anim->Table->Name)) {
        xAnimPlaySetState(ent->model->Anim->Single, &ent->model->Anim->Table->StateList[0], 0.0f);
        ent->miscflags |= 0x1;
    } else if (asset->animListID && !ent->atbl) {
        if (animData = xSTFindAsset(asset->animListID, &tmpsize)) {
            U32 animBoneCount = iAnimBoneCount(animData);
            if (animBoneCount == 0 || animBoneCount != iModelNumBones(modelData)) {
                animData = NULL;
            }
        }
    }

    ent->anim = animData;
    ent->animTime = 0.0f;

    zEntReset(ent);
}

static void zEntSimpleObj_Move(xEnt*, xScene*, F32, xEntFrame*)
{
}

void zEntSimpleObj_Update(zEntSimpleObj* ent, xScene* sc, F32 dt)
{
    xEntUpdate(ent, sc, dt);

    if (!ent->anim) return;
    if (!ent->model) return;
    if (ent->model->Flags & 0x400) return;
    
    F32 duration = iAnimDuration(ent->anim);

    ent->animTime += dt;
    if (ent->animTime >= duration) {
        ent->animTime -= duration;
    }
    
    xQuat* q0 = (xQuat*)giAnimScratch;
    xVec3* t0 = (xVec3*)(q0 + 65);

    iAnimEval(ent->anim, ent->animTime, 0, t0, q0);
    iModelAnimMatrices(ent->model->Data, q0, t0, ent->model->Mat + 1);
}

void zEntSimpleObj_Setup(zEntSimpleObj* ent)
{
    zEntSetup(ent);
}

void zEntSimpleObj_Save(zEntSimpleObj* ent, xSerial* s)
{
    zEntSave(ent, s);
}

void zEntSimpleObj_Load(zEntSimpleObj* ent, xSerial* s)
{
    zEntLoad(ent, s);
}

void zEntSimpleObj_Reset(zEntSimpleObj* ent, xScene*)
{
    zEntReset(ent);

    ent->animTime = 0.0f;
    ent->chkby &= (U8)~(k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC | k_XENT_COLLTYPE_DYN);

    if (ent->sasset->collType & k_XENT_COLLTYPE_STAT) {
        ent->chkby |= (k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC);
    }

    xEntBoundUpdate(ent, (xVec3*)&ent->model->Mat->pos);
}

S32 zEntSimpleObjEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget)
{
    zEntSimpleObj* s = (zEntSimpleObj*)to;

    switch (toEvent) {
    case eEventVisible:
    case eEventFastVisible:
        xEntShow(s);
        if (toParam) {
            if ((S32)(0.5f + toParam[0]) == 77) {
                zFXPopOn(*s, toParam[1], toParam[2]);
            }
        }
        break;
    case eEventInvisible:
    case eEventFastInvisible:
        xEntHide(s);
        if (toParam) {
            if ((S32)(0.5f + toParam[0]) == 77) {
                zFXPopOff(*s, toParam[1], toParam[2]);
            }
        }
        break;
    case eEventCollision_Visible_On:
        xEntShow(s);
        if (toParam) {
            if ((S32)(0.5f + toParam[0]) == 77) {
                zFXPopOn(*s, toParam[1], toParam[2]);
            }
        }
        // fallthrough
    case eEventCollisionOn:
        s->chkby = (k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC);
        xEntBoundUpdate(s, (xVec3*)&s->model->Mat->pos);
        break;
    case eEventCollision_Visible_Off:
        xEntHide(s);
        if (toParam) {
            if ((S32)(0.5f + toParam[0]) == 77) {
                zFXPopOff(*s, toParam[1], toParam[2]);
            }
        }
        // fallthrough
    case eEventCollisionOff:
        s->chkby = 0;
        xEntBoundUpdate(s, (xVec3*)&s->model->Mat->pos);
        break;
    case eEventCameraCollideOn:
        zCollGeom_CamEnable(s);
        break;
    case eEventCameraCollideOff:
        zCollGeom_CamDisable(s);
        break;
    case eEventReset:
        zEntSimpleObj_Reset(s, globals.sceneCur);
        break;
    case eEventAnimPlay:
    case eEventAnimPlayLoop:
    case eEventAnimStop:
    case eEventAnimPause:
    case eEventAnimResume:
    case eEventAnimTogglePause:
    case eEventAnimPlayRandom:
    case eEventAnimPlayMaybe:
        zEntAnimEvent(s, toEvent, toParam);
        break;
    case eEventSetSkyDome:
        xSkyDome_AddEntity(s, (S32)toParam[0], (S32)toParam[1]);
        break;
    case eEventSetGoo:
        zGooAdd(s, toParam[0], (S32)toParam[1]);
        break;
    case eEventGooSetWarb:
        zFXGooEventSetWarb(s, toParam);
        break;
    case eEventGooSetFreezeDuration:
        zFXGooEventSetFreezeDuration(s, toParam[0]);
        break;
    case eEventGooMelt:
        zFXGooEventMelt(s);
        break;
    case eEventLaunchShrapnel:
    {
        zShrapnelAsset* shrap = (zShrapnelAsset*)toParamWidget;
        if (shrap && shrap->initCB) {
            shrap->initCB(shrap, s->model, NULL, NULL);
        }
        break;
    }
    case eEventDestroy:
        xEntHide(s);
        break;
    }

    return 1;
}
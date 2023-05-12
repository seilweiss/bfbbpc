#include "zEntDestructObj.h"

#include "xSnd.h"
#include "xstransvc.h"
#include "xString.h"
#include "xEntAsset.h"
#include "zEntDestructObjAsset.h"
#include "zCollGeom.h"
#include "zGlobals.h"
#include "zEvent.h"
#include "zRumble.h"
#include "zFX.h"
#include "zShrapnelAsset.h"

static zParEmitter* sEmitDust;
static zParEmitter* sEmitXplo;
static zParEmitter* sEmitWeb;
static zParEmitter* sEmitFire;
static zParEmitter* sEmitSmoke;
static zShrapnelAsset* sShrapDefault;

namespace {
void SwapModel(zEntDestructObj* s, xModelInstance* model)
{
    if (s->model == model) return;

    if (s->collModel && s->collModel != s->model) {
        xModelInstanceFree(s->collModel);
        s->collModel = NULL;
    }

    if (s->camcollModel && s->camcollModel != s->model) {
        xModelInstanceFree(s->camcollModel);
        s->camcollModel = NULL;
    }

    *model->Mat = *s->model->Mat;
    
    model->Flags &= (U16)~(0x4000 | 0x400 | 0x2 | 0x1);
    model->Flags |= (U16)(s->model->Flags & (0x4000 | 0x400 | 0x2 | 0x1));

    s->model = model;

    iBoxForModelLocal(&s->bound.box.box, model);
    zCollGeom_EntSetup(s);

    if (s->sfx_destroy) {
        xSndPlay3D(s->sfx_destroy->soundAssetID, s->sfx_destroy->volume, 0.0f,
                   128, 0, &s->sfx_destroy->pos, 0.0f, SND_CAT_GAME, 0.0f);
    }
}
}

void zEntDestructObj_FindFX()
{
    sEmitDust = zParEmitterFind("PAREMIT_DOBJ_DUST");
    sEmitXplo = zParEmitterFind("PAREMIT_DOBJ_XPLO");
    sEmitWeb = zParEmitterFind("PzREMIT_DOBJ_WEB"); // BUG? typo?
    sEmitFire = zParEmitterFind("PAREMIT_FIRE");
    sEmitSmoke = zParEmitterFind("PAREMIT_FIRESMOKE");

    sShrapDefault = (zShrapnelAsset*)xSTFindAsset(xStrHash("destruct_obj_shrapnel"), NULL);
}

void zEntDestructObj_Init(void* ent, void* asset)
{
    zEntDestructObj_Init((zEntDestructObj*)ent, (xEntAsset*)asset);
}

void zEntDestructObj_Init(zEntDestructObj* ent, xEntAsset* asset)
{
    zEntInit(ent, asset, 'DSTR');

    zEntDestructObjAsset* dasset = (zEntDestructObjAsset*)(asset + 1);
    
    ent->dasset = dasset;
    ent->healthCnt = dasset->health;

    if (dasset->shrapnelID_destroy) {
        ent->shrapnel_destroy = (zShrapnelAsset*)xSTFindAsset(dasset->shrapnelID_destroy, NULL);
    } else {
        ent->shrapnel_destroy = NULL;
    }
    if (!ent->shrapnel_destroy) {
        ent->shrapnel_destroy = (zShrapnelAsset*)xSTFindAsset(xStrHash("destruct_obj_shrapnel"), NULL);
    }

    if (dasset->shrapnelID_hit) {
        ent->shrapnel_hit = (zShrapnelAsset*)xSTFindAsset(dasset->shrapnelID_hit, NULL);
    } else {
        ent->shrapnel_hit = NULL;
    }
    if (!ent->shrapnel_hit) {
        ent->shrapnel_hit = (zShrapnelAsset*)xSTFindAsset(xStrHash("destruct_obj_shrapnel"), NULL);
    }

    if (dasset->sfx_destroy) {
        ent->sfx_destroy = (xSFXAsset*)xSTFindAsset(dasset->sfx_destroy, NULL);
    } else {
        ent->sfx_destroy = NULL;
    }

    if (dasset->sfx_hit) {
        ent->sfx_hit = (xSFXAsset*)xSTFindAsset(dasset->sfx_hit, NULL);
    } else {
        ent->sfx_hit = NULL;
    }

    ent->penby |= k_XENT_COLLTYPE_PC;
    ent->state = 0;

    if (ent->dasset->collType & k_XENT_COLLTYPE_STAT) {
        ent->chkby |= (k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC);
    } else {
        ent->chkby = 0;
    }

    ent->chkby |= k_XENT_COLLTYPE_DYN;
    ent->move = (xEntMoveCallback)zEntDestructObj_Move;
    ent->eventFunc = zEntDestructObjEventCB;
    ent->update = (xEntUpdateCallback)zEntDestructObj_Update;

    if (ent->linkCount) {
        ent->link = (xLinkAsset*)((U8*)ent->asset + sizeof(xEntAsset) + sizeof(zEntDestructObjAsset));
    } else {
        ent->link = NULL;
    }

    ent->eventFunc = zEntDestructObjEventCB; // redundant
    
    ent->throw_target = 0;
    if (ent->bound.type == k_XBOUNDTYPE_SPHERE) {
        if (ent->bound.sph.r <= 2.0f) {
            ent->throw_target = 1;
        }
    } else if (ent->bound.type == k_XBOUNDTYPE_CYL) {
        if (ent->bound.cyl.r <= 2.0f) {
            ent->throw_target = 1;
        }
    } else {
        if (ent->bound.box.box.upper.x - ent->bound.box.box.lower.x <= 4.0f &&
            ent->bound.box.box.upper.z - ent->bound.box.box.lower.z <= 4.0f) {
            ent->throw_target = 1;
        }
    }
    if (!(ent->dasset->dflags & 0x800)) {
        ent->throw_target = 0;
    }

    ent->base_model = ent->model;
    
    ent->hit_model = NULL;
    if (dasset->hitModel) {
        RpAtomic* imodel = (RpAtomic*)xSTFindAsset(dasset->hitModel, NULL);
        if (imodel) {
            ent->hit_model = xEntLoadModel(NULL, imodel);
            *ent->hit_model->Mat = *ent->model->Mat;
            ent->hit_model->Flags &= (U16)~0x400;
        }
    }

    ent->destroy_model = NULL;
    if (dasset->destroyModel) {
        RpAtomic* imodel = (RpAtomic*)xSTFindAsset(dasset->destroyModel, NULL);
        if (imodel) {
            ent->destroy_model = xEntLoadModel(NULL, imodel);
            *ent->destroy_model->Mat = *ent->model->Mat;
            ent->destroy_model->Flags &= (U16)~0x400;
        }
    }

    xEntReset(ent);

    ent->destroy_notify = NULL;
    ent->notify_context = NULL;
}

void zEntDestructObj_Move(zEntDestructObj*, xScene*, F32, xEntFrame*)
{
}

void zEntDestructObj_Update(zEntDestructObj* ent, xScene* sc, F32 dt) NONMATCH("https://decomp.me/scratch/SM73Z")
{
    xEntUpdate(ent, sc, dt);

    if (ent->healthCnt > 1) {
        ent->state = 0;
    }

    if (ent->fx_emitter && ent->fx_timer > 0.0f) {
        ent->fx_timer -= dt;

        xParEmitterCustomSettings info;
        info.custom_flags = 0x100;
        info.pos = *xEntGetCenter(ent);

        xParEmitterEmitCustom(ent->fx_emitter, dt, &info);
    }
    
    if (ent->respawn_timer) {
        ent->respawn_timer -= dt;

        if (ent->respawn_timer < 0.0f) {
            zEntDestructObj_Reset(ent, globals.sceneCur);
        }
    }
}

void zEntDestructObj_Hit(zEntDestructObj* ent, U32 mask)
{
    if (ent->dasset->dflags & mask) {
        if (mask & 0x8000) {
            zEntPlayer_SNDPlayStreamRandom(0, 16, ePlayerStreamSnd_BowlComment1, ePlayerStreamSnd_BowlComment3, 0.1f);
            zEntPlayer_SNDPlayStreamRandom(16, 35, ePlayerStreamSnd_BowlComment1, ePlayerStreamSnd_BowlComment4, 0.1f);
            zEntPlayer_SNDPlayStreamRandom(36, 100, ePlayerStreamSnd_BowlComment1, ePlayerStreamSnd_BowlComment5, 0.1f);
        }

        zEntEvent(ent, eEventHit);
    }
}

U32 zEntDestructObj_GetHit(zEntDestructObj* ent, U32 mask)
{
    return (ent->dasset->dflags & mask) != 0;
}

void zEntDestructObj_Save(zEntDestructObj* ent, xSerial* s)
{
    zEntSave(ent, s);

    s->Write(ent->state);
}

void zEntDestructObj_Load(zEntDestructObj* ent, xSerial* s)
{
    zEntLoad(ent, s);

    s->Read(&ent->state);
}

void zEntDestructObj_Setup(zEntDestructObj* ent)
{
    zEntSetup(ent);
}

void zEntDestructObj_Reset(zEntDestructObj* ent, xScene*) NONMATCH("https://decomp.me/scratch/wYjXY")
{
    xEntReset(ent);

    if (ent->flags & k_XENT_IS_STACKED) {
        ent->pflags &= k_XENT_HAS_GRAVITY;
    } else {
        ent->pflags = 0;
    }

    ent->healthCnt = ent->dasset->health;
    ent->penby |= (k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC);

    if (ent->dasset->collType & k_XENT_COLLTYPE_STAT) {
        ent->chkby |= (k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC);
    } else {
        ent->chkby = 0;
    }

    ent->bupdate(ent, (xVec3*)&ent->model->Mat->pos);
    ent->chkby |= k_XENT_COLLTYPE_DYN;
    ent->fx_timer = 0.0f;
    ent->respawn_timer = 0.0f;
    ent->fx_emitter = NULL;
    ent->state = 1;

    SwapModel(ent, ent->base_model);

    ent->destroy_notify = NULL;
    ent->notify_context = NULL;
}

U32 zEntDestructObj_isDestroyed(zEntDestructObj* ent)
{
    return ent->state == 2 ? 1 : 0;
}

void zEntDestructObj_DestroyFX(zEntDestructObj* o)
{
    if (o->sfx_destroy) {
        xSndPlay3D(o->sfx_destroy->soundAssetID, o->sfx_destroy->volume, 0.0f,
                   128, 0, &o->sfx_destroy->pos, 0.0f, SND_CAT_GAME, 0.0f);
    }

    o->fx_timer = 0.33f;

    SDRumbleType rt = SDR_Total;

    switch (o->dasset->fxType) {
    case 0:
        o->fx_emitter = NULL;
        break;
    case 1:
        o->fx_emitter = sEmitDust;
        rt = SDR_DustDestroyedObj;
        break;
    case 2:
        o->fx_emitter = sEmitXplo;
        rt = SDR_XploDestroyedObj;
        break;
    case 3:
        o->fx_emitter = sEmitWeb;
        rt = SDR_WebDestroyed;
        break;
    }

    if (rt != SDR_Total) {
        xVec3 var_28;
        xVec3Sub(&var_28, xEntGetPos(o), xEntGetPos(&globals.player.ent));

        if (xVec3Dot(&var_28, &var_28) <= 25.0f) {
            zRumbleStart(globals.currentActivePad, rt);
        }
    }
}

S32 zEntDestructObjEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget)
{
    zEntDestructObj* s = (zEntDestructObj*)to;

    switch (toEvent) {
    case eEventVisible:
    case eEventFastVisible:
        xEntShow(s);
        if (toParam && (S32)(0.5f + toParam[0]) == 77) {
            zFXPopOn(*s, toParam[1], toParam[2]);
        }
        break;
    case eEventInvisible:
    case eEventFastInvisible:
        xEntHide(s);
        if (toParam && (S32)(0.5f + toParam[0]) == 77) {
            zFXPopOff(*s, toParam[1], toParam[2]);
        }
        break;
    case eEventCollisionOn:
        s->chkby |= (k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC);
        s->bupdate(s, (xVec3*)&s->model->Mat->pos);
        break;
    case eEventCollisionOff:
        s->chkby &= (U8)~(k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC);
        break;
    case eEventCollision_Visible_On:
        s->chkby |= (k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC);
        xEntShow(s);
        s->bupdate(s, (xVec3*)&s->model->Mat->pos);
        if (toParam && (S32)(0.5f + toParam[0]) == 77) {
            zFXPopOn(*s, toParam[1], toParam[2]);
        }
        break;
    case eEventCollision_Visible_Off:
        s->chkby &= (U8)~(k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC);
        xEntHide(s);
        if (toParam && (S32)(0.5f + toParam[0]) == 77) {
            zFXPopOff(*s, toParam[1], toParam[2]);
        }
        break;
    case eEventCameraCollideOn:
        zCollGeom_CamEnable(s);
        break;
    case eEventCameraCollideOff:
        zCollGeom_CamDisable(s);
        break;
    case eEventDestroy:
        if (s->destroy_model) {
            SwapModel(s, s->destroy_model);
        } else {
            s->chkby &= (U8)~(k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC);
            xEntHide(s);
        }
        zEntDestructObj_DestroyFX(s);
        s->state = 2;
        if (s->shrapnel_destroy && s->shrapnel_destroy->initCB) {
            s->shrapnel_destroy->initCB(s->shrapnel_destroy, s->model, NULL, NULL);
        }
        if (s->destroy_notify) {
            s->destroy_notify(*s, s->notify_context);
        }
        if (s->driver) {
            s->driver->driving_count--;
            s->driver = NULL;
        }
        break;
    case eEventReset:
        zEntDestructObj_Reset(s, globals.sceneCur);
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
    case eEventHit:
        if (s->healthCnt) {
            s->healthCnt--;
            if (s->healthCnt && s->hit_model) {
                SwapModel(s, s->hit_model);
            }
            if (s->healthCnt == 0) {
                zEntEvent(s, s, eEventDestroy);
            }
        }
        break;
    case eEventSetUpdateDistance:
        if (globals.updateMgr) {
            if (toParam[0] <= 0.0f) {
                xUpdateCull_SetCB(globals.updateMgr, s, xUpdateCull_AlwaysTrueCB, NULL);
            } else {
                FloatAndVoid dist;
                dist.f = xsqr(toParam[0]);
                xUpdateCull_SetCB(globals.updateMgr, s, xUpdateCull_DistanceSquaredCB, dist.v);
            }
        }
        break;
    case eEventHit_BubbleBounce:
        zEntDestructObj_Hit(s, 0x2000);
        break;
    case eEventHit_BubbleBash:
        zEntDestructObj_Hit(s, 0x4000);
        break;
    case eEventHit_PatrickSlam:
        zEntDestructObj_Hit(s, 0x400);
        break;
    case eEventHit_Throw:
        zEntDestructObj_Hit(s, 0x800);
        break;
    case eEventLaunchShrapnel:
        if (toParamWidget) {
            zShrapnelAsset* shrap = (zShrapnelAsset*)toParamWidget;
            if (shrap->initCB) {
                shrap->initCB(shrap, s->model, NULL, NULL);
            }
        }
        break;
    }

    return 1;
}
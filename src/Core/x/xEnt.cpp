#include "xEnt.h"

#include "xDebug.h"
#include "xString.h"
#include "xScene.h"
#include "xSurface.h"
#include "xGroup.h"
#include "xstransvc.h"
#include "xFX.h"
#include "xShadow.h"
#include "xFFX.h"

#include "zBase.h"
#include "zPlatform.h"
#include "zEntDestructObj.h"
#include "zCollGeom.h"
#include "zSurface.h"
#include "zScene.h"
#include "zLight.h"
#include "zGrid.h"
#include "zEvent.h"

struct xEnt::anim_coll_data
{
    U32 flags;
    U32 bones;
    xMat4x3 old_mat;
    xMat4x3 new_mat;
    size_t verts_size;
    xVec3* verts;
    xVec3* normals;
};

namespace {
    namespace anim_coll {

#define GET_MODEL_MAT_ANIM_COLL(ent, modelvar, matvar, acvar)\
    xASSERT(ent.model != 0);\
    xASSERT(ent.model->Mat != 0);\
    xASSERT(ent.moreFlags & k_MORE_FLAGS_ANIM_COLL);\
    xASSERT(ent.anim_coll != 0);\
    xModelInstance& modelvar = *ent.model;\
    xMat4x3& matvar = *(xMat4x3*)modelvar.Mat;\
    xEnt::anim_coll_data& acvar = *ent.anim_coll

        void reset(xEnt& ent)
        {
            if (!ent.anim_coll)
            {
                ent.anim_coll = (xEnt::anim_coll_data*)xMALLOC(sizeof(xEnt::anim_coll_data));
                ent.anim_coll->flags = 0;
                ent.anim_coll->verts = NULL;
            }
            GET_MODEL_MAT_ANIM_COLL(ent, model, mat, ac);
            
            if (ac.flags & 0x8) return;

            switch (model.BoneCount)
            {
            case 0:
                xASSERTFAILFMT("Model with zero bones can't be animated. Asset: 0x%x", ent.asset->id);
                break;
            case 1:
                ac.flags |= 0x1;
                ac.old_mat = mat;
                ac.new_mat = g_I3;
                break;
            default:
                ac.flags |= 0x2;
                ac.old_mat = mat;
                ac.new_mat = g_I3;

                xModelAnimCollStart(model);

                xBox& box = ent.bound.box.box;
                xVec3 size = box.upper - box.lower;
                F32 max_size = size.x;
                if (max_size < size.y) max_size = size.y;
                if (max_size < size.z) max_size = size.z;
                max_size += 1.0f;
                box.upper += max_size;
                box.lower -= max_size;
                model.Data->boundingSphere.radius *= 3.0f;
                break;
            }
        }

        void refresh(xEnt& ent)
        {
            GET_MODEL_MAT_ANIM_COLL(ent, model, mat, ac);
            xASSERT(model.BoneCount > 0);
            xMat4x3& bone_mat = *(xMat4x3*)(model.Mat + 1);
            xMat4x3Mul(&mat, &bone_mat, &ac.old_mat);
            ac.new_mat = bone_mat;
            bone_mat = g_I3;
        }

        inline void pre_move(xEnt& ent)
        {
            xModelInstance& model = *ent.model;
            xMat4x3& mat = *(xMat4x3*)model.Mat;
            xEnt::anim_coll_data& ac = *ent.anim_coll;
            ent.frame->mat = mat = ac.old_mat;
        }

        inline void post_move(xEnt& ent)
        {
            xModelInstance& model = *ent.model;
            xMat4x3& mat = *(xMat4x3*)model.Mat;
            xEnt::anim_coll_data& ac = *ent.anim_coll;
            ac.old_mat = ent.frame->mat;
            xMat4x3Mul(&mat, &ac.new_mat, &ac.old_mat);
            ent.frame->mat = mat;
        }
    }
}

static F32 nsn_angle = xdeg2rad(30.0f);
static F32 sEntityTimePassed;
static xBox all_ents_box;
static S32 all_ents_box_init;
static S32 setMaterialTextureRestore;
S32 sSetPipeline;
static RxPipeline* oldPipe;
S32 xent_entent;

void xEntSetTimePassed(F32 sec)
{
    sEntityTimePassed = sec;
}

void xEntSceneInit()
{
    all_ents_box_init = 1;
}

void xEntSceneExit()
{
}

static void xEntAddHittableFlag(xEnt* ent)
{
    if (ent->baseType == eBaseTypeNPC ||
        ent->baseType == eBaseTypeDestructObj ||
        ent->baseType == eBaseTypeButton ||
        ent->baseType == eBaseTypeBoulder ||
        (ent->baseType == eBaseTypePlatform && ent->subType == ePlatformTypePaddle)) {
        ent->moreFlags |= k_MORE_FLAGS_HITTABLE;
    } else {
        for (U32 i = 0; i < ent->linkCount; i++) {
            if (ent->link[i].srcEvent == eEventHit ||
                ent->link[i].srcEvent == eEventHit_Melee ||
                ent->link[i].srcEvent == eEventHit_BubbleBounce ||
                ent->link[i].srcEvent == eEventHit_BubbleBash ||
                ent->link[i].srcEvent == eEventHit_BubbleBowl ||
                ent->link[i].srcEvent == eEventHit_Cruise ||
                ent->link[i].srcEvent == eEventHit_PatrickSlam ||
                ent->link[i].srcEvent == eEventHit_Throw ||
                ent->link[i].srcEvent == eEventHit_PaddleLeft ||
                ent->link[i].srcEvent == eEventHit_PaddleRight) {
                ent->moreFlags |= k_MORE_FLAGS_HITTABLE;
                break;
            }
        }
    }
}

static void hack_receive_shadow(xEnt* ent) NONMATCH("https://decomp.me/scratch/YPihn")
{
    static U32 receive_models[] = {
        xStrHash("db03_path_a"),
        xStrHash("db03_path_b"),
        xStrHash("db03_path_c"),
        xStrHash("db03_path_d"),
        xStrHash("db03_path_e"),
        xStrHash("db03_path_f"),
        xStrHash("db03_path_g"),
        xStrHash("db03_path_h"),
        xStrHash("db03_path_i"),
        xStrHash("db03_path_j"),
        xStrHash("db03_path_k"),
        xStrHash("db03_path_l"),
        xStrHash("db03_path_m"),
        xStrHash("db03_path_o"),
        xStrHash("db03_path_p")
    };

    const U32* end_receive_models = receive_models + sizeof(receive_models) / sizeof(U32);
    for (U32* id = receive_models; id != end_receive_models; id++) {
        if (ent->asset->modelInfoID == *id) {
            ent->baseFlags |= k_XBASE_RECEIVES_SHADOWS;
            ent->asset->baseFlags |= k_XBASE_RECEIVES_SHADOWS;
            break;
        }
    }
}

static void xEntAddShadowRecFlag(xEnt* ent)
{
    switch (ent->baseType) {
    case eBaseTypePlatform:
    case eBaseTypeButton:
    case eBaseTypeNPC:
    case eBaseTypeStatic:
    case eBaseTypeBoulder:
    case eBaseTypeDestructObj:
        if (ent->model->PipeFlags & 0xFF00) {
            ent->baseFlags &= (U16)~k_XBASE_RECEIVES_SHADOWS;
        }
        break;
    default:
        ent->baseFlags &= (U16)~k_XBASE_RECEIVES_SHADOWS;
    }
    hack_receive_shadow(ent);
}

void xEntInit(xEnt* ent, xEntAsset* asset)
{
    xBaseInit(ent, asset);
    
    ent->asset = asset;
    ent->update = xEntUpdate;
    ent->bupdate = xEntDefaultBoundUpdate;
    ent->render = xEntRender;
    ent->move = NULL;
    ent->transl = xEntDefaultTranslate;
    ent->flags = asset->flags;
    ent->miscflags = 0;
    ent->moreFlags = asset->moreFlags;
    ent->subType = asset->subtype;
    ent->pflags = asset->pflags;
    ent->ffx = NULL;
    ent->num_ffx = 0;
    ent->driver = NULL;
    ent->model = NULL;
    ent->collModel = NULL;
    ent->camcollModel = NULL;
    ent->frame = NULL;
    ent->collis = NULL;
    ent->lightKit = NULL;
    ent->simpShadow = NULL;
    ent->entShadow = NULL;
    ent->baseFlags |= k_XBASE_IS_ENTITY;

    xGridBoundInit(&ent->gridb, ent);

    ent->anim_coll = NULL;

    if (all_ents_box_init) {
        iBoxInitBoundVec(&all_ents_box, &asset->pos);
        all_ents_box_init = 0;
    } else {
        iBoxBoundVec(&all_ents_box, &all_ents_box, &asset->pos);
    }
}

void xEntInitForType(xEnt* ent)
{
    ent->update = xEntUpdate;
    ent->render = xEntRender;

    if (ent->collType == k_XENT_COLLTYPE_TRIG) {
        ent->pflags &= ~(k_XENT_IS_MOVING | k_XENT_HAS_VELOCITY);
        ent->chkby = k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC;
        ent->penby = 0;
    } else if (ent->collType == k_XENT_COLLTYPE_STAT) {
        ent->pflags &= ~(k_XENT_IS_MOVING | k_XENT_HAS_VELOCITY);
        ent->chkby = k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC;
        ent->penby = k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC;
    } else if (ent->collType == k_XENT_COLLTYPE_DYN) {
        ent->pflags |= k_XENT_IS_MOVING;
        ent->move = NULL;
        ent->frame = (xEntFrame*)xMALLOC(sizeof(xEntFrame));
        memset(ent->frame, 0, sizeof(xEntFrame));
        ent->pflags &= (U8)~k_XENT_HAS_VELOCITY;
        ent->chkby = k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC;
        ent->penby = k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC;
    } else if (ent->collType == k_XENT_COLLTYPE_NPC) {
        ent->pflags |= k_XENT_IS_MOVING;
        ent->move = NULL;
        ent->pflags |= k_XENT_HAS_VELOCITY;
        ent->chkby = k_XENT_COLLTYPE_PC;
        ent->penby = k_XENT_COLLTYPE_PC;
    } else if (ent->collType == k_XENT_COLLTYPE_PC) {
        ent->pflags |= k_XENT_IS_MOVING;
        ent->move = NULL;
        ent->frame = (xEntFrame*)xMALLOC(sizeof(xEntFrame));
        memset(ent->frame, 0, sizeof(xEntFrame));
        ent->pflags |= k_XENT_HAS_VELOCITY;
        ent->chkby = 0;
        ent->penby = 0;
        ent->collis = (xEntCollis*)xMALLOC(sizeof(xEntCollis));
        ent->collis->chk = 0x2F;
        ent->collis->pen = 0x2E;
        ent->collis->post = NULL;
        ent->collis->depenq = NULL;
    }

    if ((ent->moreFlags & k_MORE_FLAGS_ANIM_COLL) || (ent->flags & k_XENT_IS_STACKED)) {
        if (!ent->frame) {
            ent->frame = (xEntFrame*)xMALLOC(sizeof(xEntFrame));
            memset(ent->frame, 0, sizeof(xEntFrame));
        }
    }

    ent->baseFlags |= k_XBASE_IS_ENTITY;
}

namespace {
    F32 get_lower_extent(const xBound& bound)
    {
        switch (bound.type) {
        case k_XBOUNDTYPE_SPHERE:
            return bound.sph.r;
        case k_XBOUNDTYPE_BOX:
            return bound.box.center.y - bound.box.box.lower.y;
        case k_XBOUNDTYPE_OBB:
        {
            xMat4x3& mat = *bound.mat;
            if (mat.up.x == 0.0f && mat.up.z == 0.0f) {
                F32 y = mat.pos.y + mat.up.y * bound.box.box.lower.y;
                return bound.box.center.y - y;
            }
        
            xBox abox;
            xBoundGetBox(abox, bound);
            return bound.box.center.y - abox.lower.y;
        }
        }

        return 0.0f;
    }

    bool collide_downward(xVec3& loc, xEnt*& hit, xScene& s, xEnt& ent, F32 max_dist) NONMATCH("https://decomp.me/scratch/THH5V")
    {
        F32 lower_bound_extent = get_lower_extent(ent.bound);
        xVec3& center = *xBoundCenter(&ent.bound);

        xRay3 ray;
        ray.origin = center;
        ray.dir.assign(0.0f, -1.0f, 0.0f);
        ray.flags = XRAY3_USE_MIN | XRAY3_USE_MAX;
        ray.min_t = 0.0f;
        ray.max_t = max_dist + lower_bound_extent;

        U8 old_bound_type = ent.bound.type;
        F32 old_bound_radius = ent.bound.sph.r;

        ent.bound.type = k_XBOUNDTYPE_SPHERE;
        ent.bound.sph.r = 0.0f;
        center.y = HUGE;

        xCollis coll;
        coll.flags = k_HIT_0x100;

        xRayHitsSceneFlags(&s, &ray, &coll, 0x10, 0x26);

        ent.bound.type = old_bound_type;
        ent.bound.sph.r = old_bound_radius;
        center.y = ray.origin.y;

        if (!(coll.flags & k_HIT_IT)) return false;

        *(xVec3*)&ent.model->Mat->pos = loc;
        loc.y -= coll.dist - lower_bound_extent;
        hit = (xEnt*)coll.optr;

        return true;
    }

    void drop_stacked_entity(xEnt& ent);

    void stacked_owner_destroyed(zEntDestructObj&, void* context)
    {
        drop_stacked_entity(*(xEnt*)context);
    }

    void mount_stacked_entity(xEnt& ent, xEnt& driver)
    {
        if (driver.collType != k_XENT_COLLTYPE_DYN) return;

        if (ent.driver) {
            ent.driver->driving_count--;
        }
        ent.driver = &driver;
        driver.driving_count++;

        if (driver.baseType == eBaseTypeDestructObj) {
            zEntDestructObj& durst = (zEntDestructObj&)driver;
            durst.destroy_notify = stacked_owner_destroyed;
            durst.notify_context = &ent;
        }
    }

    void dismount_stacked_entity(xEnt& ent)
    {
        if (!ent.driver) return;

        if (ent.driver->baseType == eBaseTypeDestructObj) {
            zEntDestructObj& durst = *(zEntDestructObj*)ent.driver;
            durst.destroy_notify = NULL;
            durst.notify_context = NULL;
        }

        ent.driver = NULL;
    }

    void setup_stacked_entity(xEnt& ent)
    {
        ent.pflags = k_XENT_HAS_GRAVITY;
    }

    void drop_stacked_entity(xEnt& ent)
    {
        ent.pflags = k_XENT_HAS_GRAVITY;
        dismount_stacked_entity(ent);
    }

    void stop_stacked_entity(xEnt& ent)
    {
        ent.pflags = 0;
    }

    void update_stacked_entity(xScene& s, xEnt& ent, F32 dt)
    {
        xEntApplyPhysics(&ent, &s, dt);
        xEntMove(&ent, &s, dt);

        F32 dist = ent.model->Mat->pos.y - ent.frame->mat.pos.y;
        if (dist <= 0.0f) return;

        xVec3 hitloc;
        xEnt* hitent;
        if (collide_downward(hitloc, hitent, s, ent, dist)) {
            ent.frame->mat.pos.y = hitloc.y;
            stop_stacked_entity(ent);
            if (hitent) {
                mount_stacked_entity(ent, *hitent);
            }
        }
    }
}

void xEntSetup(xEnt* ent)
{
    xBaseSetup(ent);
    ent->baseFlags |= k_XBASE_IS_ENTITY;

    if (ent->asset->surfaceID != 0) {
        xSurface* surf = (xSurface*)xSceneResolvID(g_xSceneCur, ent->asset->surfaceID);
        if (surf) {
            surf->type = k_XSURFACETYPE_ENT;
            surf->ent = ent;
    
            xModelInstance* minst = ent->model;
            while (minst) {
                minst->Surf = surf;
                minst = minst->Next;
            }
        }
    }
    
    for (S32 i = 0; i < ent->linkCount; i++) {
        const xLinkAsset* la = &ent->link[i];
        if (la->dstEvent == eEventDrivenby) {
            xEnt* dent = (xEnt*)xSceneResolvID(g_xSceneCur, la->dstAssetID);
            if (dent) {
                ent->driver = dent;
                ent->driveMode = (S32)(la->param[0]);
                dent->driving_count++;
            }
        }
    }
    
    ent->model->RedMultiplier = ent->asset->redMult;
    ent->model->GreenMultiplier = ent->asset->greenMult;
    ent->model->BlueMultiplier = ent->asset->blueMult;
    ent->model->Alpha = ent->asset->seeThru;

    xEntAddHittableFlag(ent);
    xEntAddShadowRecFlag(ent);
    
    zCollGeom_EntSetup(ent);
    if (ent->model) {
        if (ent->bound.type == k_XBOUNDTYPE_BOX) {
            iBoxForModel(&ent->bound.box.box, ent->collModel ? ent->collModel : ent->model);
        } else if (ent->bound.type == k_XBOUNDTYPE_OBB) {
            iBoxForModelLocal(&ent->bound.box.box, ent->collModel ? ent->collModel : ent->model);
        }
    }

    if (ent->moreFlags & k_MORE_FLAGS_ANIM_COLL) {
        anim_coll::reset(*ent);
    }

    if (ent->flags & k_XENT_IS_STACKED) {
        setup_stacked_entity(*ent);
    }
}

void xEntSave(xEnt* ent, xSerial* s)
{
    xBaseSave(ent, s);
    if (xEntIsVisible(ent)) {
        s->Write_b1(1);
    } else {
        s->Write_b1(0);
    }
}

void xEntLoad(xEnt* ent, xSerial* s)
{
    xBaseLoad(ent, s);
    S32 b = 0;
    s->Read_b1(&b);
    if (b) {
        xEntShow(ent);
    } else {
        xEntHide(ent);
    }
}

void xEntReset(xEnt* ent) NONMATCH("https://decomp.me/scratch/ZQc8z")
{
    xBaseReset(ent, ent->asset);
    
    ent->baseFlags |= k_XBASE_IS_ENTITY;
    ent->flags = ent->asset->flags;
    ent->miscflags = 0;
    ent->moreFlags = ent->asset->moreFlags;

    xEntAddHittableFlag(ent);
    xEntAddShadowRecFlag(ent);
    
    xMat4x3 frame;
    xMat3x3Euler(&frame, ent->asset->ang.x, ent->asset->ang.y, ent->asset->ang.z);

    xVec3SMulBy(&frame.right, ent->asset->scale.x);
    xVec3SMulBy(&frame.up, ent->asset->scale.y);
    xVec3SMulBy(&frame.at, ent->asset->scale.z);
    xVec3Copy(&frame.pos, &ent->asset->pos);
    frame.flags = 0;

    if (ent->model) {
        xModelSetFrame(ent->model, &frame);
        if (ent->collModel) {
            xModelSetFrame(ent->collModel, &frame);
        }
        
        if (ent->moreFlags & k_MORE_FLAGS_ANIM_COLL) {
            anim_coll::reset(*ent);
        }

        xModelInstance* minst = ent->model;
        while (minst) {
            minst->RedMultiplier = ent->asset->redMult;
            minst->GreenMultiplier = ent->asset->greenMult;
            minst->BlueMultiplier = ent->asset->blueMult;
            minst->Alpha = minst->Data->geometry->matList.materials[0]->color.alpha / 255.0f;
            minst->Scale.x = 0.0f;
            minst->Scale.y = 0.0f;
            minst->Scale.z = 0.0f;
            minst = minst->Next;
        }
    }

    if (ent->frame) {
        xMat4x3Copy(&ent->frame->mat, &frame);
        ent->frame->oldmat = ent->frame->mat;
        xVec3Copy(&ent->frame->dpos, &g_O3);
        xVec3Copy(&ent->frame->dvel, &g_O3);
        xVec3Copy(&ent->frame->vel, &g_O3);
        xVec3Copy(&ent->frame->oldvel, &g_O3);
        xVec3Copy(&ent->frame->rot.axis, &ent->asset->ang);
        ent->frame->rot.angle = 0.0f;
        xRotCopy(&ent->frame->oldrot, &ent->frame->rot);
    }

    if (ent->bupdate && ent->model) {
        ent->bupdate(ent, (xVec3*)&ent->model->Mat->pos);
    }

    ent->num_updates = xrand() % 128;

    if (ent->flags & k_XENT_IS_STACKED) {
        setup_stacked_entity(*ent);
    }
}

xModelInstance* xEntLoadModel(xEnt* ent, RpAtomic* imodel)
{
    xModelInstance* model = xModelInstanceAlloc(imodel, ent, 0, 0, NULL);
    while (imodel = iModelFile_RWMultiAtomic(imodel)) {
        xModelInstanceAttach(xModelInstanceAlloc(imodel, ent, 0x8, 0, NULL), model);
    }
    if (ent) {
        ent->model = model;
    }
    return model;
}

static void xEntAddToPos(xEnt* ent, const xVec3* dpos)
{
    xVec3AddTo(&ent->frame->mat.pos, dpos);
}

void xEntSetupPipeline(xModelInstance* model)
{
    xEntSetupPipeline(model->Surf, model->Data);
}

void xEntSetupPipeline(xSurface* surf, RpAtomic* model)
{
    setMaterialTextureRestore = 0;
    sSetPipeline = 0;

    if (!surf) return;

    zSurfaceProps* pp = (zSurfaceProps*)surf->moprops;
    if (!pp) return;

    if (pp->texanim_flags & SURF_TEXANIM_ON) {
        xGroup* g = (xGroup*)zSceneFindObject(pp->texanim[0].group);
        if (g) {
            U32 texid = xGroupGetItem(g, pp->texanim[0].group_idx);
            RwTexture* texptr = (RwTexture*)xSTFindAsset(texid, NULL);
            if (texptr) {
                iModelSetMaterialTexture(model, texptr);
                setMaterialTextureRestore = 1;
            }
        }
    }

    if (pp->texanim_flags & SURF_TEXANIM_ON2) {
        xGroup* g = (xGroup*)zSceneFindObject(pp->texanim[1].group);
        if (g) {
            U32 texid = xGroupGetItem(g, pp->texanim[1].group_idx);
            RwTexture* texptr = (RwTexture*)xSTFindAsset(texid, NULL);
            if (texptr) {
                xFXanimUV2PSetTexture(texptr);
                sSetPipeline = 1;
            }
        }
    } else if (pp->uvfx_flags & UVANIM_FLAG_ON2) {
        RwTexture* texptr = (RwTexture*)xSTFindAsset(pp->asset->matfx.dualmapID, NULL);
        if (texptr) {
            xFXanimUV2PSetTexture(texptr);
            sSetPipeline = 1;
        }
    } else {
        xFXanimUV2PSetTexture(NULL);
    }

    if (pp->uvfx_flags & (UVANIM_FLAG_ON | UVANIM_FLAG_ON2)) {
        sSetPipeline = 1;
        xFXanimUVSetTranslation(&pp->uvfx[0].trans);
        xFXanimUV2PSetTranslation(&pp->uvfx[1].trans);
        xFXanimUVSetScale(&pp->uvfx[0].scale);
        xFXanimUV2PSetScale(&pp->uvfx[1].scale);
        xFXanimUVSetAngle((1/180.0f/PI) * pp->uvfx[0].rot);
        xFXanimUV2PSetAngle((1/180.0f/PI) * pp->uvfx[1].rot);
    }

    if (sSetPipeline) {
        oldPipe = model->pipeline;
        xFXanimUVAtomicSetup(model);
    }
}

void xEntRestorePipeline(xModelInstance* model)
{
    xEntRestorePipeline(model->Surf, model->Data);
}

void xEntRestorePipeline(xSurface*, RpAtomic* model)
{
    if (setMaterialTextureRestore) {
        iModelResetMaterial(model);
        setMaterialTextureRestore = 0;
    }
    if (sSetPipeline) {
        model->pipeline = oldPipe;
        sSetPipeline = 0;
    }
}

void xEntRender(xEnt* ent) NONMATCH("https://decomp.me/scratch/7rdkL")
{
    if (!ent->model || !xEntIsVisible(ent) || (ent->model->Flags & 0x400)) return;

    ent->isCulled = 0;

    if (ent->baseType == eBaseTypePlayer ||
        (ent->baseType == eBaseTypeNPC && !(ent->flags & k_XENT_0x40))) {
        S32 shadowOutside;
        xVec3 shadVec;
        shadVec.x = ent->model->Mat->pos.x;
        shadVec.y = ent->model->Mat->pos.y - 10.0f;
        shadVec.z = ent->model->Mat->pos.z;
        if (iModelCullPlusShadow(ent->model->Data, ent->model->Mat, &shadVec, &shadowOutside)) {
            if (shadowOutside) {
                ent->isCulled = 1;
                return;
            } else {
                goto render_shadow;
            }
        }
    } else if (iModelCull(ent->model->Data, ent->model->Mat)) {
        ent->isCulled = 1;
        return;
    }

    xModelRender(ent->model);

render_shadow:
    if ((ent->baseType == eBaseTypeNPC && !(ent->flags & k_XENT_0x40)) ||
        ent->baseType == eBaseTypePlayer) {
        zLightAddLocal(ent);
        xShadow_ListAdd(ent);
    }
}

void xEntUpdate(xEnt* ent, xScene* sc, F32 dt)
{
    xEntBeginUpdate(ent, sc, dt);

    if (ent->pflags & k_XENT_HAS_VELOCITY) {
        xEntApplyPhysics(ent, sc, dt);
    }

    if (ent->pflags & k_XENT_IS_MOVING) {
        xEntMove(ent, sc, dt);
    }

    xFFXApply(ent, sc, dt);

    if (ent->collis) {
        xEntCollide(ent, sc, dt);
    }

    if ((ent->flags & k_XENT_IS_STACKED) && (ent->pflags & k_XENT_HAS_GRAVITY)) {
        update_stacked_entity(*sc, *ent, dt);
    }

    xEntEndUpdate(ent, sc, dt);
}

void xEntBeginUpdate(xEnt* ent, xScene* sc, F32 dt)
{
    if (ent->model) {
        xModelUpdate(ent->model, dt);
        if (ent->frame) {
            xVec3Copy(&ent->frame->oldvel, &ent->frame->vel);
            ent->frame->oldmat = ent->frame->mat;
            xRotCopy(&ent->frame->oldrot, &ent->frame->rot);
            xMat4x3Copy(&ent->frame->mat, xModelGetFrame(ent->model));
            ent->frame->mode = 0;
        }
    }
}

void xEntEndUpdate(xEnt* ent, xScene* sc, F32 dt)
{
    ent->num_updates++;

    if (ent->model) {
        if (ent->frame) {
            if (!(ent->frame->mode & k_XENT_MODE_0x20000)) {
                xMat3x3Copy((xMat3x3*)ent->model->Mat, &ent->frame->mat);
            }
            if (!(ent->frame->mode & k_XENT_MODE_0x10000)) {
                xVec3* mpos = (xVec3*)&ent->model->Mat->pos;
                xVec3* upos = &ent->frame->mat.pos;
                xVec3Copy(mpos, upos);
            }
        }

        if (ent->bupdate) {
            ent->bupdate(ent, (xVec3*)&ent->model->Mat->pos);
        }

        xModelEval(ent->model);

        if (ent->moreFlags & k_MORE_FLAGS_ANIM_COLL) {
            anim_coll::refresh(*ent);
        }

        if (ent->endUpdate) {
            ent->endUpdate(ent, sc, dt);
        }
    }
}

void xEntDefaultBoundUpdate(xEnt* ent, xVec3* pos) NONMATCH("https://decomp.me/scratch/QboCV")
{
    xBound* bound = &ent->bound;
    if (bound->type == k_XBOUNDTYPE_SPHERE) {
        xVec3Copy(&bound->sph.center, pos);
        bound->sph.center.y += 0.7f;
        bound->sph.r = 0.7f;
    }
    xBoundUpdate(bound);
    zGridUpdateEnt(ent);
}

void xEntDefaultTranslate(xEnt* ent, xVec3* dpos, xMat4x3* dmat)
{
    if (dmat) {
        if (ent->model) {
            xMat4x3Mul((xMat4x3*)ent->model->Mat, (xMat4x3*)ent->model->Mat, dmat);
        }
        if (ent->frame) {
            xMat4x3Mul(&ent->frame->mat, &ent->frame->mat, dmat);
        }
        xMat4x3Toworld(xEntGetCenter(ent), dmat, xEntGetCenter(ent));
    } else {
        if (ent->model) {
            xVec3AddTo(xEntGetPos(ent), dpos);
        }
        if (ent->frame) {
            xVec3AddTo(&ent->frame->mat.pos, dpos);
        }
        xVec3AddTo(xEntGetCenter(ent), dpos);
    }
}

static void xEntRotationToMatrix(xEntFrame* frame)
{
    if (!(frame->mode & k_XENT_MODE_0x20)) return;

    if (frame->mode & k_XENT_MODE_0x400) {
        xVec3AddTo(&frame->rot.axis, &frame->drot.axis);
        xMat3x3Euler(&frame->mat, frame->rot.axis.x, frame->rot.axis.y, frame->rot.axis.z);
    } else {
        frame->rot.angle = xAngleClamp(frame->rot.angle + frame->drot.angle);
        xMat3x3Rot(&frame->mat, &frame->rot.axis, frame->rot.angle);
    }
}

void xEntMotionToMatrix(xEnt* ent, xEntFrame* frame)
{
    if (frame->mode & k_XENT_MODE_0x1000) {
        xEntRotationToMatrix(frame);
    }

    if (frame->mode & k_XENT_MODE_0x2) {
        if (frame->mode & k_XENT_MODE_0x800) {
            xMat3x3RMulVec(&frame->dpos, &frame->mat, &frame->dpos);
        }
        xEntAddToPos(ent, &frame->dpos);
    }

    if (frame->mode & k_XENT_MODE_0x8) {
        if (frame->mode & k_XENT_MODE_0x800) {
            xMat3x3RMulVec(&frame->dvel, &frame->mat, &frame->dvel);
        }
        xVec3AddTo(&frame->vel, &frame->dvel);
    }

    if (!(frame->mode & k_XENT_MODE_0x1000)) {
        xEntRotationToMatrix(frame);
    }
}

void xEntMove(xEnt* ent, xScene* sc, F32 dt)
{
    if (ent->moreFlags & k_MORE_FLAGS_ANIM_COLL) {
        anim_coll::pre_move(*ent);
    }

    ent->move(ent, sc, dt, ent->frame);
    xEntMotionToMatrix(ent, ent->frame);

    if (ent->driver) {
        xEntFrame* dframe = ent->driver->frame;
        if (ent->driveMode == 0) {
            xVec3 dpos;
            xVec3Sub(&dpos, &dframe->mat.pos, &dframe->oldmat.pos);
            ent->transl(ent, &dpos, NULL);
        } else if (ent->driveMode == 1) {
            RwMatrixUpdate((RwMatrix*)&dframe->oldmat);
            xMat4x3 invOldmat;
            RwMatrixInvert((RwMatrix*)&invOldmat, (RwMatrix*)&dframe->oldmat);
            xMat4x3 deltaMat;
            xMat4x3Mul(&deltaMat, &invOldmat, &dframe->mat);
            ent->transl(ent, NULL, &deltaMat);
        }
    }

    if (ent->moreFlags & k_MORE_FLAGS_ANIM_COLL) {
        anim_coll::post_move(*ent);
    }
}

void xEntApplyPhysics(xEnt* ent, xScene* sc, F32 dt)
{
    xVec3 dposvel = {};
    
    if ((ent->pflags & k_XENT_HAS_GRAVITY) && (sc->flags & k_XSCENE_HAS_GRAVITY)) {
        ent->frame->vel.y += sc->gravity * dt;
    }

    if ((ent->pflags & k_XENT_HAS_FRICTION) && (sc->flags & k_XSCENE_HAS_FRICTION)) {
        F32 tfric = 1.0f - sc->friction * dt;
        xVec3SMulBy(&ent->frame->vel, tfric);
    }

    if ((ent->pflags & k_XENT_HAS_DRAG) && (sc->flags & k_XSCENE_HAS_DRAG)) {
        F32 tdrag = 1.0f - sc->drag * dt;
        xVec3SMulBy(&ent->frame->vel, tdrag);
    }

    xVec3Add(&dposvel, &ent->frame->vel, &ent->frame->oldvel);
    xVec3SMulBy(&dposvel, 0.5f * dt);

    if (dposvel.y < 0.0f) {
        F32 dposXZ = xsqrt(xsqr(dposvel.x) + xsqr(dposvel.z));
        F32 scaleXZ = (dposXZ > EPSILON) ? (30.0f * dt * 0.63f / dposXZ) : 0.0f;
        F32 scaleY = 30.0f * dt * 0.63f / xabs(dposvel.y);
        if (scaleXZ < 1.0f) {
            dposvel.x *= scaleXZ;
            dposvel.z *= scaleXZ;
        }
        if (scaleY < 1.0f) {
            dposvel.y *= scaleY;
        }
    }
    
    xEntAddToPos(ent, &dposvel);
}

void xEntCollide(xEnt* ent, xScene* sc, F32 dt)
{
    if (!ent->model) return;

    if (ent->collis->chk & 0x2E) {
        xEntBeginCollide(ent, sc, dt);
    }
    if (ent->collis->chk & 0x8) {
        xEntCollCheckNPCsByGrid(ent, sc, xEntCollCheckOneEntNoDepen);
    }
    if (ent->collis->chk & 0x6) {
        xEntCollCheckByGrid(ent, sc, xEntCollCheckOneEntNoDepen);
    }
    if (ent->collis->chk & 0x20) {
        xEntCollCheckEnv(ent, sc);
    }
    if (ent->collis->chk & 0x2E) {
        F32 sbr = (ent->bound.type == k_XBOUNDTYPE_SPHERE) ? ent->bound.sph.r : 0.7f;
        if (ent->pflags & 0x80) {
            xCollis* coll = ent->collis->colls;
            if (coll->flags & k_HIT_IT) {
                F32 h_dot_n = xVec3Dot(&coll->hdng, &coll->norm);
                if (h_dot_n > 0.0f) {
                    xVec3Inv(&coll->norm, &coll->norm);
                    h_dot_n = -h_dot_n;
                }
                F32 depen_len = h_dot_n * coll->dist + sbr;
                if (depen_len < 0.0f || depen_len > sbr) {
                    depen_len = xclamp(depen_len, 0.0f, sbr);
                }
                xVec3SMul(&coll->depen, &coll->norm, depen_len);
            }
        }
        if (ent->frame->vel.y <= 0.0f) {
            xEntCollideFloor(ent, sc, dt);
        } else {
            xEntCollideCeiling(ent, sc, dt);
        }
        xEntCollideWalls(ent, sc, dt);
        xEntEndCollide(ent, sc, dt);
    }
}

void xEntBeginCollide(xEnt* ent, xScene* sc, F32 dt) NONMATCH("https://decomp.me/scratch/0Z2zb")
{
    if (ent->bupdate) {
        ent->bupdate(ent, &ent->frame->mat.pos);
    }
    
    for (U8 idx = 0; idx < 18; idx++) {
        xCollis* coll = &ent->collis->colls[idx];
        coll->flags = k_HIT_CALC_HDNG | k_HIT_0xF00;
        coll->optr = NULL;
        coll->mptr = NULL;
        coll->dist = HUGE;
    }

    ent->collis->idx = 6;
    ent->collis->stat_sidx = 6;
    ent->collis->stat_eidx = 6;
    ent->collis->dyn_sidx = 6;
    ent->collis->dyn_eidx = 6;
    ent->collis->npc_sidx = 6;
    ent->collis->npc_eidx = 6;
    ent->collis->env_sidx = 6;
    ent->collis->env_eidx = 6;
}

void xEntEndCollide(xEnt* ent, xScene* sc, F32 dt) WIP
{
    if (ent->collis->post) {
        ent->collis->post(ent, sc, dt, ent->collis);
    }
}

void xEntCollCheckEnv(xEnt* p, xScene* sc) NONMATCH("https://decomp.me/scratch/m7dSL")
{
    p->collis->env_sidx = p->collis->idx;
    
    xCollis* coll = &p->collis->colls[p->collis->idx];
    coll->flags = k_HIT_CALC_HDNG | k_HIT_0xF00;
    
    U8 ncolls = iSphereHitsEnv3(&p->bound.sph, sc->env, coll, 18 - p->collis->idx, xdeg2rad(45.0f));
    p->collis->idx += ncolls;
    p->collis->env_eidx = p->collis->idx;
}

static void xEntCollCheckOneGrid(xEnt* p, xScene* sc, xEntCallback hitIt, xGrid* grid)
{
    xGridIterator it;
    xEnt* ent;
    xVec3* pcenter = xEntGetCenter(p);
    S32 px, pz;
    xGridBound* cell = xGridIterFirstCell(grid, pcenter->x, pcenter->y, pcenter->z, px, pz, it);
    
    while (cell) {
        xBound* cellbound = (xBound*)(cell + 1);
        if (xQuickCullIsects(&p->bound.qcd, &cellbound->qcd)) {
            ent = (xEnt*)cell->data;
            hitIt(ent, sc, p);
        }
        cell = xGridIterNextCell(it);
    }

    xBox clbox;
    clbox.lower.x = grid->csizex * px,
        clbox.lower.z = grid->csizez * pz;
    clbox.lower.x += grid->minx,
        clbox.lower.z += grid->minz;

    F32 clcenterx = 0.5f * grid->csizex;
    F32 clcenterz = 0.5f * grid->csizez;
    clcenterx += clbox.lower.x;
    clcenterz += clbox.lower.z;

    static S32 offs[4][3][2];
    static S32 k;
    if (pcenter->x < clcenterx) {
        if (pcenter->z < clcenterz) {
            k = 0;
        } else {
            k = 1;
        }
    } else {
        if (pcenter->z < clcenterz) {
            k = 3;
        } else {
            k = 2;
        }
    }

    for (S32 i = 0; i < 3; i++) {
        S32 _x = px + offs[k][i][1];
        S32 _z = pz + offs[k][i][0];
        cell = xGridIterFirstCell(grid, _x, _z, it);
        while (cell) {
            xBound* cellbound = (xBound*)(cell + 1);
            if (xQuickCullIsects(&p->bound.qcd, &cellbound->qcd)) {
                ent = (xEnt*)cell->data;
                hitIt(ent, sc, p);
            }
            cell = xGridIterNextCell(it);
        }
    }

    cell = xGridIterFirstCell(&grid->other, it);
    while (cell) {
        xBound* cellbound = (xBound*)(cell + 1);
        if (xQuickCullIsects(&p->bound.qcd, &cellbound->qcd)) {
            ent = (xEnt*)cell->data;
            hitIt(ent, sc, p);
        }
        cell = xGridIterNextCell(it);
    }
}

void xEntCollCheckByGrid(xEnt* p, xScene* sc, xEntCallback hitIt)
{
    p->collis->stat_sidx = p->collis->idx;
    p->collis->dyn_sidx = p->collis->idx;
    xEntCollCheckOneGrid(p, sc, hitIt, &colls_grid);
    xEntCollCheckOneGrid(p, sc, hitIt, &colls_oso_grid);
    p->collis->stat_eidx = p->collis->idx;
    p->collis->dyn_eidx = p->collis->idx;
}

void xEntCollCheckNPCsByGrid(xEnt* p, xScene* sc, xEntCallback hitIt)
{
    p->collis->npc_sidx = p->collis->idx;
    xEntCollCheckOneGrid(p, sc, hitIt, &npcs_grid);
    p->collis->npc_eidx = p->collis->idx;
}

void xEntCollCheckStats(xEnt* p, xScene* sc, xEntCallback hitIt)
{
    p->collis->stat_sidx = p->collis->idx;
    xSceneForAllStatics(sc, hitIt, p);
    p->collis->stat_eidx = p->collis->idx;
}

void xEntCollCheckDyns(xEnt* p, xScene* sc, xEntCallback hitIt)
{
    p->collis->dyn_sidx = p->collis->idx;
    xSceneForAllDynamics(sc, hitIt, p);
    p->collis->dyn_eidx = p->collis->idx;
}

void xEntCollCheckNPCs(xEnt* p, xScene* sc, xEntCallback hitIt)
{
    p->collis->npc_sidx = p->collis->idx;
    xSceneForAllNPCs(sc, hitIt, p);
    p->collis->npc_eidx = p->collis->idx;
}

xEnt* xEntCollCheckOneEntNoDepen(xEnt* ent, xScene* sc, void* data) NONMATCH("https://decomp.me/scratch/yI2NT")
{
    xEnt* p = (xEnt*)data;
    xCollis* coll;
    U32 modl_coll = 0;
    
    xent_entent = 1;
    
    if (p->collis->idx >= 15) {
        xent_entent = 0;
        return NULL;
    }

    if (!(ent->chkby & p->collType)) {
        xent_entent = 0;
        return ent;
    }

    if (ent->id == p->id && (ent == p || ent->baseType != eBaseTypeBoulder)) {
        xent_entent = 0;
        return ent;
    }

    coll = &p->collis->colls[p->collis->idx];
    if (ent->collLev == 5 &&
        (p->collType & (k_XENT_COLLTYPE_NPC | k_XENT_COLLTYPE_PC))) {
        modl_coll = 1;
    }
    
    if (modl_coll) {
        coll->flags = 0;
    } else {
        coll->flags = k_HIT_CALC_HDNG | k_HIT_0xF00;
    }

    xBoundHitsBound(&p->bound, &ent->bound, coll);

    if (coll->flags & k_HIT_IT) {
        xBound tmp;
        xBound* bptr = &p->bound;
        if (modl_coll) {
            coll->flags = k_HIT_CALC_HDNG | k_HIT_0xF00;

            U8 ncolls;
            if (p->bound.type == k_XBOUNDTYPE_SPHERE) {
                xModelInstance* collModel = ent->collModel ? ent->collModel : ent->model;
                ncolls = iSphereHitsModel3(&p->bound.sph, collModel, coll, 15 - p->collis->idx, xdeg2rad(45.0f));
            } else if (p->bound.type == k_XBOUNDTYPE_BOX) {
                xVec3* upper = &p->bound.box.box.upper;
                xVec3* lower = &p->bound.box.box.lower;
                tmp.type = k_XBOUNDTYPE_SPHERE;
                xVec3Add(&tmp.sph.center, upper, lower);
                xVec3SMulBy(&tmp.sph.center, 0.5f);
                tmp.sph.r = (upper->x + upper->y + upper->z - lower->x - lower->y - lower->z) * 0.167f;
                xModelInstance* collModel = ent->collModel ? ent->collModel : ent->model;
                ncolls = iSphereHitsModel3(&p->bound.sph, collModel, coll, 15 - p->collis->idx, xdeg2rad(45.0f));
            }

            for (U8 idx = 0; idx < ncolls; idx++) {
                coll[idx].optr = ent;
                coll[idx].mptr = ent->model;
                p->collis->idx++;
            }

            xent_entent = 0;
            return ent;
        }

        coll->oid = ent->id;
        coll->optr = ent;
        coll->mptr = ent->model;
        p->collis->idx++;

        if ((ent->pflags & 0x20) &&
            ent->bound.type == k_XBOUNDTYPE_SPHERE &&
            p->bound.type == k_XBOUNDTYPE_SPHERE &&
            coll->hdng.y < -0.86602497f) {
            F32 rsum = bptr->sph.r + ent->bound.sph.r;
            F32 dx = bptr->sph.center.x - ent->bound.sph.center.x;
            F32 dy = bptr->sph.center.y - ent->bound.sph.center.y;
            F32 dz = bptr->sph.center.z - ent->bound.sph.center.z;
            F32 hsqr = xsqr(rsum) - (xsqr(dx) + xsqr(dz));
            if (hsqr >= 0.0f) {
                coll->depen.x = 0.0f;
                coll->depen.y = xsqrt(hsqr) - dy;
                coll->depen.z = 0.0f;
                coll->dist = 0.7f - coll->depen.y;
                coll->hdng.x = 0.0f;
                coll->hdng.y = -1.0f;
                coll->hdng.z = 0.0f;
            }
        }
    }

    xent_entent = 0;
    
    return ent;
}

void xEntCollideFloor(xEnt* p, xScene* sc, F32 dt)
{
    xCollis* coll = p->collis->colls;
    U8 idx;
    xCollis* ml = coll;
    xVec3 motion;
    F32 mlen;
    S32 stepping = 0;
    F32 sbr = (p->bound.type == k_XBOUNDTYPE_SPHERE) ? p->bound.sph.r : 0.7f;

    xVec3Copy(&motion, &p->frame->mat.pos);
    xVec3SubFrom(&motion, &p->frame->oldmat.pos);
    motion.y = 0.0f;

    mlen = xVec3Length(&motion);
    
    for (idx = 6; idx < p->collis->idx; idx++) {
        xCollis* mf = &p->collis->colls[idx];
        if (!(mf->flags & k_HIT_IT)) continue;
        
        xEnt* fent = (xEnt*)mf->optr;
        if (fent) {
            if (fent->collType != k_XENT_COLLTYPE_DYN && fent->collType != k_XENT_COLLTYPE_STAT &&
                (!(fent->pflags & 0x20) || mf->hdng.x != 0.0f || mf->hdng.z != 0.0f)) {
                continue;
            }

            if (!((p->collis->depenq) ?
                p->collis->depenq(p, fent, sc, dt, mf) :
                ((fent->collType & p->collis->pen) && (fent->penby & p->collType)))) {
                continue;
            }
        } else if (!(p->collis->pen & 0x20)) {
            continue;
        }

        if (mf->dist < ml->dist) {
            if (mf->hdng.y < -icos(xdeg2rad(60.0f)) &&
                (mf->norm.y > icos(nsn_angle) || p->frame->oldmat.pos.y > sc->gravity * dt * dt + p->frame->mat.pos.y)) {
                ml = mf;
                stepping = 0;
            } else if (mlen > 0.001f &&
                       mf->hdng.y < 0.65f / sbr - 1.0f &&
                       mf->norm.y > icos(nsn_angle)) {
                stepping = 1;
                ml = mf;
            }
        }
    }

    if (ml != coll) {
        F32 flr_dist = ml->dist * xabs(ml->hdng.y);
        *coll = *ml;
        if (flr_dist < sbr) {
            ml->flags |= (k_HIT_0x4 | k_HIT_0x2);
            if (stepping) {
                p->frame->mat.pos.y += 1.5f * dt;
                p->frame->mat.pos.x += ml->depen.x;
                p->frame->mat.pos.z += ml->depen.z;
            } else {
                p->frame->mat.pos.y += ml->depen.y;
            }
            p->frame->vel.y = 0.0f;
        }
    }
}

void xEntCollideCeiling(xEnt* p, xScene* sc, F32 dt) NONMATCH("https://decomp.me/scratch/mSbaz")
{
    xCollis* coll = p->collis->colls + 1;
    U8 idx;
    xCollis* ml = coll;
    F32 sbr = (p->bound.type == k_XBOUNDTYPE_SPHERE) ? p->bound.sph.r : 0.7f;

    for (idx = 6; idx < p->collis->idx; idx++) {
        xCollis* mf = &p->collis->colls[idx];
        if (mf->optr) {
            xEnt* fent = (xEnt*)mf->optr;
            if (!(p->collis->depenq ?
                p->collis->depenq(p, fent, sc, dt, mf) :
                ((fent->collType & p->collis->pen) && (fent->penby & p->collType)))) {
                continue;
            }
        } else if (!(p->collis->pen & 0x20)) {
            continue;
        }

        if (mf->hdng.y > icos(xdeg2rad(45.0f)) &&
            mf->dist < ml->dist) {
            ml = mf;
        }
    }

    if (ml != coll) {
        F32 ceil_dist = ml->dist * xabs(ml->hdng.y);
        *coll = *ml;
        ml->flags |= (k_HIT_0x2 | k_HIT_0x8);
        if (ceil_dist < sbr) {
            p->frame->mat.pos.y -= sbr - ceil_dist;
            p->frame->vel.y = 0.0f;
        }
    }
}

void xEntCollideWalls(xEnt* p, xScene* sc, F32 dt) NONMATCH("https://decomp.me/scratch/zX4mt")
{
    xCollis* coll;
    xEnt* cent;
    U8 sidx, eidx, idx;
    F32 sbr = (p->bound.type == k_XBOUNDTYPE_SPHERE) ? p->bound.sph.r : 0.7f;

    if (p->collis->pen & k_XENT_COLLTYPE_NPC) {
        sidx = p->collis->npc_sidx;
        eidx = p->collis->npc_eidx;
        for (idx = sidx; idx < eidx; idx++) {
            coll = &p->collis->colls[idx];
            cent = (xEnt*)coll->optr;
            if (!(coll->flags & k_HIT_0x2) && coll->dist < sbr) {
                if (!(p->collis->depenq ?
                    p->collis->depenq(p, cent, sc, dt, coll) :
                    (cent->penby & p->collType))) {
                    continue;
                }
                if (coll->depen.x != 0.0f || coll->depen.z != 0.0f) {
                    coll->depen.y = 0.0f;
                }
                xEntAddToPos(p, &coll->depen);
            }
        }
    }

    if (p->collis->pen & k_XENT_COLLTYPE_DYN) {
        sidx = p->collis->dyn_sidx;
        eidx = p->collis->dyn_eidx;
        for (idx = sidx; idx < eidx; idx++) {
            coll = &p->collis->colls[idx];
            cent = (xEnt*)coll->optr;
            if (!(coll->flags & k_HIT_0x2) && coll->dist < sbr) {
                if (!(p->collis->depenq ?
                    p->collis->depenq(p, cent, sc, dt, coll) :
                    (cent->penby & p->collType))) {
                    continue;
                }
                coll->depen.y = 0.0f;
                xEntAddToPos(p, &coll->depen);
            }
        }
    }

    if (p->collis->pen & k_XENT_COLLTYPE_STAT) {
        sidx = p->collis->stat_sidx;
        eidx = p->collis->stat_eidx;
        for (idx = sidx; idx < eidx; idx++) {
            coll = &p->collis->colls[idx];
            cent = (xEnt*)coll->optr;
            if (!(coll->flags & k_HIT_0x2) && coll->dist < sbr) {
                if (!(p->collis->depenq ?
                    p->collis->depenq(p, cent, sc, dt, coll) :
                    (cent->penby & p->collType))) {
                    continue;
                }
                coll->depen.y = 0.0f;
                xEntAddToPos(p, &coll->depen);
            }
        }
    }

    if (p->collis->pen & k_XENT_COLLTYPE_ENV) {
        sidx = p->collis->env_sidx;
        eidx = p->collis->env_eidx;
        for (idx = sidx; idx < eidx; idx++) {
            coll = &p->collis->colls[idx];
            cent = (xEnt*)coll->optr;
            if (!(coll->flags & k_HIT_0x2) && coll->dist < sbr) {
                coll->depen.y = 0.0f;
                xEntAddToPos(p, &coll->depen);
            }
        }
    }
}

void xEntSetNostepNormAngle(F32 angle)
{
    nsn_angle = angle;
}

xBox* xEntGetAllEntsBox()
{
    return &all_ents_box;
}

void xEntAnimateCollision(xEnt& ent, bool on)
{
    if (on && !(ent.moreFlags & k_MORE_FLAGS_ANIM_COLL)) {
        ent.moreFlags |= k_MORE_FLAGS_ANIM_COLL;
        if (!ent.frame) {
            ent.frame = (xEntFrame*)xMALLOC(sizeof(xEntFrame));
            memset(ent.frame, 0, sizeof(xEntFrame));
        }
        anim_coll::reset(ent);
    } else if (!on && (ent.moreFlags & k_MORE_FLAGS_ANIM_COLL)) {
        ent.moreFlags &= (U8)~k_MORE_FLAGS_ANIM_COLL;
    }
}

bool xEntValidType(U8 type) NONMATCH("https://decomp.me/scratch/7QjDU")
{
    return type == eBaseTypeTrigger ||
        type == eBaseTypePlayer ||
        type == eBaseTypePickup ||
        type == eBaseTypePlatform ||
        type == eBaseTypeDoor ||
        type == eBaseTypeSavePoint ||
        type == eBaseTypeItem ||
        type == eBaseTypeStatic ||
        type == eBaseTypeDynamic ||
        type == eBaseTypeBubble ||
        type == eBaseTypePendulum ||
        type == eBaseTypeHangable ||
        type == eBaseTypeButton ||
        type == eBaseTypeProjectile ||
        type == eBaseTypeDestructObj ||
        type == eBaseTypeUI ||
        type == eBaseTypeUIFont ||
        type == eBaseTypeProjectileType ||
        type == eBaseTypeEGenerator ||
        type == eBaseTypeNPC ||
        type == eBaseTypeBoulder ||
        type == eBaseTypeTeleportBox ||
        type == eBaseTypeZipLine;
}

void xEntReposition(xEnt& ent, const xMat4x3& mat)
{
    *(xMat4x3*)ent.model->Mat = mat;
    if (ent.collModel && ent.collModel != ent.model) {
        *(xMat4x3*)ent.collModel->Mat = mat;
    }
    if (ent.frame) {
        ent.frame->mat = mat;
    }
    if (ent.bound.mat) {
        *ent.bound.mat = mat;
    }
    ent.bound.sph.center = mat.pos;
    xBoundUpdate(&ent.bound);
    zGridUpdateEnt(&ent);
}

void xEntInitShadow(xEnt& ent, xEntShadow& shadow) NONMATCH("https://decomp.me/scratch/TMlnV")
{
    ent.entShadow = &shadow;
    shadow.vec.assign(0.0f, 1.0f, 0.0f);
    shadow.pos = ent.asset->pos;
    shadow.shadowModel = NULL;
    shadow.dst_cast = -1.0f;
    shadow.radius[0] = -1.0f;
    shadow.radius[1] = -1.0f;
}
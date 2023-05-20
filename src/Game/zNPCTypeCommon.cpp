#include "zNPCTypeCommon.h"

#include "xString.h"
#include "xDebugTweak.h"
#include "xEntAsset.h"
#include "xstransvc.h"
#include "xEntBoulder.h"
#include "xLinkAsset.h"
#include "xDraw.h"
#include "xSFX.h"
#include "xSFXAsset.h"
#include "xutil.h"
#include "xSnd.h"
#include "zNPCSupport.h"
#include "zNPCFXCinematic.h"
#include "zNPCSndTable.h"
#include "zNPCSndLists.h"
#include "zNPCTypes.h"
#include "zBase.h"
#include "zEvent.h"
#include "zGrid.h"
#include "zShrapnelAsset.h"
#include "zEntButton.h"
#include "zCombo.h"
#include "zEntPickup.h"
#include "zRumble.h"
#include "zEntCruiseBubble.h"
#include "zCutsceneMgr.h"
#include "zEntTeleportBox.h"
#include "zAnimFx.h"
#include "zGame.h"

U32 g_hash_lassanim[LASS_ANIM_NOMORE] = {};

const char* g_strz_lassanim[LASS_ANIM_NOMORE] = {
    "Unknown",
    "LassoGuide_Grab01",
    "LassoGuide_Hold01",
};

static char* g_strz_params[NPC_PARM_NOMORE] = {
    "Empty",
    "MoveSpeed",
    "TurnSpeed",
    "FactorAccel",
    "FactorDrift",
    "FactorMass",
    "FactorGravKnock",
    "FactorElasticity",
    "BoundMainIsBox",
    "BoundMainCenter",
    "BoundMainExtent",
    "HitPoints",
    "ScaleModel",
    "DetectRadius",
    "DetectHeight",
    "DetectOffset",
    "AttackRadius",
    "AttackFOV",
    "SoundRadius",
    "DelayFidget",
    "AttackPeriod",
    "StunTime",
    "AlertTime",
    "VtxAttackBase",
    "VtxAttack",
    "VtxAttack1",
    "VtxAttack2",
    "VtxAttack3",
    "VtxAttack4",
    "VtxEyeball",
    "VtxDmgSmokeA",
    "VtxDmgSmokeB",
    "VtxDmgSmokeC",
    "VtxDmgFlameA",
    "VtxDmgFlameB",
    "VtxDmgFlameC",
    "VtxPropel",
    "VtxExhaust",
    "VtxGen01",
    "VtxGen02",
    "VtxGen03",
    "VtxGen04",
    "VtxGen05",
    "AttackSize01",
    "AttackFrames01",
    "AttackFrames01a",
    "AttackFrames01b",
    "AttackFrames02",
    "AttackFrames02a",
    "AttackFrames02b",
    "AttackFrames03",
    "AttackFrames03a",
    "AttackFrames03b",
    "EsteemSlotA",
    "EsteemSlotB",
    "EsteemSlotC",
    "EsteemSlotD",
    "EsteemSlotE",
    "DistShadowCast",
    "ShadowCacheRadius",
    "ShadowRasterRadius",
    "TestCount",
    "EndTag_INIOnly",
    "FirstMovepoint",
    "EndTag_PropsOnly",
    "Bogus_Share",
    "EndTag_Shared",
};

static en_npcparm mdlVertToParm[NPC_MDLVERT_NOMORE] = {
    NPC_PARM_VTX_ATTACKBASE,
    NPC_PARM_VTX_ATTACK,
    NPC_PARM_VTX_ATTACK1,
    NPC_PARM_VTX_ATTACK2,
    NPC_PARM_VTX_ATTACK3,
    NPC_PARM_VTX_ATTACK4,
    NPC_PARM_VTX_EYEBALL,
    NPC_PARM_VTX_DMGSMOKEA,
    NPC_PARM_VTX_DMGSMOKEB,
    NPC_PARM_VTX_DMGSMOKEC,
    NPC_PARM_VTX_DMGFLAMEA,
    NPC_PARM_VTX_DMGFLAMEB,
    NPC_PARM_VTX_DMGFLAMEC,
    NPC_PARM_VTX_PROPEL,
    NPC_PARM_VTX_EXHAUST,
    NPC_PARM_VTX_GEN01,
    NPC_PARM_VTX_GEN02,
    NPC_PARM_VTX_GEN03,
    NPC_PARM_VTX_GEN04,
    NPC_PARM_VTX_GEN05,
};

static NPCConfig* g_ncfghead;
static zNPCSettings* g_dflt_npcsettings;
static S32 g_skipDescent;
static F32 g_tmr_talkless = 10.0f;
static S32 g_flg_wonder;
static S32 g_isConversation;
static xBase* g_ownerConversation;

static void zNPCPlyrSnd_Reset();
static void zNPCPlyrSnd_Update(F32);

xFactoryInst* ZNPC_Create_Common(S32 who, RyzMemGrow* grow, void*)
{
    zNPCCommon* com = NULL;

    switch (who) {
    case NPC_TYPE_COMMON:
        com = new (who, grow) zNPCCommon(who);
        break;
    }

    return com;
}

void ZNPC_Destroy_Common(xFactoryInst* inst)
{
    delete inst;
}

void ZNPC_Common_Startup()
{
    for (S32 i = 0; i < LASS_ANIM_NOMORE; i++) {
        g_hash_lassanim[i] = xStrHash(g_strz_lassanim[i]);
    }

    NPCSupport_Startup();
    NPCS_Startup();
    zNPCSettings_MakeDummy();
    zNPCFXStartup();
}

void ZNPC_Common_Shutdown()
{
    NPCS_Shutdown();
    NPCSupport_Shutdown();
    zNPCFXShutdown();
}

void zNPCCommon_ScenePrepare()
{
    NPCS_SndTimersReset();
    NPCS_SndTablePrepare(g_sndTrax_General);
    NPCSupport_ScenePrepare();
    zNPCCommon_WonderReset();

    g_skipDescent = 5;
}

void zNPCCommon_SceneFinish()
{
    zNPCCommon::ConfigSceneDone();
    NPCSupport_SceneFinish();
    xDebugRemoveTweak("NPC");
}

void zNPCCommon_SceneReset()
{
    NPCSupport_SceneReset();
    zNPCPlyrSnd_Reset();

    g_skipDescent = 5;
}

void zNPCCommon_ScenePostInit()
{
    NPCSupport_ScenePostInit();
}

void zNPCCommon_Timestep(xScene*, F32 dt) NONMATCH("https://decomp.me/scratch/KZZtd")
{
    NPCSupport_Timestep(dt);
    NPCS_SndTimersUpdate(dt);
    zNPCPlyrSnd_Update(dt);

    g_skipDescent--;
    if (g_skipDescent < 0) {
        g_skipDescent = 0;
    }
}

void zNPCCommon::Init(xEntAsset* entass)
{
    xSceneID2Name(globals.sceneCur, entass->id);

    xNPCBasic::Init(entass);

    this->entass = entass;
    this->npcass = (xEntNPCAsset*)(entass + 1);
    
    xLinkAsset* npclinx = (xLinkAsset*)(this->npcass + 1);
    if (linkCount) {
        this->link = npclinx;
    } else {
        this->link = NULL;
    }

    this->parmdata = zEntGetModelParams(this->entass->modelInfoID, &this->pdatsize);
    
    this->cfg_npc = this->ConfigFind(this->entass->modelInfoID);
    if (!this->cfg_npc) {
        this->cfg_npc = this->ConfigCreate(this->entass->modelInfoID);
        this->ParseINI();
    }

    if (this->cfg_npc && xVec3Length2(&this->cfg_npc->scl_model) > 0.0f) {
        this->flg_misc |= 0x4;
    }

    this->InitBounds();
}

void zNPCCommon::InitBounds() NONMATCH("https://decomp.me/scratch/JPhdS")
{
    NPCConfig* cfg = this->cfg_npc;
    xVec3 half = {};
    xSphere* sph = &this->bound.sph;
    xBBox* box = &this->bound.box;

    if (cfg->useBoxBound) {
        this->bound.type = k_XBOUNDTYPE_BOX;
    } else {
        this->bound.type = k_XBOUNDTYPE_SPHERE;
    }

    S32 r28;
    if (xVec3Length2(&cfg->off_bound) > 0.0f) {
        r28 = 1;
    } else {
        r28 = 0;
    }

    if (xVec3Length2(&cfg->dim_bound) > 0.0f) {
        xSceneID2Name(globals.sceneCur, this->id);
        this->DBG_Name();

        if (this->bound.type == k_XBOUNDTYPE_SPHERE) {
            sph->r = cfg->dim_bound.x;
            xVec3Copy(&sph->center, xEntGetPos(this));
            xVec3AddTo(&sph->center, &cfg->off_bound);
        } else {
            xVec3SMul(&half, &cfg->dim_bound, 0.5f);
            xVec3Copy(&box->center, xEntGetPos(this));
            xVec3AddTo(&box->center, &cfg->off_bound);
            xVec3Add(&box->box.upper, &box->center, &half);
            xVec3Sub(&box->box.lower, &box->center, &half);
        }
    } else {
        xSceneID2Name(globals.sceneCur, this->id);
        this->DBG_Name();

        switch (this->bound.type) {
        case k_XBOUNDTYPE_SPHERE:
            iSphereForModel(sph, this->model);
            cfg->dim_bound.x = sph->r;
            cfg->dim_bound.y = 0.0f;
            cfg->dim_bound.z = 0.0f;
            if (!r28) {
                xVec3Copy(&cfg->off_bound, &sph->center);
                cfg->off_bound.y = xmax(cfg->off_bound.y, sph->r);
            }
            sph->r = cfg->dim_bound.x;
            xVec3Copy(&sph->center, xEntGetPos(this));
            xVec3AddTo(&sph->center, &cfg->off_bound);
            break;
        case k_XBOUNDTYPE_BOX:
        case k_XBOUNDTYPE_OBB:
            iBoxForModel(&box->box, this->model);
            xVec3Sub(&cfg->dim_bound, &box->box.upper, &box->box.lower);
            if (!r28) {
                xVec3SMul(&cfg->off_bound, &cfg->dim_bound, 0.5f);
            }
            xVec3SMul(&half, &cfg->dim_bound, 0.5f);
            xVec3Copy(&box->center, xEntGetPos(this));
            xVec3AddTo(&box->center, &cfg->off_bound);
            xVec3Add(&box->box.upper, &box->center, &half);
            xVec3Sub(&box->box.lower, &box->center, &half);
            break;
        }
    }
}

void zNPCCommon::Setup()
{
    xSceneID2Name(globals.sceneCur, this->id);

    xNPCBasic::Setup();

    this->DBG_InstName();
    this->DBG_RptDataSize();

    this->npcsetass = zNPCSettings_Find(this->npcass->npcProps);

    this->ParseLinks();
    
    if (this->LassoInit()) {
        this->LassoSetup();
    }

    this->SelfSetup();
    this->DBG_AddTweakers();

    switch (this->SelfType()) {
    case NPC_TYPE_HAMMER:
    case NPC_TYPE_HAMSPIN:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("hammer_shrapnel"), NULL);
        break;
    case NPC_TYPE_TARTAR:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("tartar_shrapnel"), NULL);
        break;
    case NPC_TYPE_FODDER:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("fodder_shrapnel"), NULL);
        break;
    case NPC_TYPE_FODBZZT:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("robot_0a_bzzt_shrapnel"), NULL);
        break;
    case NPC_TYPE_CHOMPER:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("robot_0a_chomper_shrapnel"), NULL);
        break;
    case NPC_TYPE_GLOVE:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("g-love_shrapnel"), NULL);
        break;
    case NPC_TYPE_MONSOON:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("robot_4a_monsoon_shrapnel"), NULL);
        break;
    case NPC_TYPE_SLEEPY:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("robot_sleepy-time_shrapnel"), NULL);
        break;
    case NPC_TYPE_ARFARF:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("robot_arf_shrapnel"), NULL);
        break;
    case NPC_TYPE_CHUCK:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("robot_chuck_shrapnel"), NULL);
        break;
    case NPC_TYPE_SLICK:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("robot_9a_shrapnel"), NULL);
        break;
    case NPC_TYPE_DUPLOTRON:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("duplicatotron1000_shrapnel"), NULL);
        break;
    case NPC_TYPE_TIKI_WOOD:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("tiki_wooden_shrapnel"), NULL);
        break;
    case NPC_TYPE_TIKI_QUIET:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("tiki_shhhh_shrapnel"), NULL);
        break;
    case NPC_TYPE_TIKI_THUNDER:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("tiki_thunder_shrapnel"), NULL);
        break;
    case NPC_TYPE_TIKI_LOVEY:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("tiki_lovey_dovey_shrapnel"), NULL);
        break;
    case NPC_TYPE_TIKI_STONE:
        this->explosion = (zShrapnelAsset*)xSTFindAsset(xStrHash("tiki_stone_shrapnel"), NULL);
        break;
    default:
        this->explosion = NULL;
        break;
    }
    
    S32 wason = 0;
    xPsyche* psy = this->psy_instinct;

    if (psy) {
        if (psy->ImmTranIsOn()) {
            wason = 1;
        }
        psy->ImmTranOff();
    }

    this->Reset();

    if (psy && wason) {
        psy->ImmTranOn();
    }
}

void zNPCCommon::Reset() NONMATCH("https://decomp.me/scratch/cl84A")
{
    xSceneID2Name(globals.sceneCur, this->id);

    xNPCBasic::Reset();

    this->entShadow->dst_cast = this->cfg_npc->dst_castShadow;
    this->entShadow->radius[0] = this->cfg_npc->rad_shadowCache;
    this->entShadow->radius[1] = this->cfg_npc->rad_shadowRaster;

    this->ParseProps();

    this->npcset = *this->npcsetass;

    if (this->entass->flags & k_XENT_IS_VISIBLE) {
        xEntShow(this);
    } else {
        xEntHide(this);
    }

    if (this->flg_move & 0x2) {
        this->pflags |= k_XENT_HAS_GRAVITY;
    } else if (this->flg_move & 0x4) {
        this->pflags &= (U8)~k_XENT_HAS_GRAVITY;
    } else {
        this->pflags &= (U8)~k_XENT_HAS_GRAVITY;
    }

    if (this->model->Anim) {
        xAnimPlaySetState(this->model->Anim->Single, &this->model->Anim->Table->StateList[0], 0.0f);
    }

    this->drv_data = this->PRIV_GetDriverData();
    if (this->drv_data) {
        xEntDriveInit(this->drv_data, this);
        this->drv_data->flags |= 0x1;
    }

    if (this->lassdata) {
        this->lassdata->stage = LASS_STAT_PENDING;
    }
}

void zNPCCommon::Destroy()
{
    this->SelfDestroy();
}

void zNPCCommon::Damage(en_NPC_DAMAGE_TYPE damtype, xBase* who, const xVec3* vec_hit)
{
    static NPCMsg msg;
    NPCDamageInfo* dmg = &msg.dmgdata;
    
    if (!(this->flg_vuln & 0x1)) return;
    if (who && who->baseType == eBaseTypePlayer && !(this->flg_vuln & 0xFFFF0000)) return;

    switch (damtype) {
    case DMGTYP_ABOVE:
        if (!(this->flg_vuln & 0x40000)) return;
        break;
    case DMGTYP_BELOW:
        if (!(this->flg_vuln & 0x20000)) return;
        break;
    case DMGTYP_SIDE:
        if (gCurrentPlayer == eCurrentPlayerSpongeBob) {
            if (!(this->flg_vuln & 0x10000)) return;
        } else if (gCurrentPlayer == eCurrentPlayerPatrick) {
            if (!(this->flg_vuln & 0x80000000)) return;
        } else if (gCurrentPlayer == eCurrentPlayerSandy) {
            if (!(this->flg_vuln & 0x2000000)) return;
        } else {
            if (!(this->flg_vuln & 0x10000)) return;
        }
        break;
    case DMGTYP_INSTAKILL:
        this->tmr_invuln = -1.0f;
        break;
    case DMGTYP_HITBYTOSS:
        if (!(this->flg_vuln & 0x4)) return;
        break;
    case DMGTYP_NPCATTACK:
        if (!(this->flg_vuln & 0x8)) return;
        break;
    case DMGTYP_ROPE:
        if (!(this->flg_vuln & 0x1000000)) return;
        break;
    case DMGTYP_CRUISEBUBBLE:
        if (!(this->flg_vuln & 0x80000)) return;
        break;
    case DMGTYP_PROJECTILE:
        if (!(this->flg_vuln & 0x8)) return;
        break;
    case DMGTYP_BUBBOWL:
        if (!(this->flg_vuln & 0x100000)) return;
        break;
    case DMGTYP_BOULDER:
        if (!(this->flg_vuln & 0x200000)) return;
        break;
    case DMGTYP_THUNDER_TIKI_EXPLOSION:
        if (!(this->flg_vuln & 0x2)) return;
        break;
    case DMGTYP_DAMAGE_SURFACE:
    case DMGTYP_SURFACE:
        if (!(this->flg_vuln & 0x10)) return;
        break;
    case DMGTYP_BUNGEED:
        if (!(this->flg_vuln & 0x400000)) return;
        break;
    }

    if (this->tmr_invuln < 0.0f) {
        this->tmr_invuln = 0.5f;

        memset(&msg, 0, sizeof(msg));

        msg.from = this->id;
        msg.sendto = this->id;
        msg.msgid = NPC_MID_DAMAGE;
        msg.infotype = NPC_MDAT_DAMAGE;
        
        dmg->dmg_type = damtype;
        dmg->dmg_from = who;

        if (vec_hit) {
            xVec3Copy(&dmg->vec_dmghit, vec_hit);
        } else {
            xVec3Copy(&dmg->vec_dmghit, &g_O3);
        }

        zNPCMsg_SendMsg(&msg, this);
    }
}

S32 zNPCCommon::Respawn(const xVec3* pos, zMovePoint* mvptFirst, zMovePoint* mvptSpawnRef)
{
    static NPCMsg msg;

    memset(&msg, 0, sizeof(msg));

    msg.msgid = NPC_MID_RESPAWN;
    msg.from = this->id;
    msg.sendto = this->id;
    msg.infotype = NPC_MDAT_SPAWN;

    if (pos) {
        xVec3Copy(&msg.spawning.pos_spawn, pos);
    } else {
        xVec3Copy(&msg.spawning.pos_spawn, &this->entass->pos);
    }

    if (mvptFirst) {
        msg.spawning.nav_firstMovepoint = mvptFirst;
    } else {
        msg.spawning.nav_firstMovepoint = NULL;
    }

    msg.spawning.nav_spawnReference = mvptSpawnRef;
    msg.spawning.spawnSuccess = 0;

    zNPCMsg_SendMsg(&msg, this);

    return msg.spawning.spawnSuccess;
}

S32 zNPCCommon::NPCMessage(NPCMsg* mail)
{
    S32 handled = 1;

    switch (mail->msgid) {
    case NPC_MID_SYSEVENT:
    {
        xPsyche* psy = this->psy_instinct;
        
        switch (mail->sysevent.toEvent) {
        case eEventNPCPatrolOn:
            this->npcset.allowPatrol = 1;
            break;
        case eEventNPCPatrolOff:
            this->npcset.allowPatrol = 0;
            break;
        case eEventNPCWanderOn:
            this->npcset.allowDetect = 1;
            break;
        case eEventNPCWanderOff:
            this->npcset.allowDetect = 0;
            break;
        case eEventNPCDetectOn:
            this->npcset.allowDetect = 1;
            break;
        case eEventNPCDetectOff:
            this->npcset.allowDetect = 0;
            break;
        case eEventNPCChaseOn:
            this->npcset.allowChase = 1;
            break;
        case eEventNPCChaseOff:
            this->npcset.allowChase = 0;
            break;
        case eEventNPCSplineOKOn:
            this->npcset.useNavSplines = 1;
            break;
        case eEventNPCSplineOKOff:
            this->npcset.useNavSplines = 0;
            break;
        case eEventNPCSetActiveOff:
            if (psy && psy->HasGoal('NGN7')) {
                psy->GoalSet('NGN7', 1);
            }
            break;
        case eEventNPCSetActiveOn:
            if (psy) {
                psy->GIDOfPending();
                
                S32 r4 = psy->GIDOfSafety();
                if (r4) {
                    psy->GoalSet(r4, 1);
                }
            }
            break;
        default:
            handled = 0;
            break;
        }
        break;
    }
    case NPC_MID_RESPAWN:
    {
        xVec3Copy(xEntGetPos(this), &mail->spawning.pos_spawn);

        zMovePoint* mvpt = mail->spawning.nav_spawnReference;
        if (!mvpt) mvpt = mail->spawning.nav_firstMovepoint;

        this->nav_past = mvpt;
        this->nav_curr = mvpt;
        this->nav_dest = mvpt;
        this->nav_lead = mvpt;

        mail->spawning.spawnSuccess = 1;
        break;
    }
    case NPC_MID_DAMAGE:
        break;
    case NPC_MID_DEV_ANIMCYCLE:
        if (this->psy_instinct && this->psy_instinct->HasGoal('NGX1')) {
            this->psy_instinct->GoalSet('NGX1', 0);
        }
        break;
    case NPC_MID_DEV_ANIMSPIN:
        if (this->psy_instinct && this->psy_instinct->HasGoal('NGX0')) {
            this->psy_instinct->GoalSet('NGX0', 0);
        }
        break;
    case NPC_MID_DEV_HEROMODE:
        if (this->psy_instinct && this->psy_instinct->HasGoal('NGX2')) {
            this->psy_instinct->GoalSet('NGX2', 0);
        }
        break;
    case NPC_MID_DEV_DONE:
        break;
    default:
        handled = 1;
        break;
    }
    
    return handled;
}

void zNPCCommon::Move(xScene* xscn, F32 dt, xEntFrame* frm)
{
    if (this->drv_data && (this->drv_data->driver || this->drv_data->odriver)) {
        S32 backit = 0;
        xVec3 var_28;

        if (this->frame->mode & 0x2) {
            backit = 1;
            var_28 = this->frame->dpos;
        }

        xEntDriveUpdate(this->drv_data, xscn, dt, NULL);

        if (backit) {
            this->frame->mode |= 0x2;
            this->frame->dpos = var_28;
        }
    }

    xNPCBasic::Move(xscn, dt, frm);
}

void zNPCCommon::Process(xScene* xscn, F32 dt)
{
    if (this->flg_misc & 0x4) {
        this->ModelScaleSet(&this->cfg_npc->scl_model);
    }

    this->flg_upward &= ~0x2;

    xNPCBasic::Process(xscn, dt);
}

void zNPCCommon::BUpdate(xVec3* pos) NONMATCH("https://decomp.me/scratch/zpv2r")
{
    NPCConfig* cfg = this->cfg_npc;

    if (cfg->useBoxBound) {
        this->bound.type = k_XBOUNDTYPE_BOX;
        
        xBBox* box = &this->bound.box;
        xVec3 half = cfg->dim_bound * 0.5f;

        box->center = *pos + cfg->off_bound;
        box->box.upper = box->center + half;
        box->box.lower = box->center - half;
    } else {
        this->bound.type = k_XBOUNDTYPE_SPHERE;

        xSphere* sph = &this->bound.sph;

        sph->center = *pos + cfg->off_bound;
        sph->r = cfg->dim_bound.x;
    }

    if (this->bound.type != k_XBOUNDTYPE_NONE) {
        xQuickCullForBound(&this->bound.qcd, &this->bound);
    }

    zGridUpdateEnt(this);
}

F32 zNPCCommon::BoundAsRadius(S32 useCfg) const NONMATCH("https://decomp.me/scratch/rCFqE")
{
    F32 rad = 1.0f;

    if (useCfg) {
        NPCConfig* cfg = this->cfg_npc;

        if (cfg->useBoxBound) {
            xVec3 dim = cfg->dim_bound;
            rad = (dim.x + dim.y + dim.z) * (1/6.f);
        } else {
            rad = cfg->dim_bound.x;
        }
    } else {
        if (this->bound.type == k_XBOUNDTYPE_BOX) {
            const xBBox* box = &this->bound.box;
            const xVec3* le = &box->box.lower;
            const xVec3* ue = &box->box.upper;
            rad = ((ue->x + ue->y + ue->z) - (le->x + le->y - le->z)) * (1/6.f);
        } else if (this->bound.type == k_XBOUNDTYPE_SPHERE) {
            rad = this->bound.sph.r;
        }
    }

    return rad;
}

void zNPCCommon::NewTime(xScene* xscn, F32 dt)
{
    if (this->flg_misc & 0x2) {
        this->SndQueUpdate(dt);
    }

    this->tmr_invuln = xmax(-1.0f, this->tmr_invuln - dt);

    xNPCBasic::NewTime(xscn, dt);
}

S32 zNPCCommon::SysEvent(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget, S32* handled)
{
    static NPCMsg npcmsg;
    
    S32 doOtherEvents = 1;
    zNPCCommon* npc = (zNPCCommon*)to;
    xVec3 pos = {};
    xVec3 dir = {};

    *handled = 1;

    switch (toEvent) {
    case eEventSceneEnd:
    {
        xPsyche* psy = this->psy_instinct;
        if (psy) {
            psy->GoalNone(1, 1);
        }
        break;
    }
    case eEventNPCSpecial_PlatformSnap:
    case eEventNPCSpecial_PlatformFall:
    {
        memset(&npcmsg, 0, sizeof(npcmsg));

        npcmsg.msgid = NPC_MID_SYSEVENT;
        npcmsg.infotype = NPC_MDAT_SYSEVENT;
        npcmsg.from = this->id;
        npcmsg.sendto = this->id;

        NPCSysEvent* se = &npcmsg.sysevent;
        se->doLinkEvents = 1;
        se->handled = 0;
        se->to = to;
        se->from = from;
        se->toEvent = toEvent;
        se->toParamWidget = toParamWidget;

        if (!toParam) {
            se->toParam[0] = 0.0f;
            se->toParam[1] = 0.0f;
            se->toParam[2] = 0.0f;
            se->toParam[3] = 0.0f;
        } else {
            se->toParam[0] = toParam[0];
            se->toParam[1] = toParam[1];
            se->toParam[2] = toParam[2];
            se->toParam[3] = toParam[3];
        }

        zNPCMsg_SendMsg(&npcmsg, -1.0f, NULL);

        *handled = npcmsg.sysevent.handled;
        doOtherEvents = se->doLinkEvents;
        break;
    }
    case eEventHit:
        this->ConvertHitEvent(from, to, toEvent, toParam, toParamWidget, handled);
        break;
    case eEventReset:
        this->Reset();
        break;
    case eEventVisible:
        xEntShow(npc);
        break;
    case eEventInvisible:
        xEntHide(npc);
        break;
    case eEventSetUpdateDistance:
        if (globals.updateMgr) {
            if (toParam[0] <= 0.0f) {
                xUpdateCull_SetCB(globals.updateMgr, npc, xUpdateCull_AlwaysTrueCB, NULL);
            } else {
                FloatAndVoid dist;
                dist.f = xsqr(toParam[0]);
                xUpdateCull_SetCB(globals.updateMgr, npc, xUpdateCull_DistanceSquaredCB, dist.v);
            }
        }
        break;
    case eEventLaunchShrapnel:
    {
        zShrapnelAsset* shrap = (zShrapnelAsset*)toParamWidget;
        if (shrap && shrap->initCB) {
            xVec3 currVel;
            xVec3Sub(&currVel, &npc->frame->mat.pos, &npc->frame->oldmat.pos);
            xVec3SMulBy(&currVel, 1.0f / globals.update_dt);

            shrap->initCB(shrap, this->model, &currVel, NULL);
        }
        break;
    }
    case eEventNPCKillQuietly:
        this->Damage(DMGTYP_KILLEVENT, from, NULL);
        break;
    case eEventKill:
        this->Damage(DMGTYP_INSTAKILL, from, NULL);
        break;
    case eEventNPCRespawn:
        this->Respawn(NULL, NULL, NULL);
        break;
    case eEventNPCPatrolOn:
    case eEventNPCPatrolOff:
    case eEventNPCWanderOn:
    case eEventNPCWanderOff:
    case eEventNPCDetectOn:
    case eEventNPCDetectOff:
    case eEventNPCChaseOn:
    case eEventNPCChaseOff:
    case eEventNPCFightOn:
    case eEventNPCFightOff:
    {
        memset(&npcmsg, 0, sizeof(npcmsg));

        npcmsg.msgid = NPC_MID_SYSEVENT;
        npcmsg.infotype = NPC_MDAT_SYSEVENT;
        npcmsg.from = this->id;
        npcmsg.sendto = this->id;
        
        NPCSysEvent* se = &npcmsg.sysevent;
        se->doLinkEvents = 1;
        se->handled = 0;
        se->to = to;
        se->from = from;
        se->toEvent = toEvent;
        se->toParamWidget = toParamWidget;

        if (!toParam) {
            se->toParam[0] = 0.0f;
            se->toParam[1] = 0.0f;
            se->toParam[2] = 0.0f;
            se->toParam[3] = 0.0f;
        } else {
            se->toParam[0] = toParam[0];
            se->toParam[1] = toParam[1];
            se->toParam[2] = toParam[2];
            se->toParam[3] = toParam[3];
        }

        zNPCMsg_SendMsg(&npcmsg, -1.0f, NULL);

        *handled = npcmsg.sysevent.handled;
        doOtherEvents = se->doLinkEvents;
        break;
    }
    case eEventNPCForceConverseStart:
        if (NPCC_ForceTalkOk()) {
            memset(&npcmsg, 0, sizeof(npcmsg));
            
            npcmsg.msgid = NPC_MID_SYSEVENT;
            npcmsg.infotype = NPC_MDAT_SYSEVENT;
            npcmsg.from = this->id;
            npcmsg.sendto = this->id;

            NPCSysEvent* se = &npcmsg.sysevent;
            se->doLinkEvents = 1;
            se->handled = 0;
            se->to = to;
            se->from = from;
            se->toEvent = toEvent;
            se->toParamWidget = toParamWidget;

            if (!toParam) {
                se->toParam[0] = 0.0f;
                se->toParam[1] = 0.0f;
                se->toParam[2] = 0.0f;
                se->toParam[3] = 0.0f;
            } else {
                se->toParam[0] = toParam[0];
                se->toParam[1] = toParam[1];
                se->toParam[2] = toParam[2];
                se->toParam[3] = toParam[3];
            }

            zNPCMsg_SendMsg(&npcmsg, -1.0f, NULL);

            *handled = npcmsg.sysevent.handled;
            doOtherEvents = se->doLinkEvents;
        }

        break;
    case eEventNPCCheerForMe:
        zNPCMsg_SendMsg(NPC_MID_CELEBRATE, this);
        break;
    case eEventNPCGoToSleep:
    case eEventNPCWakeUp:
    {
        memset(&npcmsg, 0, sizeof(npcmsg));
        
        npcmsg.msgid = NPC_MID_SYSEVENT;
        npcmsg.infotype = NPC_MDAT_SYSEVENT;
        npcmsg.from = this->id;
        npcmsg.sendto = this->id;

        NPCSysEvent* se = &npcmsg.sysevent;
        se->doLinkEvents = 1;
        se->handled = 0;
        se->to = to;
        se->from = from;
        se->toEvent = toEvent;
        se->toParamWidget = toParamWidget;

        if (!toParam) {
            se->toParam[0] = 0.0f;
            se->toParam[1] = 0.0f;
            se->toParam[2] = 0.0f;
            se->toParam[3] = 0.0f;
        } else {
            se->toParam[0] = toParam[0];
            se->toParam[1] = toParam[1];
            se->toParam[2] = toParam[2];
            se->toParam[3] = toParam[3];
        }

        zNPCMsg_SendMsg(&npcmsg, -1.0f, NULL);

        *handled = npcmsg.sysevent.handled;
        doOtherEvents = se->doLinkEvents;
        break;
    }
    case eEventNPCSetActiveOn:
    case eEventNPCSetActiveOff:
        this->SelfType();

        if (this->psy_instinct) {
            memset(&npcmsg, 0, sizeof(npcmsg));
            
            npcmsg.msgid = NPC_MID_SYSEVENT;
            npcmsg.infotype = NPC_MDAT_SYSEVENT;
            npcmsg.from = this->id;
            npcmsg.sendto = this->id;
    
            NPCSysEvent* se = &npcmsg.sysevent;
            se->doLinkEvents = 1;
            se->handled = 0;
            se->to = to;
            se->from = from;
            se->toEvent = toEvent;
            se->toParamWidget = toParamWidget;
    
            if (!toParam) {
                se->toParam[0] = 0.0f;
                se->toParam[1] = 0.0f;
                se->toParam[2] = 0.0f;
                se->toParam[3] = 0.0f;
            } else {
                se->toParam[0] = toParam[0];
                se->toParam[1] = toParam[1];
                se->toParam[2] = toParam[2];
                se->toParam[3] = toParam[3];
            }
    
            zNPCMsg_SendMsg(&npcmsg, -1.0f, NULL);
    
            *handled = npcmsg.sysevent.handled;
            doOtherEvents = se->doLinkEvents;
        }

        break;
    case eEventDuploWaveBegin:
    case eEventDuploWaveComplete:
    case eEventDuploNPCBorn:
    case eEventDuploNPCKilled:
    case eEventDuploExpiredMaxNPC:
    case eEventDuploPause:
    case eEventDuploResume:
    case eEventDuploKillKids:
    {
        memset(&npcmsg, 0, sizeof(npcmsg));

        npcmsg.msgid = NPC_MID_SYSEVENT;
        npcmsg.infotype = NPC_MDAT_SYSEVENT;
        npcmsg.from = this->id;
        npcmsg.sendto = this->id;

        NPCSysEvent* se = &npcmsg.sysevent;
        se->doLinkEvents = 1;
        se->handled = 0;
        se->to = to;
        se->from = from;
        se->toEvent = toEvent;
        se->toParamWidget = toParamWidget;

        if (!toParam) {
            se->toParam[0] = 0.0f;
            se->toParam[1] = 0.0f;
            se->toParam[2] = 0.0f;
            se->toParam[3] = 0.0f;
        } else {
            se->toParam[0] = toParam[0];
            se->toParam[1] = toParam[1];
            se->toParam[2] = toParam[2];
            se->toParam[3] = toParam[3];
        }

        zNPCMsg_SendMsg(&npcmsg, -1.0f, NULL);

        *handled = npcmsg.sysevent.handled;
        doOtherEvents = se->doLinkEvents;
        break;
    }
    case eEventNPCScript_ScriptBegin:
        zNPCMsg_SendMsg(NPC_MID_SCRIPTBEGIN, this);
        break;
    case eEventNPCScript_ScriptEnd:
        zNPCMsg_SendMsg(NPC_MID_SCRIPTEND, this);
        break;
    case eEventNPCScript_SetPos:
        if (toParamWidget) {
            NPCC_pos_ofBase(toParamWidget, &pos);
            this->MatPosSet(&pos);
        }
        break;
    case eEventNPCScript_SetDir:
        if (toParamWidget) {
            NPCC_pos_ofBase(toParamWidget, &dir);
            xVec3SubFrom(&dir, xEntGetPos(this));
            dir.y = 0.0f;

            if (xVec3Length2(&dir) > 0.001f) {
                xVec3Normalize(&dir, &dir);
                xVec3Copy((xVec3*)&this->model->Mat->at, &dir);
                xVec3Copy((xVec3*)&this->model->Mat->up, &g_Y3);
                xVec3Cross((xVec3*)&this->model->Mat->right, &dir, &g_Y3);
            }
        }
        break;
    case eEventNPCScript_LookNormal:
    case eEventNPCScript_LookAlert:
    case eEventNPCScript_FaceWidget:
    case eEventNPCScript_GotoWidget:
    case eEventNPCScript_AttackWidget:
    case eEventNPCScript_FollowWidget:
    case eEventNPCScript_PlayAnim:
    case eEventNPCScript_LeadPlayer:
        zEventName(toEvent);
        break;
    case eEventUnknown:
    case eEventMount:
    case eEventDismount:
    case eEventDeath:
    case eEventSceneBegin:
    case eEventRoomBegin:
    case eEventRoomEnd:
    case eEventNPCScript_ScriptReady:
    case eEventNPCScript_Halt:
    case eEventNPCScript_FaceWidgetDone:
    case eEventNPCScript_GotoWidgetDone:
    case eEventNPCScript_AttackWidgetDone:
    case eEventNPCScript_PlayAnimDone:
        break;
    default:
        *handled = 0;
        doOtherEvents = xNPCBasic::SysEvent(from, to, toEvent, toParam, toParamWidget, handled);
        break;
    }

    return doOtherEvents;
}

void zNPCCommon::ConvertHitEvent(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget, S32* handled)
{
    xVec3 pos_cruiser = {};
    xVec3* vec_hit = NULL;
    F32 mag;
    en_NPC_DAMAGE_TYPE what = DMGTYP_UNDECIDED;

    if (from) {
        switch (from->baseType) {
        case eBaseTypePlayer:
        {
            U32 mvinf = zEntPlayer_MoveInfo();
            if (mvinf & 0x8) {
                what = DMGTYP_BELOW;
            } else if (mvinf & 0x10) {
                what = DMGTYP_ABOVE;
            } else if (mvinf & 0x20) {
                what = DMGTYP_SIDE;
            } else {
                what = DMGTYP_SIDE;
            }

            xVec3Sub(&pos_cruiser, xEntGetPos(&globals.player.ent), xEntGetPos(this));
            vec_hit = &pos_cruiser;
            break;
        }
        case eBaseTypeCruiseBubble:
            what = DMGTYP_CRUISEBUBBLE;

            if (toParam) {
                pos_cruiser.x = toParam[0];
                pos_cruiser.y = toParam[1];
                pos_cruiser.z = toParam[2];

                mag = xVec3Length(&pos_cruiser);
                if (mag > 0.001f) {
                    xVec3SubFrom(&pos_cruiser, xEntGetPos(this));
                    xVec3SMulBy(&pos_cruiser, 2.0f / mag);

                    vec_hit = &pos_cruiser;
                }
            }
            
            break;
        case eBaseTypeProjectile:
            what = DMGTYP_PROJECTILE;
            break;
        case eBaseTypeBoulder:
            if (globals.player.bubblebowl && globals.player.bubblebowl == from) {
                what = DMGTYP_BUBBOWL;
            } else {
                what = DMGTYP_BOULDER;
            }
            break;
        }
    }

    this->Damage(what, from, vec_hit);
}

void zNPCCommon::VelStop() NONMATCH("https://decomp.me/scratch/YizcX")
{
    this->spd_throttle = 0.0f;
    this->frame->dvel.x = 0.0f;
    this->frame->dvel.y = 0.0f;
    this->frame->dvel.z = 0.0f;
    this->frame->vel.x = 0.0f;
    this->frame->vel.y = 0.0f;
    this->frame->vel.z = 0.0f;
    this->frame->mode |= 0xC;

    if (!this->drv_data) {
        this->frame->dpos.x = 0.0f;
        this->frame->dpos.y = 0.0f;
        this->frame->dpos.z = 0.0f;
        this->frame->mode |= 0x2;
    } else if (!this->drv_data->driver) {
        this->frame->dpos.x = 0.0f;
        this->frame->dpos.y = 0.0f;
        this->frame->dpos.z = 0.0f;
        this->frame->mode |= 0x2;
    }
}

F32 zNPCCommon::ThrottleAdjust(F32 dt, F32 spd_want, F32 accel)
{
    NPCConfig* cfg = this->cfg_npc;
    F32 acc;
    S32 speedup = (spd_want > this->spd_throttle);

    acc = (accel < 0.0f) ? (dt * cfg->fac_accelMax) : (dt * accel);

    if (xeq(spd_want, this->spd_throttle, acc)) {
        this->spd_throttle = spd_want;
        return this->spd_throttle;
    }

    if (speedup) {
        this->spd_throttle += acc;
        this->spd_throttle = xclamp(this->spd_throttle, 0.0f, spd_want);
    } else {
        this->spd_throttle -= acc;
        this->spd_throttle = xmax(spd_want, this->spd_throttle);
    }

    return this->spd_throttle;
}

F32 zNPCCommon::ThrottleAccel(F32 dt, S32 speedup, F32 pct_max)
{
    NPCConfig* cfg = this->cfg_npc;
    F32 acc = dt * cfg->fac_accelMax;
    F32 spd_top = pct_max * cfg->spd_moveMax;

    if (speedup) {
        this->spd_throttle += acc;
    } else {
        this->spd_throttle -= acc;
    }

    this->spd_throttle = xclamp(this->spd_throttle, 0.0f, spd_top);

    return this->spd_throttle;
}

F32 zNPCCommon::ThrottleApply(F32 dt, const xVec3* dir, S32 force3D)
{
    xVec3 var_30;
    if (xVec3Length2(dir) < 0.00001f) {
        var_30 = *NPCC_faceDir(this) * this->spd_throttle;
    } else {
        var_30 = *dir * this->spd_throttle;
    }

    var_30 *= dt;

    if ((this->flg_move & 0x2) && !force3D) {
        this->frame->dpos.x = var_30.x;
        this->frame->dpos.z = var_30.z;
    } else {
        this->frame->dpos = var_30;
    }

    this->frame->mode |= 0x2;

    return this->spd_throttle;
}

F32 zNPCCommon::TurnToFace(F32 dt, const xVec3* dir_want, F32 useTurnRate) NONMATCH("https://decomp.me/scratch/KRonE")
{
    F32 f29 = (useTurnRate < 0.0f) ? (dt * this->cfg_npc->spd_turnMax) : (dt * useTurnRate);
    F32 f30 = NPCC_dir_toXZAng(dir_want);
    F32 f31 = NPCC_dir_toXZAng(NPCC_faceDir(this));
    
    F32 f28 = this->frame->rot.angle;
    if (this->frame->mode & 0x20) {
        f28 += this->frame->drot.angle;
    }

    F32 f1_ = xDangleClamp(f30 - f28);
    F32 f2_ = xclamp(f1_, -f29, f29);
    F32 f2 = xclamp(f28 + f2_, f31 - f29, f31 + f29);
    F32 f1 = f2 - f28;

    this->frame->drot.angle = f1;
    this->frame->mode |= 0x20;

    return f1;
    
    /*
    F32 ang_caller;
    F32 ang_wouldbe;
    F32 rot_lim;
    F32 ang_true;
    F32 ang_past;
    F32 ang_diff;
    */
}

void zNPCCommon::ParseLinks()
{
    for (S32 i = 0; i < this->linkCount; i++) {
        switch (this->link[i].srcEvent) {
        case eEventNPCScript_ScriptBegin:
        case eEventNPCScript_ScriptEnd:
        case eEventNPCScript_ScriptReady:
        case eEventNPCScript_Halt:
        case eEventNPCScript_SetPos:
        case eEventNPCScript_SetDir:
        case eEventNPCScript_LookNormal:
        case eEventNPCScript_LookAlert:
        case eEventNPCScript_FaceWidget:
        case eEventNPCScript_FaceWidgetDone:
        case eEventNPCScript_GotoWidget:
        case eEventNPCScript_GotoWidgetDone:
        case eEventNPCScript_AttackWidget:
        case eEventNPCScript_AttackWidgetDone:
        case eEventNPCScript_FollowWidget:
        case eEventNPCScript_PlayAnim:
        case eEventNPCScript_PlayAnimDone:
        case eEventNPCScript_LeadPlayer:
            xSceneID2Name(globals.sceneCur, this->id);
            this->flg_misc |= 0x1;
            break;
        }
    }
}

void zNPCCommon::ParseINI()
{
    NPCConfig* cfg = this->cfg_npc;

    this->TagVerts();

    for (S32 i = NPC_PARM_MOVERATE; i < NPC_PARM_ENDTAG_INI; i++) {
        en_npcparm pid = (en_npcparm)i;

        switch (pid) {
        case NPC_PARM_ACCEL:
            this->GetParm(pid, &cfg->fac_accelMax);
            break;
        case NPC_PARM_DRIFT:
            this->GetParm(pid, &cfg->fac_driftMax);
            break;
        case NPC_PARM_MASS:
            this->GetParm(pid, &cfg->npcMass);
            if (cfg->npcMass < 0.5f) {
                cfg->npcMass = 0.5f;
            }
            cfg->npcMassInv = 1.0f / cfg->npcMass;
            break;
        case NPC_PARM_TOSSELASTIC:
            this->GetParm(pid, &cfg->fac_elastic);
            break;
        case NPC_PARM_TOSSGRAV:
            this->GetParm(pid, &cfg->fac_gravKnock);
            break;
        case NPC_PARM_HITPOINTS:
            this->GetParm(pid, &cfg->pts_damage);
            break;
        case NPC_PARM_BND_ISBOX:
            this->GetParm(pid, &cfg->useBoxBound);
            break;
        case NPC_PARM_BND_CENTER:
            this->GetParm(pid, &cfg->off_bound);
            break;
        case NPC_PARM_BND_EXTENT:
            this->GetParm(pid, &cfg->dim_bound);
            break;
        case NPC_PARM_MOVERATE:
            this->GetParm(pid, &cfg->spd_moveMax);
            break;
        case NPC_PARM_TURNRATE:
        {
            F32 fv = xrad2deg(cfg->spd_turnMax);
            this->GetParm(pid, &fv);
            cfg->spd_turnMax = xdeg2rad(fv);
            break;
        }
        case NPC_PARM_MODELSCALE:
            this->GetParm(pid, &cfg->scl_model);
            break;
        case NPC_PARM_DETECT_RAD:
            this->GetParm(pid, &cfg->rad_detect);
            break;
        case NPC_PARM_DETECT_HYT:
            this->GetParm(pid, &cfg->hyt_detect);
            break;
        case NPC_PARM_DETECT_OFF:
            this->GetParm(pid, &cfg->off_detect);
            break;
        case NPC_PARM_TIMEFIDGET:
            this->GetParm(pid, &cfg->tym_fidget);
            break;
        case NPC_PARM_TIMEATTACK:
            this->GetParm(pid, &cfg->tym_attack);
            break;
        case NPC_PARM_TIMESTUN:
            this->GetParm(pid, &cfg->tym_stun);
            break;
        case NPC_PARM_TIMEALERT:
            this->GetParm(pid, &cfg->tym_alert);
            break;
        case NPC_PARM_ATTACK_RAD:
            this->GetParm(pid, &cfg->rad_attack);
            break;
        case NPC_PARM_ATTACK_FOV:
        {
            F32 fv = xrad2deg(cfg->fov_attack);
            this->GetParm(pid, &fv);
            cfg->fov_attack = xdeg2rad(fv);
            break;
        }
        case NPC_PARM_SND_RAD:
            this->GetParm(pid, &cfg->rad_sound);
            break;
        case NPC_PARM_ATK_SIZE01:
            this->GetParm(pid, &cfg->rad_dmgSize);
            break;
        case NPC_PARM_ATK_FRAMES01:
        case NPC_PARM_ATK_FRAMES01A:
        case NPC_PARM_ATK_FRAMES01B:
        case NPC_PARM_ATK_FRAMES02:
        case NPC_PARM_ATK_FRAMES02A:
        case NPC_PARM_ATK_FRAMES02B:
        case NPC_PARM_ATK_FRAMES03:
        case NPC_PARM_ATK_FRAMES03A:
        case NPC_PARM_ATK_FRAMES03B:
            this->GetParm(pid, &cfg->animFrameRange[pid - NPC_PARM_ATK_FRAMES01]);
            break;
        case NPC_PARM_ESTEEM_A:
        case NPC_PARM_ESTEEM_B:
        case NPC_PARM_ESTEEM_C:
        case NPC_PARM_ESTEEM_D:
        case NPC_PARM_ESTEEM_E:
            this->GetParm(pid, &cfg->cnt_esteem[pid - NPC_PARM_ESTEEM_A]);
            break;
        case NPC_PARM_SHADOW_CASTDIST:
            this->GetParm(pid, &cfg->dst_castShadow);
            break;
        case NPC_PARM_SHADOW_RADCACHE:
            this->GetParm(pid, &cfg->rad_shadowCache);
            break;
        case NPC_PARM_SHADOW_RADRASTER:
            this->GetParm(pid, &cfg->rad_shadowRaster);
            break;
        case NPC_PARAM_TEST_COUNT:
            this->GetParm(pid, &cfg->test_count);
            break;
        }
    }
}

void zNPCCommon::ParseProps()
{
    for (S32 i = NPC_PARM_FIRSTMVPT; i < NPC_PARM_ENDTAG_SHARE; i++) {
        en_npcparm pid = (en_npcparm)i;

        switch (pid) {
        case NPC_PARM_FIRSTMVPT:
            this->MvptReset(NULL);
            break;
        }
    }
}

void zNPCCommon::CollideReview()
{
    xEntCollis* npccol = this->collis;
    xCollis* colrec = &npccol->colls[0];
    S32 i;
    S32 hitthings = 0;

    if (!(colrec->flags & k_HIT_IT) && (this->flg_move & 0x2)) {
        this->colFreq = 0;
    }

    if (this->drv_data) {
        if ((colrec->flags & k_HIT_IT) && colrec->optr) {
            xEnt* flent = (xEnt*)colrec->optr;
            if (this->IsMountableType((en_ZBASETYPE)flent->baseType)) {
                if (!this->drv_data->driver) {
                    xEntDriveMount(this->drv_data, flent, 1.0f, colrec);
                } else if (flent != this->drv_data->driver) {
                    xEntDriveDismount(this->drv_data, 0.3f);
                    xEntDriveMount(this->drv_data, flent, 1.0f, colrec);
                }
            }
        } else if (this->drv_data->driver) {
            xEntDriveDismount(this->drv_data, 0.3f);
        }
    }

    for (i = npccol->env_sidx; i < npccol->env_eidx; i++) {
        if (!(npccol->colls[i].flags & k_HIT_0x4)) {
            hitthings++;
        }
    }

    for (i = npccol->stat_sidx; i < npccol->stat_eidx; i++) {
        if (!(npccol->colls[i].flags & k_HIT_0x4)) {
            hitthings++;
        }
    }

    for (i = npccol->dyn_sidx; i < npccol->dyn_eidx; i++) {
        if (npccol->colls[i].flags & k_HIT_0x4) {
            xBase* base = (xBase*)npccol->colls[i].optr;
            if (base->baseType == eBaseTypeButton) {
                zEntButton_Hold((zEntButton*)base, 0x800);
            }
        }
        hitthings++;
    }

    for (i = npccol->npc_sidx; i < npccol->npc_eidx; i++) {
        if (!(npccol->colls[i].flags & k_HIT_0x4)) {
            hitthings++;
        }
    }

    if (hitthings) {
        S32 cf = (S32)(8.0f * xurand() + 2.0);
        colFreq = xmin(colFreq, cf);
    }
}

S32 zNPCCommon::IsMountableType(en_ZBASETYPE type)
{
    switch (type) {
    case eBaseTypePlatform:
        return 1;
    }

    return 0;
}

void zNPCCommon::SelfDestroy()
{
    xBehaveMgr* bmgr = xBehaveMgr_GetSelf();

    if (this->psy_instinct) {
        bmgr->UnSubscribe(this->psy_instinct);
    }

    this->psy_instinct = NULL;
}

void zNPCCommon::TagVerts()
{
    NPCConfig* cfg = this->cfg_npc;
    S32 i;
    xVec3 tmp_pos = {};
    F32 vertinfo[4] = {};

    for (i = 0; i < 20; i++) {
        if (mdlVertToParm[i] != NPC_PARM_NONE) {
            this->GetParm(mdlVertToParm[i], vertinfo);

            tmp_pos.x = vertinfo[0];
            tmp_pos.y = vertinfo[1];
            tmp_pos.z = vertinfo[2];

            if (xVec3Dot(&tmp_pos, &tmp_pos) > 0.001f) {
                xModelInstance* mdl;
                S32 idx_atmoic = (S32)vertinfo[3];

                if (idx_atmoic < 1) {
                    mdl = this->model;
                } else {
                    mdl = this->ModelAtomicFind(idx_atmoic, -1, NULL);
                }

                if (mdl) {
                    iModelTagSetup(&cfg->tag_vert[i], mdl->Data, tmp_pos.x, tmp_pos.y, tmp_pos.z);

                    if (xVec3Length2(&tmp_pos)) {
                        cfg->flg_vert |= (1 << i);
                    }

                    xSceneID2Name(globals.sceneCur, this->id);
                }
            }
        }
    }
}

S32 zNPCCommon::GetVertPos(en_mdlvert vid, xVec3* pos)
{
    NPCConfig* cfg = this->cfg_npc;

    if (!(cfg->flg_vert & (1 << vid))) {
        return 0;
    }

    iModelTagEval(this->model->Data, &cfg->tag_vert[vid], this->model->Mat, pos);

    return 1;
}

S32 zNPCCommon::IsAttackFrame(F32 tym_anim, S32 series)
{
    S32 result = 0;
    NPCConfig* cfg = this->cfg_npc;

    if (!cfg) return 0;

    xVec3 var_30, var_3C, var_48;

    switch (series) {
    case 0:
    case 1:
        var_30 = cfg->animFrameRange[0];
        var_3C = cfg->animFrameRange[1];
        var_48 = cfg->animFrameRange[2];
        break;
    case 2:
        var_30 = cfg->animFrameRange[3];
        var_3C = cfg->animFrameRange[4];
        var_48 = cfg->animFrameRange[5];
        break;
    default:
        var_30 = cfg->animFrameRange[6];
        var_3C = cfg->animFrameRange[7];
        var_48 = cfg->animFrameRange[8];
        break;
    }

    F32 tym = (tym_anim < 0.0f) ? this->AnimTimeCurrent() : tym_anim;

    if (tym >= var_30.x && tym <= var_30.y) {
        result = 1;
    } else if (tym >= var_3C.x && tym <= var_3C.y) {
        result = 2;
    } else if (tym >= var_48.x && tym <= var_48.y) {
        result = 3;
    }

    return result;
}

void zNPCCommon::GiveReward()
{
    S32 i;
    U32 s;
    NPCConfig* cfg = this->cfg_npc;
    U32 shinies[5];
    S32 esteem;

    if (this->SelfType() != NPC_TYPE_ARFDOG) {
        if ((this->SelfType() & 0xFFFFFF00) == 'NTT\0') {
            zCombo_Add(1);
        } else {
            zCombo_Add(3);
        }
    }

    for (i = 0, s = 0; i < 5; i++) {
        esteem = cfg->cnt_esteem[i];
        if (esteem >= 1) {
            if (esteem == globals.player.g.ShinyValuePurple) {
                shinies[s] = 0;
                s++;
            } else if (esteem == globals.player.g.ShinyValueBlue) {
                shinies[s] = 1;
                s++;
            } else if (esteem == globals.player.g.ShinyValueGreen) {
                shinies[s] = 2;
                s++;
            } else if (esteem == globals.player.g.ShinyValueYellow) {
                shinies[s] = 3;
                s++;
            } else if (esteem == globals.player.g.ShinyValueRed) {
                shinies[s] = 4;
                s++;
            } else {
                shinies[i] = 4;
                s++;
                break;
            }
        }
    }

    if (s) {
        zEntPickup_SpawnNRewards(shinies, s, *this->Pos());
    }

    this->PlayerKiltMe();
}

static void zNPCPlyrSnd_Reset()
{
    g_tmr_talkless = 10.0f;
}

static void zNPCPlyrSnd_Update(F32 dt)
{
    g_tmr_talkless = xmax(-1.0f, g_tmr_talkless - dt);
}

void zNPCCommon::PlayerKiltMe()
{
    en_xEventTags r30 = eEventUnknown;
    S32 r31 = this->SelfType();

    if (!SomethingWonderful() && g_tmr_talkless < 0.0f) {
        g_tmr_talkless = 3.0f + (xurand() - 0.5f) * 0.25f * 3.0f;

        if ((r31 & 0xFFFFFF00) == 'NTR\0') {
            r30 = eEventSituationDestroyedRobot;
        } else if ((r31 & 0xFFFFFF00) == 'NTT\0') {
            r30 = eEventSituationDestroyedTiki;
        }

        if (r30 != eEventUnknown) {
            zEntEvent(this, &globals.player.ent, r30);
        }
    }
}

void zNPCCommon::ISeePlayer() NONMATCH("https://decomp.me/scratch/M08oY")
{
    en_xEventTags ven = eEventUnknown;

    if (!SomethingWonderful() && g_tmr_talkless < 0.0f) {
        g_tmr_talkless = 3.0f + (xurand() - 0.5f) * 0.25f * 3.0f;

        switch (this->SelfType()) {
        //case NPC_TYPE_UNKNOWN:
        //case NPC_TYPE_BASIC:
        //case NPC_TYPE_COMMON:
        case NPC_TYPE_AMBIENT:
        case NPC_TYPE_VILLAGER:
        case NPC_TYPE_ROBOT:
        case NPC_TYPE_BOSS:
        case NPC_TYPE_TEST:
        case NPC_TYPE_BADGUY:
        case NPC_TYPE_JELLYPINK:
        case NPC_TYPE_JELLYBLUE:
        case NPC_TYPE_KINGNEPTUNE:
        case NPC_TYPE_MIMEFISH:
        //case NPC_TYPE_COW:
            break;
        case NPC_TYPE_TIKI_WOOD:
            ven = eEventSituationSeeWoodTiki;
            break;
        case NPC_TYPE_TIKI_LOVEY:
            ven = eEventSituationSeeLoveyTiki;
            break;
        case NPC_TYPE_TIKI_QUIET:
            ven = eEventSituationSeeShhhTiki;
            break;
        case NPC_TYPE_TIKI_THUNDER:
            ven = eEventSituationSeeThunderTiki;
            break;
        case NPC_TYPE_TIKI_STONE:
            ven = eEventSituationSeeStoneTiki;
            break;
        case NPC_TYPE_FISH:
        //case NPC_TYPE_FISH_MALE:
        //case NPC_TYPE_FISH_FEMALE:
        //case NPC_TYPE_FISH_ELDER:
        //case NPC_TYPE_FISH_ELDESS:
        //case NPC_TYPE_FISH_BOY:
        //case NPC_TYPE_FISH_GIRL:
        //case NPC_TYPE_BALLOONBOY:
        //case NPC_TYPE_GARY:
        //case NPC_TYPE_SQUIDWARD:
        case NPC_TYPE_SQUIDWARD_MUSIC:
        case NPC_TYPE_SQUIDWARD_BANDAID:
        //case NPC_TYPE_DUTCHMAN_NSB:
        //case NPC_TYPE_SANDYBIKINI:
        //case NPC_TYPE_SANDYNPC:
        //case NPC_TYPE_PATNPC:
        //case NPC_TYPE_BOBNPC:
        //case NPC_TYPE_PLANKNPC:
        //case NPC_TYPE_MRKRABS:
        //case NPC_TYPE_MSPUFFS:
        //case NPC_TYPE_LARRY:
        //case NPC_TYPE_BUBBUDDY:
        //case NPC_TYPE_NEWSFISH:
        case NPC_TYPE_NEWSFISHTV:
        //case NPC_TYPE_MOTORIST:
        //case NPC_TYPE_MERMANCHAIR:
        //case NPC_TYPE_MERMAN:
        case NPC_TYPE_BARNACLEBOY:
        //case NPC_TYPE_WORM:
            break;
        case NPC_TYPE_HAMMER:
        case NPC_TYPE_HAMSPIN:
            ven = eEventSituationSeeHammer;
            break;
        case NPC_TYPE_TARTAR:
            ven = eEventSituationSeeTarTar;
            break;
        case NPC_TYPE_GLOVE:
            ven = eEventSituationSeeGLove;
            break;
        case NPC_TYPE_MONSOON:
            ven = eEventSituationSeeMonsoon;
            break;
        case NPC_TYPE_SLEEPY:
            ven = eEventSituationSeeSleepyTime;
            break;
        case NPC_TYPE_ARFDOG:
            break;
        case NPC_TYPE_ARFARF:
            ven = eEventSituationSeeArf;
            break;
        case NPC_TYPE_CHUCK:
            break;
        case NPC_TYPE_TUBELET:
        case NPC_TYPE_TUBESLAVE:
            ven = eEventSituationSeeTubelets;
            break;
        case NPC_TYPE_SLICK:
            ven = eEventSituationSeeSlick;
            break;
        case NPC_TYPE_FODDER:
        case NPC_TYPE_FODBOMB:
        case NPC_TYPE_FODBZZT:
        case NPC_TYPE_CHOMPER:
        case NPC_TYPE_CRITTER:
            ven = eEventSituationSeeFodder;
            break;
        case NPC_TYPE_DUPLOTRON:
            break;
        case NPC_TYPE_KINGJELLY:
            ven = eEventSituationSeeKingJellyfish;
            break;
        case NPC_TYPE_DUTCHMAN:
            ven = eEventSituationSeeDutchman;
            break;
        case NPC_TYPE_PRAWN:
            ven = eEventSituationSeePrawn;
            break;
        case NPC_TYPE_BOSSSANDY:
            ven = eEventSituationSeeSandyBoss;
            break;
        case NPC_TYPE_BOSSPAT:
            ven = eEventSituationSeePatrickBoss;
            break;
        case NPC_TYPE_BOSS_SB1:
        case NPC_TYPE_BOSSBOBBY:
            ven = eEventSituationSeeSpongeBobBoss;
            break;
        case NPC_TYPE_BOSSPLANKTON:
            ven = eEventSituationSeeRobotPlankton;
            break;
        //case NPC_TYPE_BADGUYMEDIUM:
        //    break;
        }

        if (ven != eEventUnknown) {
            zEntEvent(this, &globals.player.ent, ven);
        }
    }
}

void zNPCCommon::ConfigSceneDone()
{
    g_ncfghead = NULL;
}

NPCConfig* zNPCCommon::ConfigCreate(U32 modelID)
{
    NPCConfig* cfg = (NPCConfig*)xMALLOC(sizeof(NPCConfig));
    memset(cfg, 0, sizeof(NPCConfig));

    cfg->modelID = modelID;

    if (!g_ncfghead) {
        g_ncfghead = cfg;
    } else {
        cfg->Insert(g_ncfghead);
    }

    return cfg;
}

NPCConfig* zNPCCommon::ConfigFind(U32 modelID)
{
    NPCConfig* cfg = NULL;

    for (NPCConfig* r3 = g_ncfghead; r3 != NULL; r3 = r3->Next()) {
        if (r3->modelID == modelID) {
            cfg = r3;
            break;
        }
    }

    return cfg;
}

void zNPCCommon::GetParm(en_npcparm pid, S32* val)
{
    this->GetParm(pid, (void*)val);
}

void zNPCCommon::GetParm(en_npcparm pid, F32* val)
{
    this->GetParm(pid, (void*)val);
}

void zNPCCommon::GetParm(en_npcparm pid, xVec3* val)
{
    this->GetParm(pid, (void*)val);
}

void zNPCCommon::GetParm(en_npcparm pid, zMovePoint** val)
{
    this->GetParm(pid, (void*)val);
}

void zNPCCommon::GetParm(en_npcparm pid, void* val) NONMATCH("https://decomp.me/scratch/dg7eV")
{
    char** names = g_strz_params;
    xModelAssetParam* pmdata = this->parmdata;
    U32 pmsize = this->pdatsize;

    this->GetParmDefault(pid, val);

    switch (pid) {
    case NPC_PARM_BND_ISBOX:
    case NPC_PARM_HITPOINTS:
    case NPC_PARM_ESTEEM_A:
    case NPC_PARM_ESTEEM_B:
    case NPC_PARM_ESTEEM_C:
    case NPC_PARM_ESTEEM_D:
    case NPC_PARM_ESTEEM_E:
    case NPC_PARAM_TEST_COUNT:
        if (pmdata && pmsize) {
            *(S32*)val = zParamGetInt(pmdata, pmsize, names[pid], *(S32*)val);
        }
        break;
    case NPC_PARM_MOVERATE:
    case NPC_PARM_TURNRATE:
    case NPC_PARM_ACCEL:
    case NPC_PARM_DRIFT:
    case NPC_PARM_MASS:
    case NPC_PARM_TOSSGRAV:
    case NPC_PARM_TOSSELASTIC:
    case NPC_PARM_DETECT_RAD:
    case NPC_PARM_DETECT_HYT:
    case NPC_PARM_DETECT_OFF:
    case NPC_PARM_ATTACK_RAD:
    case NPC_PARM_ATTACK_FOV:
    case NPC_PARM_SND_RAD:
    case NPC_PARM_TIMEFIDGET:
    case NPC_PARM_TIMEATTACK:
    case NPC_PARM_TIMESTUN:
    case NPC_PARM_TIMEALERT:
    case NPC_PARM_ATK_SIZE01:
    case NPC_PARM_SHADOW_CASTDIST:
    case NPC_PARM_SHADOW_RADCACHE:
    case NPC_PARM_SHADOW_RADRASTER:
        if (pmdata && pmsize) {
            *(F32*)val = zParamGetFloat(pmdata, pmsize, names[pid], *(F32*)val);
        }
        break;
    case NPC_PARM_BND_CENTER:
    case NPC_PARM_BND_EXTENT:
    case NPC_PARM_MODELSCALE:
    case NPC_PARM_ATK_FRAMES01:
    case NPC_PARM_ATK_FRAMES01A:
    case NPC_PARM_ATK_FRAMES01B:
    case NPC_PARM_ATK_FRAMES02:
    case NPC_PARM_ATK_FRAMES02A:
    case NPC_PARM_ATK_FRAMES02B:
    case NPC_PARM_ATK_FRAMES03:
    case NPC_PARM_ATK_FRAMES03A:
    case NPC_PARM_ATK_FRAMES03B:
        if (pmdata && pmsize) {
            zParamGetVector(pmdata, pmsize, names[pid], *(xVec3*)val, (xVec3*)val);
        }
        break;
    case NPC_PARM_VTX_ATTACKBASE:
    case NPC_PARM_VTX_ATTACK:
    case NPC_PARM_VTX_ATTACK1:
    case NPC_PARM_VTX_ATTACK2:
    case NPC_PARM_VTX_ATTACK3:
    case NPC_PARM_VTX_ATTACK4:
    case NPC_PARM_VTX_EYEBALL:
    case NPC_PARM_VTX_DMGSMOKEA:
    case NPC_PARM_VTX_DMGSMOKEB:
    case NPC_PARM_VTX_DMGSMOKEC:
    case NPC_PARM_VTX_DMGFLAMEA:
    case NPC_PARM_VTX_DMGFLAMEB:
    case NPC_PARM_VTX_DMGFLAMEC:
    case NPC_PARM_VTX_PROPEL:
    case NPC_PARM_VTX_EXHAUST:
    case NPC_PARM_VTX_GEN01:
    case NPC_PARM_VTX_GEN02:
    case NPC_PARM_VTX_GEN03:
    case NPC_PARM_VTX_GEN04:
    case NPC_PARM_VTX_GEN05:
        zParamGetFloatList(pmdata, pmsize, names[pid], 4, NULL, (F32*)val);
        break;
    case NPC_PARM_FIRSTMVPT:
        if (this->npcass->movepoint) {
            zMovePoint* mvpt = zMovePoint_From_xAssetID(this->npcass->movepoint);
            if (mvpt) {
                *(zMovePoint**)val = mvpt;
            }
        }
        break;
    case NPC_PARM_BOGUSSHARE:
        if (pmdata && pmsize) {
            *(S32*)val = zParamGetInt(pmdata, pmsize, names[pid], *(S32*)val);
        }
        break;
    case NPC_PARM_NONE:
    case NPC_PARM_ENDTAG_INI:
    case NPC_PARM_ENDTAG_PROPS:
    case NPC_PARM_ENDTAG_SHARE:
        break;
    }
}

S32 zNPCCommon::GetParmDefault(en_npcparm pid, void* val) NONMATCH("https://decomp.me/scratch/mNHLS")
{
    S32 result = 1;
    S32* ivp = (S32*)val;
    F32* fvp = (F32*)val;
    F32* fvlist = (F32*)val;
    xVec3* vec = (xVec3*)val;
    zMovePoint** mvpt = (zMovePoint**)val;

    switch (pid) {
    case NPC_PARM_HITPOINTS:
        *ivp = 1;
        break;
    case NPC_PARM_MOVERATE:
        *fvp = 2.0f;
        break;
    case NPC_PARM_TURNRATE:
        *fvp = 90.0f;
        break;
    case NPC_PARM_ACCEL:
        *fvp = 1.0f;
        break;
    case NPC_PARM_DRIFT:
        *fvp = 0.0f;
        break;
    case NPC_PARM_MASS:
        *fvp = 1.0f;
        break;
    case NPC_PARM_TOSSGRAV:
        *fvp = 25.0f;
        break;
    case NPC_PARM_TOSSELASTIC:
        *fvp = 0.5f;
        break;
    case NPC_PARM_BND_ISBOX:
        *ivp = 0;
        break;
    case NPC_PARM_BND_CENTER:
        xVec3Copy(vec, &g_O3);
        break;
    case NPC_PARM_BND_EXTENT:
        xVec3Copy(vec, &g_O3);
        break;
    case NPC_PARM_DETECT_RAD:
        *fvp = 3.0f;
        break;
    case NPC_PARM_DETECT_HYT:
        *fvp = 6.0f;
        break;
    case NPC_PARM_DETECT_OFF:
        *fvp = -2.0f;
        break;
    case NPC_PARM_ATTACK_RAD:
        *fvp = 5.0f;
        break;
    case NPC_PARM_ATTACK_FOV:
        *fvp = 60.0f;
        break;
    case NPC_PARM_SND_RAD:
        *fvp = 30.0f;
        break;
    case NPC_PARM_ESTEEM_A:
    case NPC_PARM_ESTEEM_B:
    case NPC_PARM_ESTEEM_C:
    case NPC_PARM_ESTEEM_D:
    case NPC_PARM_ESTEEM_E:
        *ivp = 0;
        break;
    case NPC_PARM_SHADOW_CASTDIST:
    case NPC_PARM_SHADOW_RADCACHE:
    case NPC_PARM_SHADOW_RADRASTER:
        *fvp = -1.0f;
        break;
    case NPC_PARM_ATK_SIZE01:
        *fvp = 0.5f;
        break;
    case NPC_PARM_ATK_FRAMES01:
        vec->x = -1.0f;
        vec->y = 100.0f;
        vec->z = 0.0f;
        break;
    case NPC_PARM_ATK_FRAMES01A:
    case NPC_PARM_ATK_FRAMES01B:
    case NPC_PARM_ATK_FRAMES02:
    case NPC_PARM_ATK_FRAMES02A:
    case NPC_PARM_ATK_FRAMES02B:
    case NPC_PARM_ATK_FRAMES03:
    case NPC_PARM_ATK_FRAMES03A:
    case NPC_PARM_ATK_FRAMES03B:
        vec->x = -1.0f;
        vec->y = -2.0f;
        vec->z = 0.0f;
        break;
    case NPC_PARM_MODELSCALE:
        xVec3Copy(vec, &g_O3);
        break;
    case NPC_PARM_TIMEFIDGET:
        *fvp = 15.0f;
        break;
    case NPC_PARM_TIMEATTACK:
        *fvp = 15.0f;
        break;
    case NPC_PARM_TIMESTUN:
        *fvp = 5.0f;
        break;
    case NPC_PARM_TIMEALERT:
        *fvp = 5.0f;
        break;
    case NPC_PARAM_TEST_COUNT:
        *ivp = 1;
        break;
    case NPC_PARM_VTX_ATTACKBASE:
    case NPC_PARM_VTX_ATTACK:
    case NPC_PARM_VTX_ATTACK1:
    case NPC_PARM_VTX_ATTACK2:
    case NPC_PARM_VTX_ATTACK3:
    case NPC_PARM_VTX_ATTACK4:
    case NPC_PARM_VTX_EYEBALL:
    case NPC_PARM_VTX_DMGSMOKEA:
    case NPC_PARM_VTX_DMGSMOKEB:
    case NPC_PARM_VTX_DMGSMOKEC:
    case NPC_PARM_VTX_DMGFLAMEA:
    case NPC_PARM_VTX_DMGFLAMEB:
    case NPC_PARM_VTX_DMGFLAMEC:
    case NPC_PARM_VTX_PROPEL:
    case NPC_PARM_VTX_EXHAUST:
    case NPC_PARM_VTX_GEN01:
    case NPC_PARM_VTX_GEN02:
    case NPC_PARM_VTX_GEN03:
    case NPC_PARM_VTX_GEN04:
    case NPC_PARM_VTX_GEN05:
        fvlist[0] = 0.0f;
        fvlist[1] = 0.0f;
        fvlist[2] = 0.0f;
        fvlist[3] = 0.0f;
        break;
    case NPC_PARM_FIRSTMVPT:
        *mvpt = NULL;
        break;
    case NPC_PARM_ENDTAG_INI:
    case NPC_PARM_ENDTAG_PROPS:
    case NPC_PARM_ENDTAG_SHARE:
        result = 0;
        break;
    default:
        result = 0;
        break;
    }

    return result;
}

S32 zNPCCommon::CanDoSplines()
{
    return (this->npcset.useNavSplines && (flg_move & 0x8));
}

zMovePoint* zNPCCommon::FirstAssigned()
{
    zMovePoint* nav_first = NULL;
    this->GetParm(NPC_PARM_FIRSTMVPT, &nav_first);
    return nav_first;
}

void zNPCCommon::MvptReset(zMovePoint* nav_goto) NONMATCH("https://decomp.me/scratch/lhUW9")
{
    if (nav_goto) {
        this->nav_dest = nav_goto;
    } else {
        this->GetParm(NPC_PARM_FIRSTMVPT, &this->nav_dest);
    }

    this->nav_past = this->nav_dest;
    this->nav_curr = this->nav_dest;
    this->nav_lead = this->nav_dest;
    this->spl_mvptspline = NULL;
    this->len_mvptspline = 0.0f;
    this->dst_curspline = 0.0f;
}

S32 zNPCCommon::MvptCycle() NONMATCH("https://decomp.me/scratch/CGooj")
{
    zMovePoint* nav_tmp = NULL;
    
    this->spl_mvptspline = NULL;
    this->len_mvptspline = 0.0f;
    this->dst_curspline = 0.0f;

    if (!this->nav_curr) {
        this->GetParm(NPC_PARM_FIRSTMVPT, &this->nav_curr);
        nav_tmp = this->nav_curr;
    } else if (this->nav_curr && this->nav_dest) {
        zMovePointGetNext(this->nav_dest, this->nav_curr, &nav_tmp, NULL);
    } else if (this->nav_curr) {
        zMovePointGetNext(this->nav_curr, NULL, &nav_tmp, NULL);
    }

    if (this->nav_curr) {
        this->nav_past = this->nav_curr;
    }

    if (this->nav_dest) {
        this->nav_curr = this->nav_dest;
    }

    this->nav_dest = nav_tmp;

    if (this->CanDoSplines() && this->nav_dest && this->nav_dest->HasSpline()) {
        this->spl_mvptspline = this->nav_dest->spl;

        while (this->nav_dest->asset->bezIndex != 0) {
            this->nav_dest = (zMovePoint*)this->nav_dest->nodes[0];
        }

        this->len_mvptspline = xSpline3_ArcTotal(this->spl_mvptspline);
    }

    if (this->nav_dest) {
        zMovePointGetNext(this->nav_dest, this->nav_curr, &this->nav_lead, NULL);
    }

    return (this->nav_dest != NULL);
}

S32 zNPCCommon::HaveLOSToPos(xVec3* pos, F32 dist, xScene* xscn, xBase* tgt, xCollis* colCallers) NONMATCH("https://decomp.me/scratch/wxm2S")
{
    S32 result;
    xRay3 ray = {};
    xVec3 mypos = {};
    
    xCollis* colrec;
    if (colCallers) {
        colrec = colCallers;
    } else {
        static xCollis localCollis = { k_HIT_0xF00 | k_HIT_CALC_HDNG };
        memset(&localCollis, 0, sizeof(localCollis));
        localCollis.flags = k_HIT_0xF00 | k_HIT_CALC_HDNG;

        colrec = &localCollis;
    }

    this->DBG_PStatCont((en_npcperf)1);
    this->DBG_PStatOn((en_npcperf)2);

    if (!this->GetVertPos(NPC_MDLVERT_LOSEYEBALL, &mypos)) {
        xVec3Copy(&mypos, xEntGetCenter(this));
    }

    NPCC_xBoundAway(&this->bound);

    ray.min_t = 0.0f;
    ray.max_t = dist;
    xVec3Sub(&ray.dir, pos, &mypos);
    xVec3Normalize(&ray.dir, &ray.dir);
    xVec3Copy(&ray.origin, &mypos);
    ray.flags = XRAY3_USE_MIN | XRAY3_USE_MAX;
    xRayHitsScene(xscn, &ray, colrec);

    NPCC_xBoundBack(&this->bound);

    if (!(colrec->flags & k_HIT_IT)) {
        result = 1;
    } else if (colrec->dist > dist) {
        result = 1;
    } else if (tgt && colrec->oid) {
        if (tgt->id == colrec->oid) {
            result = 1;
        } else {
            result = 0;
        }
    } else {
        result = 0;
    }

    if (this->DBG_IsNormLog((en_npcdcat)13, 2)) {
        if (result) {
            xDrawSetColor(g_GREEN);
        } else {
            xDrawSetColor(g_NEON_BLUE);
        }

        xDrawLine(&mypos, pos);
    }

    this->DBG_PStatCont((en_npcperf)2);
    this->DBG_PStatOn((en_npcperf)1);

    return result;
}

xModelInstance* zNPCCommon::ModelAtomicHide(S32 index, xModelInstance* mdl)
{
    xModelInstance* minst = mdl ? mdl : this->ModelAtomicFind(index, -1, NULL);
    if (!minst) return minst;

    minst->Flags &= ~0x1;

    return minst;
}

xModelInstance* zNPCCommon::ModelAtomicShow(S32 index, xModelInstance* mdl)
{
    xModelInstance* minst = mdl ? mdl : this->ModelAtomicFind(index, -1, NULL);
    if (!minst) return minst;

    minst->Flags |= 0x2;
    minst->Flags |= 0x1;

    return minst;
}

xModelInstance* zNPCCommon::ModelAtomicFind(S32 index, S32 idx_prev, xModelInstance* mdl_prev)
{
    xModelInstance* da_atomic = NULL;
    xModelInstance* minst = mdl_prev;
    S32 midx = 0;

    if (minst && idx_prev >= 0) {
        midx = idx_prev;
    } else {
        minst = this->model;
    }

    while (minst) {
        if (midx == index) {
            da_atomic = minst;
            break;
        }
        minst = minst->Next;
        midx++;
    }

    return da_atomic;
}

void zNPCCommon::ModelScaleSet(F32 x, F32 y, F32 z)
{
    xModelInstance* minst = this->model;

    while (minst) {
        minst->Scale.x = x;
        minst->Scale.y = y;
        minst->Scale.z = z;
        minst = minst->Next;
    }
}

S32 zNPCCommon::AnimStart(U32 animID, S32 forceRestart)
{
    static S32 dumptable = 0;
    if (dumptable) {
        dumptable = 0;
        this->AnimGetTable();
    }

    xAnimState* r3_ = this->AnimCurState();
    if (r3_ && r3_->ID == animID && !forceRestart) {
        return r3_->ID;
    }

    xAnimTable* r3 = this->AnimGetTable();
    xAnimTransition* da_tran = r3->TransitionList;

    while (da_tran) {
        if (da_tran->Dest->ID == animID) {
            break;
        }
        da_tran = da_tran->Next;
    }

    if (da_tran) {
        xAnimPlayStartTransition(this->model->Anim, da_tran);
    } else {
        xSceneID2Name(globals.sceneCur, this->id);
    }

    if (da_tran) {
        return da_tran->Dest->ID;
    }

    return 0;
}

void zNPCCommon::AnimSetState(U32 animID, F32 time)
{
    xAnimTable* r3 = this->AnimGetTable();
    xAnimState* state = xAnimTableGetStateID(r3, animID);

    xAnimPlaySetState(this->model->Anim->Single, state, time);
}

xAnimState* zNPCCommon::AnimFindState(U32 animID)
{
    xAnimTable* r3 = this->AnimGetTable();
    return xAnimTableGetStateID(r3, animID);
}

xAnimSingle* zNPCCommon::AnimCurSingle()
{
    if (!this->model->Anim && (this->SelfType() & 0xFFFFFF00) == 'NTT\0') {
        return NULL;
    }

    return this->model->Anim->Single;
}

xAnimState* zNPCCommon::AnimCurState()
{
    if (!this->model->Anim && (this->SelfType() & 0xFFFFFF00) == 'NTT\0') {
        return NULL;
    }

    return this->model->Anim->Single->State;
}

U32 zNPCCommon::AnimCurStateID()
{
    xAnimState* r3 = this->AnimCurState();
    if (r3) return r3->ID;
    return 0;
}

F32 zNPCCommon::AnimDuration(xAnimState* ast)
{
    if (!ast) ast = this->AnimCurState();
    if (!ast) return 0.0f;
    return ast->Data->Duration;
}

F32 zNPCCommon::AnimTimeRemain(xAnimState* ast)
{
    return this->AnimDuration(ast) - this->AnimTimeCurrent();
}

F32 zNPCCommon::AnimTimeCurrent()
{
    return this->model->Anim->Single->Time;
}

void zNPCSettings_MakeDummy() NONMATCH("https://decomp.me/scratch/amPtL")
{
    static zNPCSettings dum;
    dum.id = 0xFEEDCAFE;
    dum.baseType = eBaseTypeNPCSettings;
    dum.linkCount = 0;
    dum.baseFlags = k_XBASE_IS_ENABLED;
    dum.type = 0xBAD0CACA;
    dum.version = 2;
    dum.handle = 0;
    dum.basisType = NPCP_BASIS_NONE;
    dum.allowDetect = 1;
    dum.allowWander = 1;
    dum.allowPatrol = 1;
    dum.reduceCollide = 0;
    dum.useNavSplines = 1;
    dum.allowChase = 1;
    dum.allowAttack = 1;
    dum.assumeLOS = 0;
    dum.assumeFOV = 0;
    dum.duploWaveMode = NPCP_DUPOWAVE_CONTINUOUS;
    dum.duploSpawnLifeMax = -1;
    dum.duploSpawnDelay = 5.0f;

    g_dflt_npcsettings = &dum;
}

zNPCSettings* zNPCSettings_Find(U32 id)
{
    zNPCSettings* set = NULL;
    U32 size = 0;

    if (id) {
        set = (zNPCSettings*)xSTFindAsset(id, &size);
    }

    if (!set) {
        set = g_dflt_npcsettings;
    }

    return set;
}

void zNPCCommon::Vibrate(F32 ds2_cur, F32 ds2_max) NONMATCH("https://decomp.me/scratch/A5HIP")
{
    F32 rat = ds2_cur / xmax(ds2_max, 1.0f);

    if (rat < 0.4f) {
        this->Vibrate(NPC_VIBE_HARD, -1.0f);
    } else if (rat < 0.7f) {
        this->Vibrate(NPC_VIBE_NORM, -1.0f);
    } else if (rat < 1.0f) {
        this->Vibrate(NPC_VIBE_SOFT, -1.0f);
    }
}

void zNPCCommon::Vibrate(en_npcvibe vibe, F32 duration)
{
    xRumbleType typ_rum;
    F32 tym_rum;

    switch (vibe) {
    case NPC_VIBE_BUILD_A:
        tym_rum = 0.05f;
        typ_rum = eRumble_Light;
        break;
    case NPC_VIBE_BUILD_B:
        tym_rum = 0.05f;
        typ_rum = eRumble_Medium;
        break;
    case NPC_VIBE_BUILD_C:
        tym_rum = 0.05f;
        typ_rum = eRumble_Heavy;
        break;
    case NPC_VIBE_SOFT:
        tym_rum = 0.1f;
        typ_rum = eRumble_Light;
        break;
    case NPC_VIBE_NORM:
        tym_rum = 0.25f;
        typ_rum = eRumble_Medium;
        break;
    case NPC_VIBE_HARD:
        tym_rum = 0.45f;
        typ_rum = eRumble_Heavy;
        break;
    default:
        tym_rum = 0.0f;
        typ_rum = eRumble_Medium;
        zPadAddRumble(eRumble_Medium, 0.4f, 0, 0);
        break;
    }

    if (duration > 0.0f) {
        tym_rum = duration;
    }

    zPadAddRumble(typ_rum, tym_rum, 0, 0);
}

xVec3* zNPCCommon::MatPosSet(xVec3* pos)
{
    if (pos) {
        xVec3Copy((xVec3*)&this->model->Mat->pos, pos);
    }

    return (xVec3*)&this->model->Mat->pos;
}

S32 NPCC_NPCIsConversing()
{
    return g_isConversation;
}

void zNPCCommon_WonderReset()
{
    g_isConversation = 0;
    g_flg_wonder = 0;
}

void zNPCCommon::WonderOfTalking(S32 inprogress, xBase* owner)
{
    if (inprogress) {
        g_isConversation = 1;
        if (owner) {
            g_ownerConversation = owner;
        } else {
            g_ownerConversation = this;
        }
    } else {
        g_isConversation = 0;
        g_ownerConversation = NULL;
    }
}

S32 zNPCCommon::SomethingWonderful() NONMATCH("https://decomp.me/scratch/nuTRt")
{
    S32 flg_wonder = g_flg_wonder;

    if (globals.player.Health < 1) {
        flg_wonder |= 0x2;
    }

    if (globals.player.ControlOff & ~(CONTROL_OWNER_CRUISE_BUBBLE | CONTROL_OWNER_SPRINGBOARD)) {
        flg_wonder |= 0x4;
    }

    if (cruise_bubble::active() && (this->SelfType() & 0xFFFFFF00) == 'NTF\0') {
        flg_wonder |= 0x40;
    }

    if (globals.cmgr && globals.cmgr->csn) {
        flg_wonder |= 0x1;
    }

    if (g_isConversation) {
        flg_wonder |= 0x8;
    }

    if (!NPCC_LampStatus()) {
        flg_wonder |= 0x20;
    }

    if (zEntTeleportBox_playerIn()) {
        flg_wonder |= 0x80;
    }

    return flg_wonder;
}

S32 zNPCCommon::SndPlayFromAFX(zAnimFxSound* afxsnd, U32* sid_played)
{
    en_NPC_SOUND sndtype;
    F32 radius;
    U32 aidToPlay;
    U32 xsid;
    NPCSndProp* sprop;

    sndtype = NPCS_SndTypeFromHash(afxsnd->ID, this->cfg_npc->snd_trax, this->cfg_npc->snd_traxShare);
    if (!NPCS_SndOkToPlay(sndtype)) {
        return -1;
    }

    sprop = NPCS_SndFindProps(sndtype);
    radius = this->cfg_npc->rad_sound;

    if (sndtype == NPC_STYP_BOGUS) {
        aidToPlay = afxsnd->ID;
    } else {
        aidToPlay = NPCS_SndPickSimilar(sndtype, this->cfg_npc->snd_trax, this->cfg_npc->snd_traxShare);
    }

    xsid = this->SndStart(aidToPlay, sprop, radius);

    if (sid_played) {
        *sid_played = xsid;
    }

    return 1;
}

S32 zNPCCommon::SndPlayFromSFX(xSFX* sfx, U32* sid_played)
{
    U32 aidToPlay;
    U32 xsid;
    NPCSndProp* sprop;

    this->SndKillSounds(2, 0);

    sprop = NPCS_SndFindProps(NPC_STYP_XSFXTALK);
    aidToPlay = sfx->asset->soundAssetID;
    xsid = this->SndStart(aidToPlay, sprop, xmin(sfx->asset->outerRadius, this->cfg_npc->rad_sound));

    if (sid_played) {
        *sid_played = xsid;
    }

    if (xsid) {
        return 1;
    }

    return 0;
}

S32 zNPCCommon::SndPlayRandom(en_NPC_SOUND sndtype) NONMATCH("https://decomp.me/scratch/Y5Wfo")
{
    U32 xsid = 0;
    NPCConfig* cfg = this->cfg_npc;

    if (NPCS_SndOkToPlay(sndtype)) {
        NPCSndProp* sprop = NPCS_SndFindProps(sndtype);
        U32 aidToPlay = NPCS_SndPickSimilar(sndtype, cfg->snd_trax, cfg->snd_traxShare);

        if (aidToPlay) {
            xsid = this->SndStart(aidToPlay, sprop, cfg->rad_sound);
        }
    }

    if (xsid) {
        return 1;
    }

    return 0;
}

U32 zNPCCommon::SndStart(U32 aid_toplay, NPCSndProp* sprop, F32 radius)
{
    F32 pvary = 0.0f;
    U32 priority;
    U32 owner;
    U32 xsid = 0;
    F32 vol;
    S32 flg_snd;
    U32 xsndflags;

    if (!sprop) {
        sprop = NPCS_SndFindProps(NPC_STYP_BOGUS);
    }

    flg_snd = sprop->flg_snd & 0xFFFFFF;
    vol = NPCC_ds2_toCam(this->Pos(), NULL);

    if (SQ(radius) < vol) {
        return 0;
    }

    if ((flg_snd & 0x20000) && this->SndChanIsBusy(1)) {
        return 0;
    }

    if ((flg_snd & 0x8000) && this->SndChanIsBusy(3)) {
        return 0;
    }

    static S32 idx_seq = 0;

    if (flg_snd & 0x100) {
        vol = 1.0f;
        priority = 130;
    } else if (flg_snd & 0x200) {
        vol = 0.85f;
        priority = 125;
    } else if (flg_snd & 0x400) {
        vol = 0.7f;
        priority = 121;
    } else {
        vol = 0.85f;
        priority = 125;
    }

    if (flg_snd & 0x800) {
        static const F32 pitchChoices[] = { -5, -4, -3, -2, -1, 0, 1 };
        pvary = xUtil_choose(pitchChoices, ARRAY_LENGTH(pitchChoices), NULL);
    }

    owner = (U32)this + (flg_snd & 0x3); // weird...
    
    xsndflags = 0x10000;
    if (flg_snd & 0x1000) {
        xsndflags &= ~0x10000;
    }

    if (aid_toplay && (flg_snd & 0x10000)) {
        xsid = xSndPlay(aid_toplay, vol, pvary, priority, xsndflags,
                        owner, SND_CAT_GAME, 0.0f);
    } else if (aid_toplay) {
        if (flg_snd & 0x2000) {
            xsid = xSndPlay3D(aid_toplay, 0.77f * vol, pvary, priority, xsndflags,
                              this->Pos(), 2.0f, radius,
                              SND_CAT_GAME, 0.0f);
        } else {
            xsid = xSndPlay3D(aid_toplay, 0.77f * vol, pvary, priority, xsndflags,
                              (xEnt*)owner, 2.0f, radius,
                              SND_CAT_GAME, 0.0f);
        }
    }

    NPCS_SndTypePlayed(sprop->sndtype, sprop->tym_delayNext);

    return xsid;
}

S32 zNPCCommon::SndIsAnyPlaying()
{
    S32 yep;
    S32 owner = (S32)this;
    S32 i;

    for (i = 0; i < 4; i++) {
        yep = xSndIsPlaying(0, owner + i);
        if (yep) {
            break;
        }
    }

    return yep;
}

S32 zNPCCommon::SndChanIsBusy(S32 flg_chan)
{
    return xSndIsPlaying(0, (U32)this + (flg_chan & 0x3));
}

void zNPCCommon::SndKillSounds(S32 flg_chan, S32 all)
{
    S32 owner = (S32)this;

    if (all) {
        for (S32 i = 0; i < 4; i++) {
            xSndStopChildren(owner + i);
        }
    } else {
        xSndStopChildren(owner + (flg_chan & 0x3));
    }
}

S32 zNPCCommon::SndQueUpdate(F32 dt) NONMATCH("https://decomp.me/scratch/5SMYN")
{
    NPCSndQueue* que;
    NPCSndProp* sprop;
    S32 i;
    S32 cnt = 0;

    for (i = 0; i < 4; i++) {
        que = &this->snd_queue[i];
        if (que->sndtype == NPC_STYP_BOGUS) continue;
        
        cnt++;
        
        que->tmr_delay -= dt;
        if (que->tmr_delay > 0.0f) continue;
        
        cnt--;
        
        sprop = NPCS_SndFindProps(que->sndtype);

        this->SndStart(que->sndDirect, sprop, que->radius);

        que->sndtype = NPC_STYP_BOGUS;
        que->sndDirect = 0;
    }

    if (cnt > 0) {
        this->flg_misc |= 0x2;
    } else {
        this->flg_misc &= ~0x2;
    }

    return cnt;
}

S32 zNPCCommon::LassoInit()
{
    this->lassdata = this->PRIV_GetLassoData();
    
    if (this->lassdata) {
        memset(this->lassdata, 0, sizeof(zNPCLassoInfo));
        this->lassdata->stage = LASS_STAT_PENDING;
        this->lassdata->lassoee = this;
    }

    if (this->lassdata) {
        return 1;
    }

    return 0;
}

S32 zNPCCommon::LassoSetup()
{
    zNPCLassoInfo* lass = this->lassdata;

    if (lass &&
        lass->grabGuideModel && lass->grabGuideAnim &&
        lass->holdGuideModel && lass->holdGuideAnim) {
        zLasso_AddGuide(this, lass->grabGuideAnim, lass->grabGuideModel);
        zLasso_AddGuide(this, lass->holdGuideAnim, lass->holdGuideModel);

        lass->holdGuideModel->Flags &= ~0x1;
        lass->grabGuideModel->Flags &= ~0x1;
        lass->holdGuideModel->Flags |= 0x2;
        lass->grabGuideModel->Flags |= 0x2;

        this->flg_vuln |= 0x1000000;
    } else {
        this->flg_vuln &= ~0x1000000;
    }

    return (this->flg_vuln & 0x1000000);
}

S32 zNPCCommon::LassoUseGuides(S32 idx_grabmdl, S32 idx_holdmdl)
{
    xModelInstance* minst;
    xModelInstance* mod_grab = NULL;
    xModelInstance* mod_hold = NULL;
    zNPCLassoInfo* lass = this->lassdata;
    S32 midx = 0;
    S32 haveAnims = 0;

    for (minst = this->model; minst; minst = minst->Next) {
        if (midx == idx_grabmdl) {
            mod_grab = minst;
        }
        if (midx == idx_holdmdl) {
            mod_hold = minst;
        }
        if (mod_grab && mod_hold) {
            break;
        }

        midx++;
    }

    lass->grabGuideModel = mod_grab;
    lass->holdGuideModel = mod_hold;

    if (mod_grab && mod_hold) {
        haveAnims = this->LassoGetAnims(mod_grab, mod_hold);
    }

    return haveAnims;
}

S32 zNPCCommon::LassoGetAnims(xModelInstance* modgrab, xModelInstance* modhold)
{
    xAnimState* ast;
    xAnimState* ast_grab = NULL;
    xAnimState* ast_hold = NULL;
    zNPCLassoInfo* lass = this->lassdata;

    if (modgrab->Anim && !modhold->Anim) {
        modhold->Anim = modgrab->Anim;
    }

    if (!modgrab->Anim && modhold->Anim) {
        modgrab->Anim = modhold->Anim;
    }

    if (!modgrab->Anim || !modhold->Anim) {
        xAnimPlay* play = this->model->Anim;
        while (play) {
            xAnimTable* tab = play->Table;
            if (tab && !strcmp(tab->Name, "LassoGuides")) {
                modgrab->Anim = play;
                modhold->Anim = play;
                break;
            }
            play = play->Next;
        }
    }

    if (modhold->Anim && modhold->Anim) { // BUG: modgrab->Anim not checked
        ast = modgrab->Anim->Table ? modgrab->Anim->Table->StateList : NULL;

        while (ast) {
            if (ast->ID == g_hash_lassanim[LASS_ANIM_GRAB]) {
                ast_grab = ast;
            }
            if (ast->ID == g_hash_lassanim[LASS_ANIM_HOLD]) {
                ast_hold = ast;
            }
            if (ast_grab && ast_hold) {
                break;
            }
            ast = ast->Next;
        }

        lass->grabGuideAnim = ast_grab;
        lass->holdGuideAnim = ast_hold;
    }

    return ast_grab && ast_hold;
}

void zNPCCommon::LassoSyncAnims(en_lassanim lassanim)
{
    xAnimState* lass_ast = NULL;
    xModelInstance* lass_mdl = NULL;

    switch (lassanim) {
    case LASS_ANIM_GRAB:
        lass_ast = this->lassdata->grabGuideAnim;
        lass_mdl = this->lassdata->grabGuideModel;
        break;
    case LASS_ANIM_HOLD:
        lass_ast = this->lassdata->holdGuideAnim;
        lass_mdl = this->lassdata->holdGuideModel;
        break;
    }

    if (lass_ast && lass_mdl) {
        xAnimPlaySetState(lass_mdl->Anim->Single, lass_ast, this->AnimTimeCurrent());
    }

    this->DBG_IsNormLog((en_npcdcat)14, -1);
}

zNPCLassoInfo* zNPCCommon::GimmeLassInfo()
{
    if (this->flg_vuln & 0x1000000) {
        return this->lassdata;
    }
    return NULL;
}

void zNPCCommon::LassoNotify(en_LASSO_EVENT event)
{
    zNPCLassoInfo* lass = this->lassdata;

    if (lass->stage == LASS_STAT_DONE && event != LASS_EVNT_BEGIN) {
        return;
    }

    switch (event) {
    case LASS_EVNT_BEGIN:
        lass->stage = LASS_STAT_PENDING;
        break;
    case LASS_EVNT_ENDED:
        lass->stage = LASS_STAT_DONE;
        break;
    case LASS_EVNT_GRABSTART:
        lass->stage = LASS_STAT_GRABBING;
        break;
    case LASS_EVNT_YANK:
        lass->stage = LASS_STAT_TOSSING;
        break;
    case LASS_EVNT_ABORT:
        lass->stage = LASS_STAT_DONE;
        break;
    }
}

void zNPCCommon::AddBaseline(xPsyche* psy,
                             xGoalProcessCallback eval_idle,
                             xGoalProcessCallback eval_patrol,
                             xGoalProcessCallback eval_wander,
                             xGoalProcessCallback eval_waiting,
                             xGoalProcessCallback eval_fidget)
{
    xGoal* goal;

    goal = psy->AddGoal('NGN0', NULL);
    goal->SetCallbacks(eval_idle, NULL, NULL, NULL);

    goal = psy->AddGoal('NGN1', NULL);
    goal->SetCallbacks(eval_wander, NULL, NULL, NULL);

    goal = psy->AddGoal('NGN2', NULL);
    goal->SetCallbacks(eval_patrol, NULL, NULL, NULL);

    goal = psy->AddGoal('NGN4', NULL);
    goal->SetCallbacks(eval_waiting, NULL, NULL, NULL);

    goal = psy->AddGoal('NGN3', NULL);
    goal->SetCallbacks(eval_fidget, NULL, NULL, NULL);

    goal = psy->AddGoal('NGN7', NULL);

    this->AddDEVGoals(psy);
}

void zNPCCommon::AddScripting(xPsyche* psy,
                              xGoalProcessCallback eval_script,
                              xGoalProcessCallback eval_playanim,
                              xGoalProcessCallback eval_attack,
                              xGoalProcessCallback eval_move,
                              xGoalProcessCallback eval_follow,
                              xGoalProcessCallback eval_lead,
                              xGoalProcessCallback eval_wait) NONMATCH("https://decomp.me/scratch/xuZpV")
{
    xGoal* goal;

    this->DBG_Name();

    if (this->flg_misc & 0x1) {
        goal = psy->AddGoal('NGS0', NULL);
        goal->SetCallbacks(eval_script, NULL, NULL, NULL);

        goal = psy->AddGoal('NGS1', NULL);
        goal->SetCallbacks(eval_playanim, NULL, NULL, NULL);

        goal = psy->AddGoal('NGS2', NULL);
        goal->SetCallbacks(eval_attack, NULL, NULL, NULL);

        goal = psy->AddGoal('NGS3', NULL);
        goal->SetCallbacks(eval_move, NULL, NULL, NULL);

        goal = psy->AddGoal('NGS5', NULL);
        goal->SetCallbacks(eval_follow, NULL, NULL, NULL);

        goal = psy->AddGoal('NGS6', NULL);
        goal->SetCallbacks(eval_lead, NULL, NULL, NULL);

        goal = psy->AddGoal('NGS7', NULL);
        goal->SetCallbacks(eval_wait, NULL, NULL, NULL);
    }
}

void zNPCCommon::AddDEVGoals(xPsyche* psy)
{
}

xAnimTable* ZNPC_AnimTable_Common() NONMATCH("https://decomp.me/scratch/ypJan")
{
    xAnimTable* table = xAnimTableNew("zNPCCommon", NULL, 0);

    xAnimTableNewStateDefault(table, "Idle01", 0x110, 0x1);

    return table;
}

xAnimTable* ZNPC_AnimTable_LassoGuide() NONMATCH("https://decomp.me/scratch/gdJJ5")
{
    xAnimTable* table = xAnimTableNew("LassoGuides", NULL, 0);

    xAnimTableNewStateDefault(table, g_strz_lassanim[LASS_ANIM_GRAB], 0, 0x1);
    xAnimTableNewStateDefault(table, g_strz_lassanim[LASS_ANIM_HOLD], 0, 0x1);

    xAnimTableNewTransitionDefault(table, g_strz_lassanim[LASS_ANIM_GRAB], g_strz_lassanim[LASS_ANIM_HOLD], 0, NULL);

    return table;
}

void NPCC_BuildStandardAnimTran(xAnimTable* table, char** namelist, S32* ourAnims, S32 idx_dflt, F32 blend) NONMATCH("https://decomp.me/scratch/iKxSo")
{
    xAnimTransition* def = NULL;
    char** names = namelist;
    S32 i;

    for (i = 0; ourAnims[i];) {
        if (idx_dflt == ourAnims[i]) {
            i++;
            continue;
        }

        if (!def) {
            def = xAnimTableNewTransition(table, names[ourAnims[i]], names[idx_dflt], NULL, NULL, 0x10, 0, 0.0f, 0.0f, 0, 0, blend, NULL);
            continue;
        }

        xAnimTableAddTransition(table, def, names[ourAnims[i]]);
        i++;
    }

    for (i = 0; ourAnims[i];) {
        if (idx_dflt == ourAnims[i]) {
            i++;
            continue;
        }

        xAnimTableNewTransition(table, names[ourAnims[i]], names[idx_dflt], NULL, NULL, 0, 0, 0.0f, 0.0f, 0, 0, blend, NULL);
        xAnimTableNewTransition(table, names[idx_dflt], names[ourAnims[i]], NULL, NULL, 0, 0, 0.0f, 0.0f, 0, 0, blend, NULL);
        i++;
    }
}

void zNPCCommon_EjectPhlemOnPawz()
{
    zGameIsPaused();

    if (globals.sceneCur->pendingPortal) {
        NPCWidget* talk_font = NPCWidget_Find(NPC_WIDGE_TALK);
        if (talk_font && talk_font->IsVisible()) {
            talk_font->Off(NULL, 1);
        }
    }
}
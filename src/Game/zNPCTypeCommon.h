#pragma once

#include "xNPCBasic.h"
#include "xDynAsset.h"
#include "xEntDrive.h"
#include "xBehaveMgr.h"
#include "zNPCConfig.h"
#include "zMovePoint.h"
#include "zNPCSnd.h"
#include "zNPCMessenger.h"
#include "zNPCSpawner.h"
#include "zNPCLasso.h"
#include "zNPCGoalCommon.h"
#include "zGlobals.h"

struct zShrapnelAsset;
struct xModelAssetParam;
enum en_ZBASETYPE;
struct zAnimFxSound;
struct xSFX;

struct xEntNPCAsset
{
    S32 npcFlags;
    S32 npcModel;
    S32 npcProps;
    U32 movepoint;
    U32 taskWidgetPrime;
    U32 taskWidgetSecond;
};

enum en_npcbtyp
{
    NPCP_BASIS_NONE,
    NPCP_BASIS_EVILROBOT,
    NPCP_BASIS_FRIENDLYROBOT,
    NPCP_BASIS_LOVINGCITIZEN,
    NPCP_BASIS_GRUMPYCITIZEN,
    NPCP_BASIS_NOMORE,
    NPCP_BASIS_FORCE = FORCEENUMSIZEINT
};

enum en_dupowavmod
{
    NPCP_DUPOWAVE_CONTINUOUS,
    NPCP_DUPOWAVE_DISCREET,
    NPCP_DUPOWAVE_NOMORE,
    NPCP_DUPOWAVE_FORCE = FORCEENUMSIZEINT
};

struct zNPCSettings : xDynAsset
{
    en_npcbtyp basisType;
    char allowDetect;
    char allowPatrol;
    char allowWander;
    char reduceCollide;
    char useNavSplines;
    char pad[3];
    char allowChase;
    char allowAttack;
    char assumeLOS;
    char assumeFOV;
    en_dupowavmod duploWaveMode;
    F32 duploSpawnDelay;
    S32 duploSpawnLifeMax;
};

enum en_npcparm
{
    NPC_PARM_NONE,
    NPC_PARM_MOVERATE,
    NPC_PARM_TURNRATE,
    NPC_PARM_ACCEL,
    NPC_PARM_DRIFT,
    NPC_PARM_MASS,
    NPC_PARM_TOSSGRAV,
    NPC_PARM_TOSSELASTIC,
    NPC_PARM_BND_ISBOX,
    NPC_PARM_BND_CENTER,
    NPC_PARM_BND_EXTENT,
    NPC_PARM_HITPOINTS,
    NPC_PARM_MODELSCALE,
    NPC_PARM_DETECT_RAD,
    NPC_PARM_DETECT_HYT,
    NPC_PARM_DETECT_OFF,
    NPC_PARM_ATTACK_RAD,
    NPC_PARM_ATTACK_FOV,
    NPC_PARM_SND_RAD,
    NPC_PARM_TIMEFIDGET,
    NPC_PARM_TIMEATTACK,
    NPC_PARM_TIMESTUN,
    NPC_PARM_TIMEALERT,
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
    NPC_PARM_ATK_SIZE01,
    NPC_PARM_ATK_FRAMES01,
    NPC_PARM_ATK_FRAMES01A,
    NPC_PARM_ATK_FRAMES01B,
    NPC_PARM_ATK_FRAMES02,
    NPC_PARM_ATK_FRAMES02A,
    NPC_PARM_ATK_FRAMES02B,
    NPC_PARM_ATK_FRAMES03,
    NPC_PARM_ATK_FRAMES03A,
    NPC_PARM_ATK_FRAMES03B,
    NPC_PARM_ESTEEM_A,
    NPC_PARM_ESTEEM_B,
    NPC_PARM_ESTEEM_C,
    NPC_PARM_ESTEEM_D,
    NPC_PARM_ESTEEM_E,
    NPC_PARM_SHADOW_CASTDIST,
    NPC_PARM_SHADOW_RADCACHE,
    NPC_PARM_SHADOW_RADRASTER,
    NPC_PARAM_TEST_COUNT,
    NPC_PARM_ENDTAG_INI,
    NPC_PARM_FIRSTMVPT,
    NPC_PARM_ENDTAG_PROPS,
    NPC_PARM_BOGUSSHARE,
    NPC_PARM_ENDTAG_SHARE,
    NPC_PARM_NOMORE,
    NPC_PARM_FORCEINT = FORCEENUMSIZEINT
};

enum en_npcvibe
{
    NPC_VIBE_SOFT,
    NPC_VIBE_NORM,
    NPC_VIBE_HARD,
    NPC_VIBE_BUILD_A,
    NPC_VIBE_BUILD_B,
    NPC_VIBE_BUILD_C,
    NPC_VIBE_NOMORE,
    NPC_VIBE_FORCE = FORCEENUMSIZEINT
};

struct zNPCCommon : xNPCBasic
{
protected:
    xEntAsset* entass;
    xEntNPCAsset* npcass;
    zNPCSettings* npcsetass;
    S32 flg_vuln;
    S32 flg_move;
    S32 flg_misc;
    S32 flg_able;
    NPCConfig* cfg_npc;
    zNPCSettings npcset;
    zMovePoint* nav_past;
    zMovePoint* nav_curr;
    zMovePoint* nav_dest;
    zMovePoint* nav_lead;
    xSpline3* spl_mvptspline;
    F32 len_mvptspline;
    F32 dst_curspline;
    xEntDrive* drv_data;
    xPsyche* psy_instinct;
    zNPCCommon* npc_duplodude;
    F32 spd_throttle;
    S32 flg_xtrarend;
    F32 tmr_fidget;
    F32 tmr_invuln;
    zShrapnelAsset* explosion;
    xModelAssetParam* parmdata;
    U32 pdatsize;
    zNPCLassoInfo* lassdata;
    NPCSndQueue snd_queue[4];

public:
    zNPCCommon(S32 myType) : xNPCBasic(myType) {}

    virtual void Init(xEntAsset* entass);
    virtual void Setup();
    virtual void Reset();
    virtual void Process(xScene* xscn, F32 dt);
    virtual void BUpdate(xVec3* pos);
    virtual void NewTime(xScene* xscn, F32 dt);
    virtual void Move(xScene* xscn, F32 dt, xEntFrame* frm);
    virtual S32 SysEvent(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget, S32* handled);
    virtual void CollideReview();
    virtual void Destroy();
    virtual S32 NPCMessage(NPCMsg* mail);
    virtual void RenderExtra() {}
    virtual void RenderExtraPostParticles() {}
    virtual void ParseINI();
    virtual void ParseLinks();
    virtual void ParseProps();
    virtual void SelfSetup() {}
    virtual void SelfDestroy();
    virtual S32 IsHealthy() { return 1; }
    virtual S32 IsAlive() { return 1; }
    virtual void Damage(en_NPC_DAMAGE_TYPE damtype, xBase* who, const xVec3* vec_hit);
    virtual S32 Respawn(const xVec3* pos, zMovePoint* mvptFirst, zMovePoint* mvptSpawnRef);
    virtual void DuploOwner(zNPCCommon* duper) { this->npc_duplodude = duper; }
    virtual void DuploNotice(en_SM_NOTICES note, void* data) {}
    virtual S32 CanRope() { return this->flg_vuln & 0x1000000; }
    virtual void LassoNotify(en_LASSO_EVENT event);
    virtual S32 SetCarryState(en_NPC_CARRY_STATE stat) { return 0; }
    virtual void Stun(F32 stuntime) {}
    virtual void SpeakBegin() {}
    virtual void SpeakEnd() {}
    virtual void SpeakStart(U32 sndid, U32 sndhandle, S32 anim) {}
    virtual void SpeakStop() {}
    virtual U32 AnimPick(S32 gid, en_NPC_GOAL_SPOT gspot, xGoal* rawgoal) { return 0; }
    virtual void GetParm(en_npcparm pid, void* val);
    virtual S32 GetParmDefault(en_npcparm pid, void* val);
    virtual F32 GenShadCacheRad() { return 2.4f; }
    virtual xEntDrive* PRIV_GetDriverData() { return NULL; }
    virtual zNPCLassoInfo* PRIV_GetLassoData() { return NULL; }
    virtual S32 LassoSetup();

    void InitBounds();
    F32 BoundAsRadius(S32 useCfg) const;
    void ConvertHitEvent(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget, S32* handled);
    void VelStop();
    F32 ThrottleAdjust(F32 dt, F32 spd_want, F32 accel);
    F32 ThrottleAccel(F32 dt, S32 speedup, F32 pct_max);
    F32 ThrottleApply(F32 dt, const xVec3* dir, S32 force3D);
    F32 TurnToFace(F32 dt, const xVec3* dir_want, F32 useTurnRate);
    S32 IsMountableType(en_ZBASETYPE type);
    void TagVerts();
    S32 GetVertPos(en_mdlvert vid, xVec3* pos);
    S32 IsAttackFrame(F32 tym_anim, S32 series);
    void GiveReward();
    void PlayerKiltMe();
    void ISeePlayer();
    static void ConfigSceneDone();
    NPCConfig* ConfigCreate(U32 modelID);
    NPCConfig* ConfigFind(U32 modelID);
    void GetParm(en_npcparm pid, S32* val);
    void GetParm(en_npcparm pid, F32* val);
    void GetParm(en_npcparm pid, xVec3* val);
    void GetParm(en_npcparm pid, zMovePoint** val);
    S32 CanDoSplines();
    zMovePoint* FirstAssigned();
    void MvptReset(zMovePoint* nav_goto);
    S32 MvptCycle();
    S32 HaveLOSToPos(xVec3* pos, F32 dist, xScene* xscn, xBase* tgt, xCollis* colCallers);
    xModelInstance* ModelAtomicHide(S32 index, xModelInstance* mdl);
    xModelInstance* ModelAtomicShow(S32 index, xModelInstance* mdl);
    xModelInstance* ModelAtomicFind(S32 index, S32 idx_prev, xModelInstance* mdl_prev);
    void ModelScaleSet(F32 x, F32 y, F32 z);
    S32 AnimStart(U32 animID, S32 forceRestart);
    void AnimSetState(U32 animID, F32 time);
    xAnimState* AnimFindState(U32 animID);
    xAnimSingle* AnimCurSingle();
    xAnimState* AnimCurState();
    U32 AnimCurStateID();
    F32 AnimDuration(xAnimState* ast);
    F32 AnimTimeRemain(xAnimState* ast);
    F32 AnimTimeCurrent();
    void Vibrate(F32 ds2_cur, F32 ds2_max);
    void Vibrate(en_npcvibe vibe, F32 duration);
    xVec3* MatPosSet(xVec3* pos);
    void WonderOfTalking(S32 inprogress, xBase* owner);
    S32 SomethingWonderful();
    S32 SndPlayFromAFX(zAnimFxSound* afxsnd, U32* sid_played);
    S32 SndPlayFromSFX(xSFX* sfx, U32* sid_played);
    S32 SndPlayRandom(en_NPC_SOUND sndtype);
    U32 SndStart(U32 aid_toplay, NPCSndProp* sprop, F32 radius);
    S32 SndIsAnyPlaying();
    S32 SndChanIsBusy(S32 flg_chan);
    void SndKillSounds(S32 flg_chan, S32 all);
    S32 SndQueUpdate(F32 dt);
    S32 LassoInit();
    S32 LassoUseGuides(S32 idx_grabmdl, S32 idx_holdmdl);
    S32 LassoGetAnims(xModelInstance* modgrab, xModelInstance* modhold);
    void LassoSyncAnims(en_lassanim lassanim);
    zNPCLassoInfo* GimmeLassInfo();
    void AddBaseline(xPsyche* psy,
                     xGoalProcessCallback eval_idle,
                     xGoalProcessCallback eval_patrol,
                     xGoalProcessCallback eval_wander,
                     xGoalProcessCallback eval_waiting,
                     xGoalProcessCallback eval_fidget);
    void AddScripting(xPsyche* psy,
                      xGoalProcessCallback eval_script,
                      xGoalProcessCallback eval_playanim,
                      xGoalProcessCallback eval_attack,
                      xGoalProcessCallback eval_move,
                      xGoalProcessCallback eval_follow,
                      xGoalProcessCallback eval_lead,
                      xGoalProcessCallback eval_wait);
    void AddDEVGoals(xPsyche* psy);

    xVec3* Pos() { return (xVec3*)&this->model->Mat->pos; }
    xVec3* Center() { return xEntGetCenter(this); }

    xMat4x3* BoneMat(S32 r4) const
    {
        return (xMat4x3*)&this->model->Mat[r4];
    }

    xVec3* BonePos(S32 r4) const
    {
        return (xVec3*)&this->model->Mat[r4].pos;
    }

    void XZVecToPos(xVec3* r29, const xVec3* r30, F32* r31)
    {
        xVec3Sub(r29, r30, this->Pos());
        if (r31) *r31 = r29->y;
        r29->y = 0.0f;
    }

    void XZVecToPlayer(xVec3* r30, F32* r31)
    {
        this->XZVecToPos(r30, xEntGetPos(&globals.player.ent), r31);
    }

    void XYZVecToPos(xVec3* r30, xVec3* r31)
    {
        xVec3Sub(r30, r31, this->Pos());
    }

    F32 XZDstSqToPos(const xVec3* r5, xVec3* r31, F32* r6)
    {
        xVec3 var_18;
        if (!r31) r31 = &var_18;
        this->XZVecToPos(r31, r5, r6);
        return xVec3Length2(r31);
    }

    F32 XZDstSqToPlayer(xVec3* r30, F32* r31)
    {
        return this->XZDstSqToPos(xEntGetPos(&globals.player.ent), r30, r31);
    }

    F32 XYZDstSqToPos(xVec3* r5, xVec3* r31)
    {
        xVec3 var_18;
        if (!r31) r31 = &var_18;
        this->XYZVecToPos(r31, r5);
        return xVec3Length2(r31);
    }

    F32 XYZDstSqToPlayer(xVec3* r31)
    {
        this->XYZDstSqToPos(xEntGetPos(&globals.player.ent), r31);
    }

    void ModelScaleSet(F32 s) { this->ModelScaleSet(s, s, s); }
    void ModelScaleSet(const xVec3* s) { this->ModelScaleSet(s->x, s->y, s->z); }
    xAnimTable* AnimGetTable() { return this->model->Anim->Table; }

    const char* DBG_Name() { return NULL; }
    const char* DBG_InstName() { return this->DBG_Name(); }
    void DBG_AddTweakers() {}
    void DBG_RptDataSize() {}
};

extern U32 g_hash_lassanim[LASS_ANIM_NOMORE];
extern const char* g_strz_lassanim[LASS_ANIM_NOMORE];

xFactoryInst* ZNPC_Create_Common(S32 who, RyzMemGrow* grow, void*);
void ZNPC_Destroy_Common(xFactoryInst* inst);
void ZNPC_Common_Startup();
void ZNPC_Common_Shutdown();
void zNPCCommon_ScenePrepare();
void zNPCCommon_SceneFinish();
void zNPCCommon_SceneReset();
void zNPCCommon_ScenePostInit();
void zNPCCommon_Timestep(xScene*, F32 dt);
void zNPCSettings_MakeDummy();
zNPCSettings* zNPCSettings_Find(U32);
S32 NPCC_NPCIsConversing();
void zNPCCommon_WonderReset();
xAnimTable* ZNPC_AnimTable_Common();
xAnimTable* ZNPC_AnimTable_LassoGuide();
void NPCC_BuildStandardAnimTran(xAnimTable* table, char** namelist, S32* ourAnims, S32 idx_dflt, F32 blend);
void zNPCCommon_EjectPhlemOnPawz();
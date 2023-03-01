#pragma once

#include "xNPCBasic.h"
#include "xDynAsset.h"
#include "xEntDrive.h"
#include "xBehaveMgr.h"
#include "zNPCConfig.h"
#include "zMovePoint.h"
#include "zNPCSndTable.h"
#include "zNPCMessenger.h"
#include "zNPCSpawner.h"
#include "zNPCLasso.h"
#include "zNPCGoalCommon.h"

struct zShrapnelAsset;
struct xModelAssetParam;

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
typedef enum en_npcbtyp npcbtyp;

enum en_dupowavmod
{
    NPCP_DUPOWAVE_CONTINUOUS,
    NPCP_DUPOWAVE_DISCREET,
    NPCP_DUPOWAVE_NOMORE,
    NPCP_DUPOWAVE_FORCE = FORCEENUMSIZEINT
};
typedef enum en_dupowavmod dupowavmod;

struct zNPCSettings : xDynAsset
{
    npcbtyp basisType;
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
    dupowavmod duploWaveMode;
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
typedef enum en_npcparm npcparm;

class zNPCCommon : public xNPCBasic
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
    virtual void Damage(NPC_DAMAGE_TYPE damtype, xBase* who, const xVec3* vec_hit);
    virtual S32 Respawn(const xVec3* pos, zMovePoint* mvptFirst, zMovePoint* mvptSpawnRef);
    virtual void DuploOwner(zNPCCommon* duper) { npc_duplodude = duper; }
    virtual void DuploNotice(SM_NOTICES note, void* data);
    virtual S32 CanRope() WIP { return 0; }
    virtual void LassoNotify(LASSO_EVENT event);
    virtual S32 SetCarryState(NPC_CARRY_STATE stat) { return 0; }
    virtual void Stun(F32 stuntime) {}
    virtual void SpeakBegin() {}
    virtual void SpeakEnd() {}
    virtual void SpeakStart(U32 sndid, U32 sndhandle, S32 anim) {}
    virtual void SpeakStop() {}
    virtual U32 AnimPick(S32 gid, NPC_GOAL_SPOT gspot, xGoal* rawgoal) { return 0; }
    virtual void GetParm(npcparm pid, void* val);
    virtual S32 GetParmDefault(en_npcparm pid, void* val);
    virtual F32 GenShadCacheRad() { return 2.4f; }
    virtual xEntDrive* PRIV_GetDriverData() { return NULL; }
    virtual zNPCLassoInfo* PRIV_GetLassoData() { return NULL; }
    virtual S32 LassoSetup();
};
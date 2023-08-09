#pragma once

#include "xEnt.h"

struct zNPCCommon;
struct zMovePoint;

enum en_NPC_UI_WIDGETS
{
    NPC_WIDGE_TALK,
    NPC_WIDGE_NOMORE,
    NPC_WIDGE_FORCE
};

struct NPCWidget
{
    en_NPC_UI_WIDGETS idxID;
    xBase* base_widge;
    const zNPCCommon* npc_ownerlock;
    
    void Init(en_NPC_UI_WIDGETS);
    void Reset();
    S32 On(const zNPCCommon* npc, S32 theman);
    S32 Off(const zNPCCommon* npc, S32 theman);
    S32 IsVisible();
    S32 Lock(const zNPCCommon*);
    S32 Unlock(const zNPCCommon*);
    S32 NPCIsTheLocker(const zNPCCommon* npc_lock);
    S32 IsLocked() { return this->npc_ownerlock != NULL; }
};

enum en_npctgt
{
    NPC_TGT_NONE,
    NPC_TGT_PLYR,
    NPC_TGT_ENT,
    NPC_TGT_BASE,
    NPC_TGT_POS,
    NPC_TGT_MVPT,
    NPC_TGT_NOMORE,
    NPC_TGT_FORCEINT = FORCEENUMSIZEINT
};

struct NPCTarget
{
    en_npctgt typ_target;
    union
    {
        xEnt* ent_target;
        xBase* bas_target;
        xVec3 pos_target;
        zMovePoint* nav_target;
    };
    zNPCCommon* npc_owner;

    void TargetSet(xEnt*, S32);
    void TargetClear();
    S32 FindNearest(S32 flg_consider, xBase* skipme, xVec3* from, F32 dst_max);
    void PosGet(xVec3* pos);
    S32 InCylinder(xVec3* from, F32 rad, F32 hyt, F32 off);
    S32 IsDead();
    S32 HaveTarget() { return this->typ_target != NPC_TGT_NONE; }
};

struct NPCLaser
{
    RwRaster* rast_laser;
    F32 radius[2];
    F32 uv_scroll[2];
    RwRGBA rgba[2];
    F32 uv_base[2];

    void Render(xVec3* pos_src, xVec3* pos_tgt);

    void Prepare() NONMATCH("https://decomp.me/scratch/1MyUe")
    {
        this->rast_laser = NULL;
        this->uv_base[0] = 0.0f;
        this->uv_base[1] = 0.0f;
    }

    void TextureSet(RwRaster* r3)
    {
        this->rast_laser = r3;
    }

    RwRaster* TextureGet()
    {
        return this->rast_laser;
    }

    void RadiusSet(F32 f1, F32 f2)
    {
        this->radius[0] = f1;
        this->radius[1] = f2;
    }

    void UVScrollSet(F32 f1, F32 f2)
    {
        this->uv_scroll[0] = f1;
        this->uv_scroll[1] = f2;
    }

    void UVScrollUpdate(F32 dt) NONMATCH("https://decomp.me/scratch/6wfKG")
    {
        this->uv_base[0] += dt * this->uv_scroll[0];

        while (this->uv_base[0] > 1.0f) this->uv_base[0] -= 1.0f;
        while (this->uv_base[0] < -0.0f) this->uv_base[0] += 1.0f;

        this->uv_base[1] += dt * this->uv_scroll[1];

        while (this->uv_base[1] > 1.0f) this->uv_base[1] -= 1.0f;
        while (this->uv_base[1] < -0.0f) this->uv_base[1] += 1.0f;
    }

    void ColorSet(const RwRGBA* r3, const RwRGBA* r4)
    {
        this->rgba[0] = *r3;
        this->rgba[1] = *r4;
    }
};

struct NPCCone
{
    F32 rad_cone;
    RwRGBA rgba_top;
    RwRGBA rgba_bot;
    RwRaster* rast_cone;
    F32 uv_tip[2];
    F32 uv_slice[2];

    void RenderCone(xVec3* pos_tiptop, xVec3* pos_botcenter);

    void RadiusSet(F32 f1)
    {
        this->rad_cone = f1;
    }

    void ColorSet(RwRGBA r3, RwRGBA r4)
    {
        this->rgba_top = r3;
        this->rgba_bot = r4;
    }

    void TextureSet(RwRaster* r3)
    {
        this->rast_cone = r3;
    }

    void UVBaseSet(F32 f1, F32 f2)
    {
        this->uv_tip[0] = f1;
        this->uv_tip[1] = f2;
    }

    void UVSliceSet(F32 f1, F32 f2)
    {
        this->uv_slice[0] = f1;
        this->uv_slice[1] = f2;
    }
};

struct NPCBlinker
{
    F32 tmr_uvcell;
    S32 idx_uvcell;

    void Reset();
    void Update(F32 dt, F32 ratio, F32 tym_slow, F32 tym_fast);
    void IndexToUVCoord(S32, F32*, F32*);
    void Render(const xVec3* pos_blink, F32 rad_blink, const RwRaster* rast_blink);
};

enum en_fwstate
{
    FW_STAT_UNUSED,
    FW_STAT_READY,
    FW_STAT_FLIGHT,
    FW_STAT_BOOM,
    FW_STAT_DONE,
    FW_STAT_NOMORE,
    FW_STAT_FORCE = FORCEENUMSIZEINT
};

enum en_fwstyle
{
    FW_STYL_DEFAULT,
    FW_STYL_JULY4TH,
    FW_STYL_XMAS,
    FW_STYL_STPATS,
    FW_STYL_VALENTINE,
    FW_STYL_NOMORE,
    FW_STYL_FORCE = FORCEENUMSIZEINT
};

struct Firework
{
    static F32 acc_thrust;
    static F32 acc_gravity;

    en_fwstate fwstate : 8;
    en_fwstyle fwstyle : 8;
    S32 flg_firework : 16;
    F32 tym_lifespan;
    F32 tmr_remain;
    xVec3 pos;
    xVec3 vel;

    void Cleanup();
    void Update(F32 dt);
    void FlyFlyFly(F32 dt);
    void Detonate();
};

typedef enum _tageNPCSnd
{
    eNPCSnd_GloveAttack,
    eNPCSnd_SleepyAttack,
    eNPCSnd_TubeAttack,
    eNPCSnd_FodBzztAttack,
    eNPCSnd_JellyfishAttack,
    eNPCSnd_Total
} eNPCSnd;

extern S32 g_pc_playerInvisible;

void NPCSupport_Startup();
void NPCSupport_Shutdown();
void NPCSupport_ScenePrepare();
void NPCSupport_SceneFinish();
void NPCSupport_ScenePostInit();
void NPCSupport_SceneReset();
void NPCSupport_Timestep(F32 dt);
void NPCWidget_Startup();
void NPCWidget_Shutdown();
void NPCWidget_ScenePrepare();
void NPCWidget_SceneFinish();
void NPCWidget_SceneReset();
void NPCWidget_ScenePostInit();
NPCWidget* NPCWidget_Find(en_NPC_UI_WIDGETS which);
void Firework_Release(Firework*);
void Firework_ScenePrepare();
void Firework_SceneFinish();
void Firework_SceneReset(S32);
void Firework_Timestep(F32 dt);
void NPCC_ang_toXZDir(F32 angle, xVec3* dir);
F32 NPCC_dir_toXZAng(const xVec3* dir);
F32 NPCC_aimMiss(xVec3* dir_aim, xVec3* pos_src, xVec3* pos_tgt, F32 dst_miss, xVec3* pos_miss);
F32 NPCC_aimVary(xVec3* dir_aim, xVec3* pos_src, xVec3* pos_tgt, F32 dst_vary, S32 flg_vary, xVec3* pos_aimPoint);
S32 NPCC_chk_hitPlyr(xBound* bnd, xCollis* collide);
S32 NPCC_chk_hitEnt(xEnt* tgt, xBound* bnd, xCollis* collide);
S32 NPCC_LineHitsBound(xVec3* a, xVec3* b, xBound* bnd, xCollis* callers_colrec);
S32 NPCC_bnd_ofBase(xBase*, xBound*);
S32 NPCC_pos_ofBase(xBase* tgt, xVec3* pos);
void NPCC_xBoundAway(xBound* bnd);
void NPCC_xBoundBack(xBound* bnd);
S32 NPCC_HaveLOSToPos(xVec3* pos_src, xVec3* pos_tgt, F32 dst_max, xBase* tgt, xCollis* colCallers);
F32 NPCC_DstSqPlyrToPos(const xVec3* pos);
F32 NPCC_ds2_toCam(const xVec3* pos_from, xVec3* delta);
void NPCC_Bounce(xVec3* vec_input, xVec3* vec_anti, F32 elastic);
void NPCC_rotHPB(xMat3x3* mat, F32 heading, F32 pitch, F32 bank);
F32 NPCC_TmrCycle(F32* tmr, F32 dt, F32 interval);
void NPCC_MakePerp(xVec3* dir_perp, const xVec3* dir_axis);
void NPCC_MakeArbPlane(const xVec3* dir_norm, xVec3* at, xVec3* rt);
RwTexture* NPCC_FindRWTexture(const char* txtrname);
RwTexture* NPCC_FindRWTexture(U32 hashid);
RwRaster* NPCC_FindRWRaster(const char* txtrname);
RwRaster* NPCC_FindRWRaster(RwTexture* txtr);
void NPCC_GenSmooth(xVec3** pos_base, xVec3** pos_mid);
void zNPC_SNDInit();
void zNPC_SNDPlay3D(eNPCSnd snd, xEnt* ent);
void zNPC_SNDStop(eNPCSnd snd);
S32 NPCC_LampStatus();
U32 NPCC_ForceTalkOk();

inline xVec3* NPCC_faceDir(xEnt* ent) { return (xVec3*)&ent->model->Mat->at; }
inline xVec3* NPCC_rightDir(xEnt* ent) { return (xVec3*)&ent->model->Mat->right; }
inline xVec3* NPCC_upDir(xEnt* ent) { return (xVec3*)&ent->model->Mat->up; }

inline F32 NPCC_DstSq(const xVec3* r3, const xVec3* r4, xVec3* r5)
{
    xVec3 var_18;
    if (!r5) r5 = &var_18;
    xVec3Sub(r5, r4, r3);
    return xVec3Length2(r5);
}

inline F32 SQ(F32 x) { return x * x; }
inline F32 LERP(F32 f1, F32 f2, F32 f3) { return f1 * (f3 - f2) + f2; }
inline U8 LERP(F32 f1, U8 r3, U8 r4) { return (U8)(f1 * (r4 - r3)) + r3; }
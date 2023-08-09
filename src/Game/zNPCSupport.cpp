#include "zNPCSupport.h"

#include "xString.h"
#include "xutil.h"
#include "xstransvc.h"
#include "xSnd.h"
#include "zNPCHazard.h"
#include "zNPCGlyph.h"
#include "zNPCSupplement.h"
#include "zScene.h"
#include "zEvent.h"
#include "zBase.h"
#include "zNPCMgr.h"
#include "zNPCTypeCommon.h"
#include "zRenderState.h"

#define MAX_FIREWORK 32

static NPCWidget g_npc_widgets[NPC_WIDGE_NOMORE];
static U32 g_hash_uiwidgets[NPC_WIDGE_NOMORE] = {};
static const char* g_strz_uiwidgets[NPC_WIDGE_NOMORE] = {
    "MNU4 NPCTALK"
};

static U32 sNPCSndFx[eNPCSnd_Total] = {};
static U32 sNPCSndID[eNPCSnd_Total] = {};
static F32 sNPCSndFxVolume[eNPCSnd_Total] = {};

static Firework g_fireworks[MAX_FIREWORK];

F32 Firework::acc_thrust = 15.0f;
F32 Firework::acc_gravity = -10.0f;

S32 g_pc_playerInvisible;

void NPCSupport_Startup()
{
    zNPCHazard_Startup();
    zNPCGlyph_Startup();
    NPCWidget_Startup();
    NPCSupplement_Startup();
}

void NPCSupport_Shutdown()
{
    zNPCHazard_Shutdown();
    zNPCGlyph_Shutdown();
    NPCWidget_Shutdown();
    NPCSupplement_Shutdown();
}

void NPCSupport_ScenePrepare()
{
    zNPCHazard_ScenePrepare();
    zNPCGlyph_ScenePrepare();
    NPCWidget_ScenePrepare();
    NPCSupplement_ScenePrepare();
    Firework_ScenePrepare();
    NPCC_ForceTalkOk();
}

void NPCSupport_SceneFinish()
{
    zNPCHazard_SceneFinish();
    zNPCGlyph_SceneFinish();
    NPCWidget_SceneFinish();
    NPCSupplement_SceneFinish();
    Firework_SceneFinish();
}

void NPCSupport_ScenePostInit()
{
    zNPCHazard_ScenePostInit();
    zNPCGlyph_ScenePostInit();
    NPCWidget_ScenePostInit();
    NPCSupplement_ScenePostInit();
    zNPC_SNDInit();
}

void NPCSupport_SceneReset()
{
    zNPCHazard_SceneReset();
    zNPCGlyph_SceneReset();
    NPCWidget_SceneReset();
    NPCSupplement_SceneReset();
    Firework_SceneReset(0);
}

void NPCSupport_Timestep(F32 dt)
{
    zNPCGlyph_Timestep(dt);
    zNPCHazard_Timestep(dt);
    NPCSupplement_Timestep(dt);
    Firework_Timestep(dt);
}

void NPCWidget_Startup()
{
    for (S32 i = 0; i < NPC_WIDGE_NOMORE; i++) {
        g_hash_uiwidgets[i] = xStrHash(g_strz_uiwidgets[i]);
    }
}

void NPCWidget_Shutdown()
{
}

void NPCWidget_ScenePrepare()
{
}

void NPCWidget_SceneFinish()
{
    NPCWidget_SceneReset();
}

void NPCWidget_SceneReset()
{
    for (S32 i = 0; i < NPC_WIDGE_NOMORE; i++) {
        g_npc_widgets[i].Reset();
    }
}

void NPCWidget_ScenePostInit()
{
    for (S32 i = 0; i < NPC_WIDGE_NOMORE; i++) {
        g_npc_widgets[i].Init((en_NPC_UI_WIDGETS)i);
    }
}

NPCWidget* NPCWidget_Find(en_NPC_UI_WIDGETS which)
{
    return &g_npc_widgets[which];
}

void NPCWidget::Init(en_NPC_UI_WIDGETS r4)
{
    this->idxID = r4;
    this->base_widge = zSceneFindObject(g_hash_uiwidgets[this->idxID]);
}

void NPCWidget::Reset()
{
}

S32 NPCWidget::On(const zNPCCommon* npc, S32 theman)
{
    if (!theman && !this->NPCIsTheLocker(npc)) {
        if (this->IsLocked() || !this->Lock(npc)) {
            return 0;
        }
    }

    if (this->IsVisible()) {
        return 1;
    }

    zEntEvent(this->base_widge, eEventUIFocusOn_Select);
    zEntEvent(this->base_widge, eEventVisible);
    return 1;
}

S32 NPCWidget::Off(const zNPCCommon* npc, S32 theman)
{
    if (!theman && !this->NPCIsTheLocker(npc)) {
        return 0;
    }

    if (npc) {
        this->Unlock(npc);
    }

    zEntEvent(this->base_widge, eEventInvisible);
    zEntEvent(this->base_widge, eEventUIFocusOff_Unselect);
    return 1;
}

S32 NPCWidget::IsVisible()
{
    if (!this->base_widge) {
        return 0;
    }

    if (this->base_widge->baseType != eBaseTypeUIFont) {
        return 0;
    }

    return xEntIsVisible((xEnt*)this->base_widge);
}

S32 NPCWidget::Lock(const zNPCCommon* npc)
{
    if (this->npc_ownerlock && npc != this->npc_ownerlock) {
        return 0;
    }

    this->npc_ownerlock = npc;
    return 1;
}

S32 NPCWidget::Unlock(const zNPCCommon* npc)
{
    if (!this->npc_ownerlock) {
        return 1;
    }

    if (npc != this->npc_ownerlock) {
        return 0;
    }

    this->npc_ownerlock = NULL;
    return 1;
}

S32 NPCWidget::NPCIsTheLocker(const zNPCCommon* npc_lock)
{
    if (!this->IsLocked()) {
        return 0;
    }

    if (npc_lock == this->npc_ownerlock) {
        return 1;
    }

    return 0;
}

void NPCTarget::TargetSet(xEnt* r4, S32 r5)
{
    this->ent_target = r4;

    if (r4 && r5) {
        this->typ_target = NPC_TGT_PLYR;
    } else if (r4) {
        this->typ_target = NPC_TGT_ENT;
    } else {
        this->typ_target = NPC_TGT_NONE;
    }
}

void NPCTarget::TargetClear()
{
    this->ent_target = NULL;
    this->typ_target = NPC_TGT_NONE;
}

S32 NPCTarget::FindNearest(S32 flg_consider, xBase* skipme, xVec3* from, F32 dst_max) NONMATCH("https://decomp.me/scratch/wWBRW")
{
    S32 found = 0;
    st_XORDEREDARRAY* npclist;
    F32 ds2_best;
    zNPCCommon* npc, *npc_best;
    xVec3 vec = {};
    F32 fv;
    S32 i, ntyp;

    npc_best = NULL;
    ds2_best = (dst_max < 0.0f) ? HUGE : SQ(dst_max);

    if (flg_consider & 0x1) {
        this->TargetSet(&globals.player.ent, 1);

        if (from) {
            xVec3Sub(&vec, xEntGetPos(&globals.player.ent), from);
            ds2_best = xVec3Length2(&vec);
        }
    }

    if (from && (flg_consider & 0x1E)) {
        npclist = zNPCMgr_GetNPCList();

        for (i = 0; i < npclist->cnt; i++) {
            npc = (zNPCCommon*)npclist->list[i];
            ntyp = npc->SelfType();

            if (npc == skipme) continue;

            if (((ntyp & 0xFFFFFF00) != 'NTT\0' || (flg_consider & 0x4)) &&
                ((ntyp & 0xFFFFFF00) != 'NTR\0' || (flg_consider & 0x2)) &&
                ((ntyp & 0xFFFFFF00) != 'NTF\0' || (flg_consider & 0x8)) &&
                ((ntyp & 0xFFFFFF00) != 'NTA\0' || (flg_consider & 0x10))) {
                if (npc->IsAlive()) {
                    xVec3Sub(&vec, xEntGetPos(npc), from);
                    if (flg_consider & 0x80) {
                        vec.y = 0.0f;
                    }

                    fv = xVec3Length2(&vec);
                    if (fv > ds2_best) continue;
                    
                    ds2_best = fv;
                    npc_best = npc;
                    found = 1;
                }
            }
        }

        if (found) {
            this->TargetSet(npc_best, 0);
        }
    }

    return found;
}

void NPCTarget::PosGet(xVec3* pos)
{
    switch (this->typ_target) {
    case NPC_TGT_NONE:
        break;
    case NPC_TGT_PLYR:
    case NPC_TGT_ENT:
    case NPC_TGT_BASE:
        NPCC_pos_ofBase(this->bas_target, pos);
        break;
    case NPC_TGT_POS:
        xVec3Copy(pos, &this->pos_target);
        break;
    case NPC_TGT_MVPT:
        xVec3Copy(pos, zMovePointGetPos(this->nav_target));
        break;
    }
}

S32 NPCTarget::InCylinder(xVec3* from, F32 rad, F32 hyt, F32 off)
{
    S32 inrange = 1;

    xVec3 vec = {};
    this->PosGet(&vec);
    xVec3SubFrom(&vec, from);

    F32 upper = hyt + off;
    F32 lower = upper - hyt;

    if (vec.y > upper) {
        inrange = 0;
    } else if (vec.y < lower) {
        inrange = 0;
    } else if (xVec3Length2(&vec) > SQ(rad)) {
        inrange = 0;
    }

    return inrange;
}

S32 NPCTarget::IsDead()
{
    S32 dead = 0;

    switch (this->typ_target) {
    case NPC_TGT_PLYR:
        if (globals.player.Health < 1) {
            dead = 1;
        }
        break;
    case NPC_TGT_ENT:
        if (this->ent_target->baseType == eBaseTypeNPC) {
            if (!((zNPCCommon*)this->ent_target)->IsAlive()) {
                dead = 1;
            }
        }
        break;
    case NPC_TGT_BASE:
        break;
    }

    return dead;
}

void NPCLaser::Render(xVec3* pos_src, xVec3* pos_tgt) WIP NONMATCH("https://decomp.me/scratch/lNgTd")
{
    xVec3 var_70;
    xVec3Copy(&var_70, pos_src);

    xVec3 var_7C;
    xVec3Copy(&var_7C, pos_tgt);

    xVec3 var_88;
    xVec3Sub(&var_88, &var_7C, &var_70);
    xVec3Normalize(&var_88, &var_88);

    xVec3 var_94;
    xVec3Cross(&var_94, &globals.camera.mat.at, &var_88);

    F32 f1 = xVec3Length2(&var_94);
    if (f1 < 0.00001f) {
        xVec3Copy(&var_94, &g_X3);
    } else {
        xVec3SMulBy(&var_94, 1.0f / xsqrt(f1));
    }

    xVec3 var_A0;
    xVec3Cross(&var_A0, &var_94, &var_88);

    S32 i;
    
    static RwIm3DVertex laser_vtxbuf[2][14];
    RwIm3DVertex* vtx_horz = laser_vtxbuf[0];
    RwIm3DVertex* vtx_vert = laser_vtxbuf[1];

    for (i = 0; i <= 6; i++) {
        F32 rat = (F32)i / 6.0f;
        F32 f29 = LERP(rat, this->radius[0], this->radius[1]);

        xVec3 var_AC;
        var_AC.x = LERP(rat, var_70.x, var_7C.x);
        var_AC.y = LERP(rat, var_70.y, var_7C.y);
        var_AC.z = LERP(rat, var_70.z, var_7C.z);

        U8 r22 = LERP(rat, this->rgba[0].red, this->rgba[1].red);
        U8 r23 = LERP(rat, this->rgba[0].green, this->rgba[1].green);
        U8 r24 = LERP(rat, this->rgba[0].blue, this->rgba[1].blue);
        U8 r25 = LERP(rat, this->rgba[0].alpha, this->rgba[1].alpha);

        F32 u = 1.0f - rat + this->uv_base[0];
        F32 v = 1.0f - rat + this->uv_base[1];

        while (u > 1.0f) u -= 1.0f;
        while (v > 1.0f) v -= 1.0f;

        xVec3 var_B8;
        
        xVec3SMul(&var_B8, &var_94, f29);
        xVec3AddTo(&var_B8, &var_AC);
        RwIm3DVertexSetPos(&vtx_horz[0], var_B8.x, var_B8.y, var_B8.z);
        RwIm3DVertexSetRGBA(&vtx_horz[0], r22, r23, r24, r25);
        RwIm3DVertexSetU(&vtx_horz[0], 0.0f);
        RwIm3DVertexSetV(&vtx_horz[0], v);

        xVec3SMul(&var_B8, &var_94, -f29);
        xVec3AddTo(&var_B8, &var_AC);
        RwIm3DVertexSetPos(&vtx_horz[1], var_B8.x, var_B8.y, var_B8.z);
        RwIm3DVertexSetRGBA(&vtx_horz[1], r22, r23, r24, r25);
        RwIm3DVertexSetU(&vtx_horz[1], 1.0f);
        RwIm3DVertexSetV(&vtx_horz[1], v);

        vtx_horz += 2;

        xVec3SMul(&var_B8, &var_A0, f29);
        xVec3AddTo(&var_B8, &var_AC);
        RwIm3DVertexSetPos(&vtx_vert[0], var_B8.x, var_B8.y, var_B8.z);
        RwIm3DVertexSetRGBA(&vtx_vert[0], r22, r23, r24, r25);
        RwIm3DVertexSetU(&vtx_vert[0], 0.0f);
        RwIm3DVertexSetV(&vtx_vert[0], v);

        xVec3SMul(&var_B8, &var_A0, -f29);
        xVec3AddTo(&var_B8, &var_AC);
        RwIm3DVertexSetPos(&vtx_vert[1], var_B8.x, var_B8.y, var_B8.z);
        RwIm3DVertexSetRGBA(&vtx_vert[1], r22, r23, r24, r25);
        RwIm3DVertexSetU(&vtx_vert[1], 1.0f);
        RwIm3DVertexSetV(&vtx_vert[1], v);

        vtx_vert += 2;
    }

    SDRenderState old_rendstat = zRenderStateCurrent();
    if (old_rendstat == SDRS_Unknown) {
        old_rendstat = SDRS_Default;
    }

    zRenderState(SDRS_NPCVisual);

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)this->rast_laser);
    RwIm3DTransform(laser_vtxbuf[0], 14, NULL, rwIM3D_VERTEXXYZ | rwIM3D_VERTEXRGBA | rwIM3D_VERTEXUV);
    RwIm3DRenderPrimitive(rwPRIMTYPETRISTRIP);
    RwIm3DEnd();

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)this->rast_laser);
    RwIm3DTransform(laser_vtxbuf[1], 14, NULL, rwIM3D_VERTEXXYZ | rwIM3D_VERTEXRGBA | rwIM3D_VERTEXUV);
    RwIm3DRenderPrimitive(rwPRIMTYPETRISTRIP);
    RwIm3DEnd();

    zRenderState(old_rendstat);
}

void NPCCone::RenderCone(xVec3* pos_tiptop, xVec3* pos_botcenter) NONMATCH("https://decomp.me/scratch/G9pbs")
{
    RwRGBA rgba_top = this->rgba_top;
    RwRGBA rgba_bot = this->rgba_bot;
    xVec3 pos_top = *pos_tiptop;
    xVec3 pos_bot = *pos_botcenter;
    F32 f29 = this->uv_tip[0] + 0.5f * this->uv_slice[0];
    F32 f28 = this->uv_tip[1];
    F32 f31 = this->uv_tip[0] + this->uv_slice[0];
    F32 f30 = this->uv_tip[1] + this->uv_slice[1];

    void* mem = xMemPushTemp(10 * sizeof(RwIm3DVertex));
    if (!mem) {
        return;
    }

    memset(mem, 0, 10 * sizeof(RwIm3DVertex));

    RwIm3DVertex* vert_list = (RwIm3DVertex*)mem;
    RwIm3DVertex* vtx = vert_list + 1;

    RwIm3DVertexSetPos(&vert_list[0], pos_top.x, pos_top.y, pos_top.z);
    RwIm3DVertexSetRGBA(&vert_list[0], rgba_top.red, rgba_top.green, rgba_top.blue, rgba_top.alpha);
    RwIm3DVertexSetU(&vert_list[0], f29);
    RwIm3DVertexSetV(&vert_list[0], f28);

    for (S32 i = 0; i < 8; i++) {
        F32 ang_seg = i * PI/4;
        F32 f29 = isin(ang_seg);
        F32 f1 = icos(ang_seg);
        
        xVec3 var_A0;
        var_A0.x = f29;
        var_A0.y = 0.0f;
        var_A0.z = f1;
        var_A0 *= this->rad_cone;
        var_A0 += pos_bot;

        RwIm3DVertexSetPos(vtx, var_A0.x, var_A0.y, var_A0.z);
        RwIm3DVertexSetRGBA(vtx, rgba_bot.red, rgba_bot.green, rgba_bot.blue, rgba_bot.alpha);

        F32 f0 = 1/8.0f * i;
        
        RwIm3DVertexSetU(vtx, f31 + f0);
        RwIm3DVertexSetV(vtx, f30);

        vtx++;
    }

    *vtx = vert_list[1];
    RwIm3DVertexSetU(vtx, f31 + this->uv_slice[0]);
    RwIm3DVertexSetV(vtx, f30);

    SDRenderState old_rendstat = zRenderStateCurrent();
    if (old_rendstat == SDRS_Unknown) {
        old_rendstat = SDRS_Default;
    }

    zRenderState(SDRS_NPCVisual);

    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)this->rast_cone);
    RwIm3DTransform(vert_list, 10, NULL, rwIM3D_VERTEXXYZ | rwIM3D_VERTEXRGBA | rwIM3D_VERTEXUV);
    RwIm3DRenderPrimitive(rwPRIMTYPETRIFAN);
    RwIm3DEnd();

    zRenderState(old_rendstat);

    xMemPopTemp(mem);
}

void NPCBlinker::Reset()
{
    this->tmr_uvcell = -1.0f;
    this->idx_uvcell = 0;
}

void NPCBlinker::Update(F32 dt, F32 ratio, F32 tym_slow, F32 tym_fast)
{
    if (this->tmr_uvcell < 0.0f) {
        this->idx_uvcell++;
        if (this->idx_uvcell > 3) {
            this->idx_uvcell = 0;
        }

        this->tmr_uvcell = LERP(SQ(ratio), tym_slow, tym_fast);
    }

    this->tmr_uvcell = xmax(-1.0f, this->tmr_uvcell - dt);
}

void NPCBlinker::IndexToUVCoord(S32 r4, F32* r5, F32* r6) NONMATCH("https://decomp.me/scratch/4zwmp")
{
    r5[0] = 0.5f * (r4 % 2);
    r5[1] = 0.5f * ((r4 - r4 % 2) / 2);
    r6[0] = 0.5f + r5[0];
    r6[1] = 0.5f + r5[1];
}

void NPCBlinker::Render(const xVec3* pos_blink, F32 rad_blink, const RwRaster* rast_blink) NONMATCH("https://decomp.me/scratch/K6Bre")
{
    static F32 dst_toCamMax = SQ(25.0f);

    if (NPCC_ds2_toCam(pos_blink, NULL) > dst_toCamMax) {
        return;
    }

    RwRGBA rgba = { 0xFF, 0xFF, 0xFF, 0xFF };

    xMat3x3 mat;
    xMat3x3LookAt(&mat, pos_blink, &globals.camera.mat.pos);

    /*
    xVec3 dir_card;
    xVec3 dir_perp;
    */

    xVec3 var_9C;

    xVec3 var_A8;
    xVec3Add(&var_A8, &mat.at, &mat.right);
    xVec3Normalize(&var_A8, &var_A8);

    xVec3 var_B4;
    xVec3Sub(&var_B4, &mat.at, &mat.right);
    xVec3Normalize(&var_B4, &var_B4);

    xVec3Copy(&var_9C, &mat.up);

    xVec3 var_C0;
    xVec3Copy(&var_C0, pos_blink);

    xVec3 var_CC;
    xVec3Copy(&var_CC, pos_blink);

    xVec3AddScaled(&var_C0, &var_9C, rad_blink);
    xVec3AddScaled(&var_CC, &var_9C, -rad_blink);

    F32 var_11C[2];
    F32 var_124[2];
    this->IndexToUVCoord(this->idx_uvcell, var_11C, var_124);

    U8 cr = rgba.red;
    U8 cg = rgba.green;
    U8 cb = rgba.blue;
    U8 ca = rgba.alpha;

    static RwIm3DVertex blink_vtxbuf[2][14];

    S32 i;
    RwIm3DVertex* vtx_horz = blink_vtxbuf[0];
    RwIm3DVertex* vtx_vert = blink_vtxbuf[1];

    for (i = 0; i <= 6; i++) {
        F32 rat = i / 6.0f;

        xVec3 var_D8;
        var_D8.x = LERP(rat, var_C0.x, var_CC.x);
        var_D8.y = LERP(rat, var_C0.y, var_CC.y);
        var_D8.z = LERP(rat, var_C0.z, var_CC.z);

        F32 f29 = LERP(rat, var_11C[1], var_124[1]);

        xVec3 var_E4;
        
        var_E4 = var_A8 * rad_blink;
        var_E4 += var_D8;

        RwIm3DVertexSetPos(vtx_horz, var_E4.x, var_E4.y, var_E4.z);
        RwIm3DVertexSetRGBA(vtx_horz, cr, cg, cb, ca);
        RwIm3DVertexSetU(vtx_horz, var_11C[0]);
        RwIm3DVertexSetV(vtx_horz, f29);

        var_E4 = var_A8 * -rad_blink;
        var_E4 += var_D8;

        RwIm3DVertexSetPos(vtx_horz + 1, var_E4.x, var_E4.y, var_E4.z);
        RwIm3DVertexSetRGBA(vtx_horz + 1, cr, cg, cb, ca);
        RwIm3DVertexSetU(vtx_horz + 1, var_124[0]);
        RwIm3DVertexSetV(vtx_horz + 1, f29);

        vtx_horz += 2;

        var_E4 = var_B4 * -rad_blink;
        var_E4 += var_D8;

        RwIm3DVertexSetPos(vtx_vert, var_E4.x, var_E4.y, var_E4.z);
        RwIm3DVertexSetRGBA(vtx_vert, cr, cg, cb, ca);
        RwIm3DVertexSetU(vtx_vert, var_11C[0]);
        RwIm3DVertexSetV(vtx_vert, f29);

        var_E4 = var_B4 * rad_blink;
        var_E4 += var_D8;

        RwIm3DVertexSetPos(vtx_vert + 1, var_E4.x, var_E4.y, var_E4.z);
        RwIm3DVertexSetRGBA(vtx_vert + 1, cr, cg, cb, ca);
        RwIm3DVertexSetU(vtx_vert + 1, var_124[0]);
        RwIm3DVertexSetV(vtx_vert + 1, f29);

        vtx_vert += 2;
    }

    SDRenderState old_rendstat = zRenderStateCurrent();
    if (old_rendstat == SDRS_Unknown) {
        old_rendstat = SDRS_Default;
    }

    zRenderState(SDRS_NPCVisual);

    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);
    
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)rast_blink);
    RwIm3DTransform(blink_vtxbuf[0], 14, NULL, rwIM3D_VERTEXXYZ | rwIM3D_VERTEXRGBA | rwIM3D_VERTEXUV);
    RwIm3DRenderPrimitive(rwPRIMTYPETRISTRIP);
    RwIm3DEnd();

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)rast_blink);
    RwIm3DTransform(blink_vtxbuf[1], 14, NULL, rwIM3D_VERTEXXYZ | rwIM3D_VERTEXRGBA | rwIM3D_VERTEXUV);
    RwIm3DRenderPrimitive(rwPRIMTYPETRISTRIP);
    RwIm3DEnd();

    zRenderState(old_rendstat);
}

void Firework_Release(Firework* f)
{
    f->Cleanup();
    f->fwstate = FW_STAT_UNUSED;
}

void Firework_ScenePrepare()
{
    NPAR_PartySetup(NPAR_TYP_FIREWORKS, NULL, NULL);
    Firework_SceneReset(FALSE);
}

void Firework_SceneFinish()
{
    Firework_SceneReset(TRUE);
}

void Firework_SceneReset(S32 r3)
{
    for (S32 i = 0; i < MAX_FIREWORK; i++) {
        if (r3 && g_fireworks[i].fwstate != FW_STAT_UNUSED) {
            Firework_Release(&g_fireworks[i]);
        }
        g_fireworks[i].fwstate = FW_STAT_UNUSED;
    }
}

void Firework_Timestep(F32 dt)
{
    for (S32 i = 0; i < MAX_FIREWORK; i++) {
        if (g_fireworks[i].fwstate != FW_STAT_UNUSED &&
            g_fireworks[i].fwstate != FW_STAT_READY &&
            (g_fireworks[i].flg_firework & 0x4)) {
            if (g_fireworks[i].fwstate == FW_STAT_DONE) {
                Firework_Release(&g_fireworks[i]);
            } else {
                g_fireworks[i].Update(dt);
                g_fireworks[i].flg_firework &= ~0x2;
            }
        }
    }
}

void Firework::Cleanup()
{
}

void Firework::Update(F32 dt)
{
    switch (this->fwstate) {
    case FW_STAT_FLIGHT:
        this->FlyFlyFly(dt);
        if (this->tmr_remain < 0.0f) {
            this->fwstate = FW_STAT_BOOM;
        }
        break;
    case FW_STAT_BOOM:
        this->Detonate();
        this->fwstate = FW_STAT_DONE;
        break;
    case FW_STAT_DONE:
        break;
    }

    this->tmr_remain = xmax(-1.0f, this->tmr_remain - dt);
}

void Firework::FlyFlyFly(F32 dt) NONMATCH("https://decomp.me/scratch/6hPC3")
{
    F32 pam_life = 1.0f - xclamp(this->tmr_remain / this->tym_lifespan, 0.0f, 1.0f);
    if (pam_life < 0.75f) {
        xVec3 dir_trav = this->vel;
        dir_trav.normalize();

        this->vel += dir_trav * (Firework::acc_thrust * dt);
    }
    
    this->vel += g_NY3 * (Firework::acc_gravity * dt);

    NPAR_EmitFWExhaust(&this->pos, &g_O3);
}

void Firework::Detonate()
{
    xUtil_yesno(0.25f);
}

void NPCC_ang_toXZDir(F32 angle, xVec3* dir)
{
    dir->x = isin(angle);
    dir->y = 0.0f;
    dir->z = icos(angle);
}

F32 NPCC_dir_toXZAng(const xVec3* dir)
{
    return xatan2(dir->x, dir->z);
}

F32 NPCC_aimMiss(xVec3* dir_aim, xVec3* pos_src, xVec3* pos_tgt, F32 dst_miss, xVec3* pos_miss)
{
    F32 dst_toTgt = NPCC_aimVary(dir_aim, pos_src, pos_tgt, dst_miss, 0x8, pos_miss);
    return dst_toTgt;
}

F32 NPCC_aimVary(xVec3* dir_aim, xVec3* pos_src, xVec3* pos_tgt, F32 dst_vary, S32 flg_vary, xVec3* pos_aimPoint) NONMATCH("https://decomp.me/scratch/D1gIj")
{
    F32 dst_toFake = 0.0f;
    xVec3 dir_left = {};
    xVec3 dir_toFake = {};
    xVec3 dir_toReal = {};
    xVec3 vec_offset = {};
    xVec3 pos_tgtFake = {};

    xVec3Sub(&dir_toReal, pos_tgt, pos_src);

    if (flg_vary & 0x10) {
        dir_toReal.y = 0.0f;
    }

    F32 mag_vary = xVec3Length(&dir_toReal);
    if (mag_vary < 0.001f) {
        if (mag_vary > 0.0f) {
            xVec3SMulBy(&dir_toReal, 100000.0f);
            xVec3Normalize(&dir_toReal, &dir_toReal);
        } else {
            xVec3Copy(dir_aim, &g_X3);
        }

        if (pos_aimPoint) {
            xVec3Copy(pos_aimPoint, pos_tgt);
        }

        return mag_vary;
    }

    xVec3SMulBy(&dir_toReal, 1.0f / mag_vary);
    xVec3Cross(&dir_left, &g_Y3, &dir_toReal);

    F32 mag_updown;
    if (flg_vary & 0x8) {
        mag_updown = dst_vary;
    } else {
        mag_updown = 2.0f * (xurand() - 0.5f) * dst_vary;

        F32 fv;
        if ((flg_vary & 0x1) && (flg_vary & 0x2)) {
            fv = 2.0f * (xurand() - 0.5f);
        } else if (flg_vary == 0x1) {
            fv = xurand();
        } else if (flg_vary == 0x2) {
            fv = -xurand();
        } else {
            fv = 0.0f;
        }

        dst_toFake = fv * dst_vary;
    }

    xVec3AddScaled(&vec_offset, &dir_left, mag_updown);
    xVec3AddScaled(&vec_offset, &g_Y3, dst_toFake);
    xVec3Add(&pos_tgtFake, pos_tgt, &vec_offset);
    xVec3Sub(&dir_toFake, &pos_tgtFake, pos_src);
    
    F32 f31 = xVec3Normalize(&dir_toFake, dir_aim);

    if (pos_aimPoint) {
        xVec3Copy(pos_aimPoint, &pos_tgtFake);
    }

    return (flg_vary & 0x4) ? f31 : mag_vary;
}

S32 NPCC_chk_hitPlyr(xBound* bnd, xCollis* collide)
{
    return NPCC_chk_hitEnt(&globals.player.ent, bnd, collide);
}

S32 NPCC_chk_hitEnt(xEnt* tgt, xBound* bnd, xCollis* collide) NONMATCH("https://decomp.me/scratch/Bf1rk")
{
    S32 hittgt = 0;
    xCollis* colrec;
    xCollis lcl_collide = {};

    colrec = collide ? collide : &lcl_collide;
    colrec->optr = tgt;
    colrec->oid = tgt->id;

    if (collide) {
        colrec->flags = k_HIT_0xF00 | k_HIT_CALC_HDNG;
    } else {
        colrec->flags = 0;
    }

    xQuickCullForEverything(&bnd->qcd);
    xBoundHitsBound(bnd, &tgt->bound, colrec);

    if (colrec->flags & k_HIT_IT) {
        hittgt = 1;
    }

    return hittgt;
}

S32 NPCC_LineHitsBound(xVec3* a, xVec3* b, xBound* bnd, xCollis* callers_colrec) NONMATCH("https://decomp.me/scratch/0MDNL")
{
    xRay3 ray;
    xCollis local_colrec;
    xCollis* colrec = &local_colrec;

    if (callers_colrec) {
        colrec = callers_colrec;
    }

    xVec3 var_A8;
    xVec3Sub(&var_A8, b, a);
    
    F32 len = xVec3Length(&var_A8);
    if (len < 0.001f) {
        len = 0.001f;
    }

    xVec3Copy(&ray.origin, a);
    xVec3SMul(&ray.dir, &var_A8, 1.0f / len);

    ray.min_t = 0.1f;
    ray.max_t = len;
    ray.flags = XRAY3_USE_MIN | XRAY3_USE_MAX;

    xRayHitsBound(&ray, bnd, colrec);

    return colrec->flags & k_HIT_IT;
}

S32 NPCC_bnd_ofBase(xBase* tgt, xBound* bnd)
{
    S32 known = 1;

    switch (tgt->baseType) {
    case eBaseTypeCamera:
    case eBaseTypeDoor:
    case eBaseTypeVolume:
    case eBaseTypeEGenerator:
        known = 0;
        break;
    case eBaseTypePlayer:
    case eBaseTypePickup:
    case eBaseTypePlatform:
    case eBaseTypeStatic:
    case eBaseTypeDynamic:
    case eBaseTypeBubble:
    case eBaseTypePendulum:
    case eBaseTypeHangable:
    case eBaseTypeButton:
    case eBaseTypeProjectile:
    case eBaseTypeDestructObj:
    case eBaseTypeNPC:
    case eBaseTypeBoulder:
        *bnd = ((xEnt*)tgt)->bound;
        break;
    case eBaseTypeCruiseBubble:
        break;
    default:
        known = 0;
        break;
    }

    return known;
}

S32 NPCC_pos_ofBase(xBase* tgt, xVec3* pos)
{
    S32 known = 1;

    switch (tgt->baseType) {
    case eBaseTypeCamera:
        xVec3Copy(pos, &globals.camera.mat.pos);
        break;
    case eBaseTypeDoor:
    case eBaseTypeVolume:
    case eBaseTypeEGenerator:
        known = 0;
        break;
    case eBaseTypePlayer:
    case eBaseTypePickup:
    case eBaseTypePlatform:
    case eBaseTypeStatic:
    case eBaseTypeDynamic:
    case eBaseTypeBubble:
    case eBaseTypePendulum:
    case eBaseTypeHangable:
    case eBaseTypeButton:
    case eBaseTypeProjectile:
    case eBaseTypeDestructObj:
    case eBaseTypeNPC:
    case eBaseTypeBoulder:
        xVec3Copy(pos, xEntGetPos((xEnt*)tgt));
        break;
    case eBaseTypeCruiseBubble:
        known = 0;
        break;
    default:
        known = 0;
        break;
    }

    return known;
}

void NPCC_xBoundAway(xBound* bnd)
{
    if (bnd->type == k_XBOUNDTYPE_SPHERE) {
        bnd->sph.center.y -= 1000000.0f;
    } else if (bnd->type == k_XBOUNDTYPE_BOX) {
        bnd->box.center.y -= 1000000.0f;
    }
}

void NPCC_xBoundBack(xBound* bnd)
{
    if (bnd->type == k_XBOUNDTYPE_SPHERE) {
        bnd->sph.center.y += 1000000.0f;
    } else if (bnd->type == k_XBOUNDTYPE_BOX) {
        bnd->box.center.y += 1000000.0f;
    }
}

S32 NPCC_HaveLOSToPos(xVec3* pos_src, xVec3* pos_tgt, F32 dst_max, xBase* tgt, xCollis* colCallers) NONMATCH("https://decomp.me/scratch/LyDtk")
{
    S32 result;
    xRay3 ray = {};
    xScene* xscn = globals.sceneCur;
    xCollis* colrec;

    if (colCallers) {
        colrec = colCallers;
    } else {
        static xCollis localCollis = { k_HIT_0xF00 | k_HIT_CALC_HDNG };

        memset(&localCollis, 0, sizeof(xCollis));
        localCollis.flags = k_HIT_0xF00 | k_HIT_CALC_HDNG;

        colrec = &localCollis;
    }
    
    ray.min_t = 0.0f;
    ray.max_t = dst_max;

    xVec3Sub(&ray.dir, pos_tgt, pos_src);
    xVec3Normalize(&ray.dir, &ray.dir);
    xVec3Copy(&ray.origin, pos_src);

    ray.flags = XRAY3_USE_MIN | XRAY3_USE_MAX;

    xRayHitsScene(xscn, &ray, colrec);

    if (!(colrec->flags & k_HIT_IT)) {
        result = 1;
    } else if (colrec->dist > dst_max) {
        result = 1;
    } else if (tgt && colrec->oid != 0) {
        if (tgt->id == colrec->oid) {
            result = 1;
        } else {
            result = 0;
        }
    } else {
        result = 0;
    }

    return result;
}

F32 NPCC_DstSqPlyrToPos(xVec3* pos)
{
    return NPCC_DstSq(pos, xEntGetPos(&globals.player.ent), NULL);
}

F32 NPCC_ds2_toCam(const xVec3* pos_from, xVec3* delta)
{
    xVec3 delt = {};
    xVec3Sub(&delt, &globals.camera.mat.pos, pos_from);

    F32 f31 = xVec3Length2(&delt);

    if (delta) {
        xVec3Copy(delta, &delt);
    }

    return f31;
}

void NPCC_Bounce(xVec3* vec_input, xVec3* vec_anti, F32 elastic) NONMATCH("https://decomp.me/scratch/vf41s")
{
    if (vec_input->x * vec_anti->x < 0.0f) {
        vec_input->x *= -1.0f;
    }

    if (vec_input->y * vec_anti->y < 0.0f) {
        vec_input->y *= -1.0f;
    }

    if (vec_input->z * vec_anti->z < 0.0f) {
        vec_input->z *= -1.0f;
    }

    xVec3SMulBy(vec_input, elastic);
}

void NPCC_rotHPB(xMat3x3* mat, F32 heading, F32 pitch, F32 bank)
{
    //xVec3 axis; // unused
    xMat3x3 mat_rot = {};

    xMat3x3RotZ(mat, bank);

    xMat3x3RotX(&mat_rot, -pitch);
    xMat3x3Mul(mat, mat, &mat_rot);

    xMat3x3RotY(&mat_rot, heading);
    xMat3x3Mul(mat, mat, &mat_rot);
}

F32 NPCC_TmrCycle(F32* tmr, F32 dt, F32 interval)
{
    if (*tmr < 0.0f) {
        *tmr = 0.0f;
    }

    F32 parameterized = *tmr / interval;

    *tmr += dt;
    if (*tmr > interval) {
        *tmr = xfmod(*tmr, interval);
    }

    return parameterized;
}

void NPCC_MakePerp(xVec3* dir_perp, const xVec3* dir_axis)
{
    dir_perp->x = dir_axis->y - dir_axis->z;
    dir_perp->y = dir_axis->z - dir_axis->x;
    dir_perp->z = dir_axis->x - dir_axis->y;

    xVec3Normalize(dir_perp, dir_perp);
}

void NPCC_MakeArbPlane(const xVec3* dir_norm, xVec3* at, xVec3* rt)
{
    NPCC_MakePerp(at, dir_norm);
    xVec3Cross(rt, at, dir_norm);
}

RwTexture* NPCC_FindRWTexture(const char* txtrname)
{
    return (RwTexture*)xSTFindAsset(xStrHash(txtrname), NULL);
}

RwTexture* NPCC_FindRWTexture(U32 hashid)
{
    return (RwTexture*)xSTFindAsset(hashid, NULL);
}

RwRaster* NPCC_FindRWRaster(const char* txtrname)
{
    RwTexture* txtr = NPCC_FindRWTexture(txtrname);
    if (txtr) {
        return RwTextureGetRaster(txtr);
    }

    return NULL;
}

RwRaster* NPCC_FindRWRaster(RwTexture* txtr)
{
    if (txtr) {
        return RwTextureGetRaster(txtr);
    }

    return NULL;
}

void NPCC_GenSmooth(xVec3** pos_base, xVec3** pos_mid) WIP NONMATCH("https://decomp.me/scratch/1MplX")
{
    static F32 prepute[4][4];
    static const F32 yews[4] = { 0.25f, 0.5f, 0.75f, 1.0f };
    static S32 init = 0;

    S32 i;

    if (!init) {
        init = 1;

        for (i = 0; i < 4; i++) {
            F32 u = yews[i];
            F32 u2 = u * u;
            F32 u3 = u * u2;

            prepute[i][0] = u2 + -0.5f * u3 + -0.5f * u;
            prepute[i][1] = -2.5f * u2 + 1.5f * u3 + 1.0f;
            prepute[i][2] = 2.0f * u2 + -1.5f * u3 + 0.5f * u;
            prepute[i][3] = -0.5f * u2 + 0.5f * u3;
        }
    }

    for (i = 0; i < 4; i++) {
        xVec3SMul(pos_mid[i], pos_base[0], prepute[i][0]);
        xVec3AddScaled(pos_mid[i], pos_base[1], prepute[i][1]);
        xVec3AddScaled(pos_mid[i], pos_base[2], prepute[i][2]);
        xVec3AddScaled(pos_mid[i], pos_base[3], prepute[i][3]);
    }
}

void zNPC_SNDInit() NONMATCH("https://decomp.me/scratch/VlDh8")
{
    sNPCSndID[eNPCSnd_GloveAttack] = 0;
    sNPCSndID[eNPCSnd_SleepyAttack] = 0;
    sNPCSndID[eNPCSnd_TubeAttack] = 0;
    sNPCSndID[eNPCSnd_FodBzztAttack] = 0;
    sNPCSndID[eNPCSnd_JellyfishAttack] = 0;

    sNPCSndFxVolume[eNPCSnd_GloveAttack] = 0.77f;
    sNPCSndFxVolume[eNPCSnd_SleepyAttack] = 0.77f;
    sNPCSndFxVolume[eNPCSnd_TubeAttack] = 0.77f;
    sNPCSndFxVolume[eNPCSnd_FodBzztAttack] = 0.77f;
    sNPCSndFxVolume[eNPCSnd_JellyfishAttack] = 0.77f;

    sNPCSndFx[eNPCSnd_GloveAttack] = xStrHash("Glove_hover_loop");
    sNPCSndFx[eNPCSnd_SleepyAttack] = xStrHash("ST_hit2_loop");
    sNPCSndFx[eNPCSnd_TubeAttack] = xStrHash("Tube_attack21_loop");
    sNPCSndFx[eNPCSnd_FodBzztAttack] = xStrHash("FodBzzt_attack_loop");
    sNPCSndFx[eNPCSnd_JellyfishAttack] = xStrHash("Jellyfish_zap_loop");
}

void zNPC_SNDPlay3D(eNPCSnd snd, xEnt* ent)
{
    if (globals.cmgr) return;
    if (sNPCSndID[snd] != 0) return;
    if (sNPCSndFx[snd] == 0) return;

    sNPCSndID[snd] = xSndPlay3D(sNPCSndFx[snd], sNPCSndFxVolume[snd], 0.0f, 0x80, 0, ent, 2.0f, 15.0f, SND_CAT_GAME, 0.0f);
}

void zNPC_SNDStop(eNPCSnd snd)
{
    if (sNPCSndFx[snd] == 0) return;

    xSndStop(sNPCSndID[snd]);
    sNPCSndID[snd] = 0;
}

S32 NPCC_LampStatus()
{
    return g_pc_playerInvisible == 0;
}

U32 NPCC_ForceTalkOk()
{
    return globals.player.g.DisableForceConversation == 0;
}
#include "zNPCGlyph.h"

#include "xString.h"
#include "xstransvc.h"
#include "zRenderState.h"
#include "zGlobals.h"
#include "zNPCSupport.h"

static const char* g_strz_glyphmodel[NPC_GLYPH_NOMORE] = {
    "unknown",
    "shiny_obj_purple.dff",
    "shiny_obj_blue.dff",
    "shiny_obj_green.dff",
    "shiny_obj_yellow.dff",
    "shiny_obj_orange.dff",
    "fx_glyph_talk",
    "fx_glyph_talkother",
    "fx_glyph_friend",
    "fx_glyph_daze.dff"
};

static S32 g_cnt_activeGlyphs[NPC_GLYPH_NOMORE] = {};

static NPCGlyph g_glyphs_talk[8] = { NPC_GLYPH_UNKNOWN };
static NPCGlyph g_glyphs_talkOther[8] = { NPC_GLYPH_UNKNOWN };
static NPCGlyph g_glyphs_friend[1] = { NPC_GLYPH_UNKNOWN };
static NPCGlyph g_glyphs_dazed[8] = { NPC_GLYPH_UNKNOWN };
static NPCGlyph g_glyphs_shinyOne[16] = { NPC_GLYPH_UNKNOWN };
static NPCGlyph g_glyphs_shinyFive[16] = { NPC_GLYPH_UNKNOWN };
static NPCGlyph g_glyphs_shinyTen[16] = { NPC_GLYPH_UNKNOWN };
static NPCGlyph g_glyphs_shinyFifty[16] = { NPC_GLYPH_UNKNOWN };
static NPCGlyph g_glyphs_shinyHundred[16] = { NPC_GLYPH_UNKNOWN };

void zNPCGlyph_Startup()
{
}

void zNPCGlyph_Shutdown()
{
}

void zNPCGlyph_ScenePrepare() NONMATCH("https://decomp.me/scratch/TsJ34")
{
    RpAtomic* mdl_raw;
    NPCGlyph* glyph;
    U32 aid;
    S32 i, k, cnt;
    NPCGlyph* list;
    
    g_cnt_activeGlyphs[NPC_GLYPH_UNKNOWN] = 0;
    g_cnt_activeGlyphs[NPC_GLYPH_SHINYONE] = 0;
    g_cnt_activeGlyphs[NPC_GLYPH_SHINYFIVE] = 0;
    g_cnt_activeGlyphs[NPC_GLYPH_SHINYTEN] = 0;
    g_cnt_activeGlyphs[NPC_GLYPH_SHINYFIFTY] = 0;
    g_cnt_activeGlyphs[NPC_GLYPH_SHINYHUNDRED] = 0;
    g_cnt_activeGlyphs[NPC_GLYPH_TALK] = 0;
    g_cnt_activeGlyphs[NPC_GLYPH_TALKOTHER] = 0;
    g_cnt_activeGlyphs[NPC_GLYPH_FRIEND] = 0;
    g_cnt_activeGlyphs[NPC_GLYPH_DAZED] = 0;
    
    list = NULL;
    
    for (i = NPC_GLYPH_UNKNOWN+1; i < NPC_GLYPH_NOMORE; i++) {
        cnt = zNPCGlyph_TypeToList((en_npcglyph)i, &list);
        
        if (list && cnt >= 1) {
            mdl_raw = NULL;

            if (g_strz_glyphmodel[i]) {
                aid = xStrHash(g_strz_glyphmodel[i]);
                if (aid) {
                    mdl_raw = (RpAtomic*)xSTFindAsset(aid, NULL);
                }
            }

            for (k = 0; k < cnt; k++) {
                glyph = &list[k];
                glyph->Init((en_npcglyph)i, mdl_raw);
            }
        }
    }
}

void zNPCGlyph_SceneFinish() NONMATCH("https://decomp.me/scratch/dYdCF")
{
    S32 i;
    NPCGlyph* glyph;
    S32 k, cnt;
    NPCGlyph* list;

    list = NULL;

    for (i = NPC_GLYPH_UNKNOWN+1; i < NPC_GLYPH_NOMORE; i++) {
        cnt = zNPCGlyph_TypeToList((en_npcglyph)i, &list);

        if (list && cnt >= 1) {
            for (k = 0; k < cnt; k++) {
                glyph = &list[k];
                glyph->Kill();
            }
        }
    }
}

void zNPCGlyph_SceneReset()
{
}

void zNPCGlyph_ScenePostInit()
{
}

void zNPCGlyph_Timestep(F32 dt) NONMATCH("https://decomp.me/scratch/K6eOc")
{
    S32 i, k, cnt;
    NPCGlyph* list, *glyph;

    list = NULL;

    for (i = NPC_GLYPH_UNKNOWN+1; i < NPC_GLYPH_NOMORE; i++) {
        cnt = zNPCGlyph_TypeToList((en_npcglyph)i, &list);

        if (list && cnt >= 1 && g_cnt_activeGlyphs[i] >= 1) {
            for (k = 0; k < cnt; k++) {
                glyph = &list[k];

                if (glyph->flg_glyph & 0x1) {
                    if ((glyph->flg_glyph & 0x10) && glyph->tmr_glyph < 0.0f) {
                        glyph->Discard();
                    } else {
                        glyph->tmr_glyph = xmax(-1.0f, glyph->tmr_glyph - dt);
                        if (glyph->mdl_glyph && (glyph->flg_glyph & 0x20)) {
                            glyph->RotAddDelta(NULL);
                        }
                        if (glyph->flg_glyph & (0x4 | 0x8)) {
                            glyph->Timestep(dt);
                        }
                    }
                }
            }
        }
    }
}

S32 zNPCGlyph_TypeIsOpaque(en_npcglyph gtyp)
{
    switch (gtyp) {
    case NPC_GLYPH_SHINYONE:
    case NPC_GLYPH_SHINYFIVE:
    case NPC_GLYPH_SHINYTEN:
    case NPC_GLYPH_SHINYFIFTY:
    case NPC_GLYPH_SHINYHUNDRED:
    case NPC_GLYPH_TALK:
    case NPC_GLYPH_TALKOTHER:
    case NPC_GLYPH_FRIEND:
        return 1;
    case NPC_GLYPH_DAZED:
        return 0;
    default:
        return 1;
    }
}

S32 zNPCGlyph_TypeNeedsLightKit(en_npcglyph gtyp)
{
    switch (gtyp) {
    case NPC_GLYPH_SHINYONE:
    case NPC_GLYPH_SHINYFIVE:
    case NPC_GLYPH_SHINYTEN:
    case NPC_GLYPH_SHINYFIFTY:
    case NPC_GLYPH_SHINYHUNDRED:
        return 0;
    case NPC_GLYPH_TALK:
    case NPC_GLYPH_TALKOTHER:
    case NPC_GLYPH_FRIEND:
    case NPC_GLYPH_DAZED:
        return 1;
    default:
        return 1;
    }
}

S32 zNPCGlyph_TypeToList(en_npcglyph gtyp, NPCGlyph** glist)
{
    S32 cnt = 0;
    *glist = NULL;

    switch (gtyp) {
    case NPC_GLYPH_SHINYONE:
        cnt = 16;
        *glist = g_glyphs_shinyOne;
        break;
    case NPC_GLYPH_SHINYFIVE:
        cnt = 16;
        *glist = g_glyphs_shinyFive;
        break;
    case NPC_GLYPH_SHINYTEN:
        cnt = 16;
        *glist = g_glyphs_shinyTen;
        break;
    case NPC_GLYPH_SHINYFIFTY:
        cnt = 16;
        *glist = g_glyphs_shinyFifty;
        break;
    case NPC_GLYPH_SHINYHUNDRED:
        cnt = 16;
        *glist = g_glyphs_shinyHundred;
        break;
    case NPC_GLYPH_TALK:
        cnt = 8;
        *glist = g_glyphs_talk;
        break;
    case NPC_GLYPH_TALKOTHER:
        cnt = 8;
        *glist = g_glyphs_talkOther;
        break;
    case NPC_GLYPH_FRIEND:
        cnt = 1;
        *glist = g_glyphs_friend;
        break;
    case NPC_GLYPH_DAZED:
        cnt = 8;
        *glist = g_glyphs_dazed;
        break;
    }

    return cnt;
}

void zNPCCommon_Glyphs_RenderAll(S32 doOpaqueStuff) NONMATCH("https://decomp.me/scratch/Zu12w")
{
    S32 i, k;
    NPCGlyph* glyph;
    S32 cnt;
    NPCGlyph* list;
    SDRenderState old_rendstat;

    list = NULL;
    old_rendstat = zRenderStateCurrent();

    if (doOpaqueStuff) {
        zRenderState(SDRS_OpaqueModels);
    } else {
        zRenderState(SDRS_NPCVisual);
    }

    for (i = NPC_GLYPH_UNKNOWN+1; i < NPC_GLYPH_NOMORE; i++) {
        if (doOpaqueStuff && !zNPCGlyph_TypeIsOpaque((en_npcglyph)i)) continue;
        if (!doOpaqueStuff && zNPCGlyph_TypeIsOpaque((en_npcglyph)i)) continue;

        if (zNPCGlyph_TypeNeedsLightKit((en_npcglyph)i)) {
            xLightKit_Enable(globals.player.ent.lightKit, globals.currWorld);
        } else {
            xLightKit_Enable(NULL, globals.currWorld);
        }

        cnt = zNPCGlyph_TypeToList((en_npcglyph)i, &list);

        if (list && cnt >= 1 && g_cnt_activeGlyphs[i] >= 1) {
            for (k = 0; k < cnt; k++) {
                glyph = &list[k];

                if ((glyph->flg_glyph & 0x1) && (glyph->flg_glyph & 0x2)) {
                    glyph->Render();
                }
            }
        }
    }

    xLightKit_Enable(NULL, globals.currWorld);
    zRenderState(old_rendstat);
}

NPCGlyph* GLYF_Acquire(en_npcglyph type)
{
    NPCGlyph* da_glyph;
    S32 i, cnt;
    NPCGlyph* list, *r31;

    da_glyph = NULL;
    list = NULL;
    cnt = zNPCGlyph_TypeToList((en_npcglyph)type, &list);

    if (!list || cnt < 1) {
        return NULL;
    }

    for (i = 0; i < cnt; i++) {
        r31 = &list[i];
        if (!(r31->flg_glyph & 0x1)) {
            r31->Reset();
            r31->flg_glyph = 0x1;
            da_glyph = r31;
            g_cnt_activeGlyphs[type]++;
            break;
        }
    }

    return da_glyph;
}

void NPCGlyph::Reset()
{
    this->flg_glyph = 0;

    xVec3Copy(&this->pos_glyph, &g_O3);
    xVec3Copy(&this->vel_glyph, &g_O3);

    this->tmr_glyph = 0.0f;
}

void NPCGlyph::Init(en_npcglyph gtyp, RpAtomic* mdl_raw)
{
    xVec3 var_58 = {};

    this->typ_glyph = gtyp;

    if (mdl_raw) {
        this->mdl_glyph = xModelInstanceAlloc(mdl_raw, NULL, 0, 0, NULL);
        if (this->mdl_glyph) {
            xMat4x3 var_4C;
            xMat3x3Euler(&var_4C, &var_58);
            xVec3Copy(&var_4C.pos, &g_O3);
            var_4C.flags = 0;
            
            xModelSetFrame(this->mdl_glyph, &var_4C);
        }
    }

    this->Reset();
}

void NPCGlyph::Kill()
{
    if (this->mdl_glyph) {
        xModelInstanceFree(this->mdl_glyph);
    }

    this->typ_glyph = NPC_GLYPH_UNKNOWN;
    this->flg_glyph = 0;

    xVec3Copy(&this->pos_glyph, &g_O3);
    xVec3Copy(&this->vel_glyph, &g_O3);

    this->mdl_glyph = NULL;
}

void NPCGlyph::Render()
{
    if (!this->mdl_glyph) return;

    if (this->flg_glyph & 0x40) {
        xVec3Copy(&this->mdl_glyph->Scale, &this->scl_glyph);
        xModelRender(this->mdl_glyph);
        xVec3Copy(&this->mdl_glyph->Scale, &g_Onez);
    } else {
        xModelRender(this->mdl_glyph);
    }
}

void NPCGlyph::Enable(S32 ison)
{
    if (ison) {
        this->flg_glyph |= 0x2;
    } else {
        this->flg_glyph &= ~0x2;
    }
}

void NPCGlyph::Discard()
{
    if (this->typ_glyph > NPC_GLYPH_UNKNOWN && this->typ_glyph < NPC_GLYPH_NOMORE) {
        g_cnt_activeGlyphs[this->typ_glyph]--;
        g_cnt_activeGlyphs[this->typ_glyph] = xmax(0, g_cnt_activeGlyphs[this->typ_glyph]);
    }

    this->flg_glyph = 0;
}

void NPCGlyph::PosSet(xVec3* pos)
{
    if (pos) {
        xVec3Copy(&this->pos_glyph, pos);
    } else {
        xVec3Copy(&this->pos_glyph, &g_O3);
    }

    if (this->mdl_glyph) {
        xVec3Copy((xVec3*)&this->mdl_glyph->Mat->pos, &this->pos_glyph);
    }
}

void NPCGlyph::VelSet(xVec3* vel)
{
    if (vel) {
        xVec3Copy(&this->vel_glyph, vel);
    } else {
        xVec3Copy(&this->vel_glyph, &g_O3);
    }

    if (!vel) {
        this->flg_glyph &= ~0x4;
    } else if (xVec3Length2(vel) > 0.0f) {
        this->flg_glyph |= 0x4;
    } else {
        this->flg_glyph &= ~0x4;
    }
}

void NPCGlyph::ScaleSet(xVec3* scale)
{
    if (scale) {
        xVec3Copy(&this->scl_glyph, scale);
        this->flg_glyph |= 0x40;
    } else {
        xVec3 allone = { 1.0f, 1.0f, 1.0f };
        xVec3Copy(&this->scl_glyph, &allone);
        this->flg_glyph &= ~0x40;
    }
}

void NPCGlyph::RotSet(xVec3* ang, S32 doautospin)
{
    xMat3x3 mat_rot = {};
    xMat3x3Euler(&mat_rot, ang);
    this->RotSet(&mat_rot, doautospin);
}

void NPCGlyph::RotSet(xMat3x3* mat, S32 doautospin)
{
    xMat3x3Copy(&this->rot_glyph, mat);

    if (doautospin) {
        this->flg_glyph |= 0x20;
    } else {
        this->flg_glyph &= ~0x20;
    }
}

void NPCGlyph::VelSet_Shiny() WIP NONMATCH("https://decomp.me/scratch/9x3kD")
{
    xVec3 vel = {};
    xVec3* dest = &globals.player.ent.bound.sph.center;
    xVec3* src = &this->pos_glyph;
    xVec3 vec;
    F32 dx__ = dest->x - src->x;
    F32 dy__ = dest->y - src->y;
    F32 dz__ = dest->z - src->z;

    vec.x = dx__;
    vec.y = dy__;
    vec.z = dz__;
    
    xVec3Normalize(&vec, &vec);
    xVec3Copy(&vel, &vec);

    F32 var_28 = 1.0f;
    F32 f1 = xsqr(dest->x - src->x) + xsqr(dest->y - src->y) + xsqr(dest->z - src->z);
    xsqrtfast(var_28, f1);
    
    F32 total_mult = (globals.player.PredictCurrVel + 2.0f * var_28) * 1.9f;
    if (total_mult > 45.0f) total_mult = 45.0f;
    if (total_mult < 3.5f) total_mult = 3.5f;

    xVec3SMul(&vel, &vel, total_mult);

    this->VelSet(&vel);
}

void NPCGlyph::RotAddDelta(xMat3x3* mat_rot)
{
    if (!mat_rot) {
        mat_rot = &this->rot_glyph;
    }

    xMat4x3* frame = xModelGetFrame(this->mdl_glyph);

    xMat3x3Mul(frame, mat_rot, frame);
    xModelSetFrame(this->mdl_glyph, frame);
}

void NPCGlyph::Timestep(F32 dt)
{
    xVec3 pos_new = {};

    switch (this->typ_glyph) {
    case NPC_GLYPH_SHINYONE:
    case NPC_GLYPH_SHINYFIVE:
    case NPC_GLYPH_SHINYTEN:
    case NPC_GLYPH_SHINYFIFTY:
    case NPC_GLYPH_SHINYHUNDRED:
        this->VelSet_Shiny();
        this->RotAddDelta(NULL);
        
        F32 chkdist;
        xVec3Dist2Macro(&globals.player.ent.bound.sph.center, &this->pos_glyph, &chkdist);
        if (chkdist <= xsqr(globals.player.ent.bound.sph.r)) {
            this->tmr_glyph = -1.0f;
        }
        break;
    case NPC_GLYPH_TALK:
    case NPC_GLYPH_TALKOTHER:
        if (this->mdl_glyph) {
            xVec3 delta;
            F32 ds2_cam = NPCC_ds2_toCam(&this->pos_glyph, &delta);
            if (ds2_cam < 0.1f) {
                break;
            }
            
            delta.y = 0.0f;
            if (SQ(delta.x) + SQ(delta.z) < 0.1f) {
                break;
            }
            
            delta.normalize();

            xMat4x3* r30 = xModelGetFrame(this->mdl_glyph);
            r30->pos = this->pos_glyph;
            r30->up = g_Y3;
            r30->right = delta;

            xVec3Cross(&r30->at, &r30->right, &g_Y3);
            xModelSetFrame(this->mdl_glyph, r30);
        }
        break;
    case NPC_GLYPH_FRIEND:
    case NPC_GLYPH_DAZED:
        if (this->flg_glyph & 0x4) {
            xVec3Copy(&pos_new, &this->pos_glyph);
            xVec3AddScaled(&pos_new, &this->vel_glyph, dt);
            this->PosSet(&pos_new);
        }
        break;
    }
}
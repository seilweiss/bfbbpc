#include "xNPCBasic.h"

#include "xEntAsset.h"
#include "xDraw.h"
#include "xFFX.h"
#include "zEnt.h"
#include "zGrid.h"

static xEntCollis g_colrec = {};

void xNPCBasic::Init(xEntAsset* asset) NONMATCH("https://decomp.me/scratch/hNbTq")
{
    xEnt* ent = this;

    if (asset->scale.x == 0.0f) asset->scale.x = 1.0f;
    if (asset->scale.y == 0.0f) asset->scale.y = 1.0f;
    if (asset->scale.z == 0.0f) asset->scale.z = 1.0f;

    xEntInit(ent, asset);

    ent->collType = k_XENT_COLLTYPE_NPC;
    ent->collLev = 4;
    ent->bound.type = k_XBOUNDTYPE_SPHERE;
    ent->moreFlags |= k_MORE_FLAGS_HITTABLE;

    zEntParseModelInfo(ent, asset->modelInfoID);
    xEntInitForType(ent);
    xEntInitShadow(*ent, entShadow_embedded);
    
    ent->simpShadow = &simpShadow_embedded;
    xShadowSimple_CacheInit(ent->simpShadow, ent, 80);

    if (ent->bound.type == k_XBOUNDTYPE_BOX) {
        iBoxForModel(&ent->bound.box.box, ent->collModel ? ent->collModel : ent->model);
    } else if (ent->bound.type == k_XBOUNDTYPE_OBB) {
        iBoxForModelLocal(&ent->bound.box.box, ent->collModel ? ent->collModel : ent->model);
    }

    if (flg_basenpc & 0x1) {
        ent->collis = (xEntCollis*)xMALLOC(sizeof(xEntCollis));
        memset(ent->collis, 0, sizeof(xEntCollis));
    }

    if (!(flg_basenpc & 0x2)) {
        ent->frame = (xEntFrame*)xMALLOC(sizeof(xEntFrame));
        memset(ent->frame, 0, sizeof(xEntFrame));
    }

    RestoreColFlags();

    f_setup = NPC_entwrap_setup;
    f_reset = NPC_entwrap_reset;
    
    ent->eventFunc = NPC_entwrap_event;
    ent->update = NPC_entwrap_update;
    ent->bupdate = NPC_entwrap_bupdate;
    ent->move = NPC_entwrap_move;
    ent->render = NPC_entwrap_render;
    ent->baseFlags &= (U16)~k_XBASE_RECEIVES_SHADOWS;
}

void xNPCBasic::Reset()
{
    xEntReset(this);

    DBG_PStatClear();

    if (!(flg_basenpc & 0x2)) {
        xVec3Copy(&frame->drot.axis, &g_Y3);
        frame->drot.angle = 0.0f;

        xVec3Copy(&frame->rot.axis, &g_Y3);
        frame->rot.angle = asset->ang.x;
    }

    flg_basenpc |= 0x4;
    colFreq = -1;
    colFreqReset = (S32)((xurand() - 0.5f) * 0.25f * 15.0f + 15.0f);

    RestoreColFlags();
}

static void NPC_alwaysUseSphere(xEnt* ent, xVec3*)
{
    xNPCBasic* npc = (xNPCBasic*)ent;
    
    xVec3 bndcent = {};
    xVec3Copy(&bndcent, xEntGetPos(npc));
    bndcent.y += 0.75f;

    npc->bound.type = k_XBOUNDTYPE_SPHERE;
    xVec3Copy(&npc->bound.sph.center, &bndcent);
    npc->bound.sph.r = 0.75f;

    if (npc->bound.type != k_XBOUNDTYPE_NONE) {
        xQuickCullForBound(&npc->bound.qcd, &npc->bound);
    }

    zGridUpdateEnt(npc);

    if (npc->DBG_IsNormLog((en_npcdcat)8, 2) || npc->DBG_IsNormLog((en_npcdcat)7, 2)) {
        xDrawSetColor(g_PIMP_GOLD);
        xBoundDraw(&npc->bound);
    }
}

static void NPC_spdBasedColFreq(xNPCBasic* npc, F32 dt) NONMATCH("https://decomp.me/scratch/SnrtN")
{
    if (dt < 0.00001f) return;

    F32 len = xVec3Length(&npc->frame->vel);
    if (len < 0.2f) return;
    
    xVec3 delt;
    F32 d;
    S32 nf;

    if (npc->bound.type == k_XBOUNDTYPE_SPHERE) {
        d = npc->bound.sph.r;
    } else {
        delt.x = npc->bound.box.box.upper.x - npc->bound.box.box.lower.x;
        delt.y = npc->bound.box.box.upper.y - npc->bound.box.box.lower.y;
        delt.z = npc->bound.box.box.upper.z - npc->bound.box.box.lower.z;
        d = xmax(delt.x, delt.z);
    }

    nf = (S32)(d / len * 30.0f);
    npc->colFreq = xmin(npc->colFreq, nf);
}

void xNPCBasic::Process(xScene* xscn, F32 dt) NONMATCH("https://decomp.me/scratch/cjFck")
{
    xEnt* ent = this;
    S32 hasgrav = 0;

    if (flg_colCheck || flg_penCheck) {
        colFreq--;
    } else {
        colFreq = 1;
    }

    if (ent->pflags & k_XENT_HAS_GRAVITY) {
        hasgrav = 1;
    }

    if (colFreq >= 0) {
        ent->pflags &= (U8)~k_XENT_HAS_GRAVITY;
    }

    if (ent->pflags & k_XENT_HAS_VELOCITY) {
        xEntApplyPhysics(ent, xscn, dt);
    }

    if (ent->pflags & k_XENT_IS_MOVING) {
        xEntMove(ent, xscn, dt);
    }

    if (ent->ffx) {
        xFFXApply(ent, xscn, dt);
    }

    if (ent->frame && (flg_upward & 0x2)) {
        ent->frame->dpos.y = xmax(0.0f, ent->frame->dpos.y);
        ent->frame->dvel.y = xmax(0.0f, ent->frame->dvel.y);
        ent->frame->vel.y = xmax(0.0f, ent->frame->vel.y);
        ent->frame->oldvel.y = xmax(0.0f, ent->frame->oldvel.y);
        ent->model->Mat->pos.y = xmax(ent->model->Mat->pos.y, ent->frame->oldmat.pos.y);
    }

    if (colFreq < 0) {
        colFreq = colFreqReset;

        DBG_PStatCont((en_npcperf)1);
        DBG_PStatOn((en_npcperf)2);

        if (!ent->collis) {
            memset(&g_colrec, 0, sizeof(g_colrec));
            ent->collis = &g_colrec;
        }

        ent->collis->chk = flg_colCheck;
        ent->collis->pen = flg_penCheck;

        xEntBoundUpdateCallback bak_bupdate = ent->bupdate;
        ent->bupdate = NPC_alwaysUseSphere;
        xEntCollide(ent, xscn, dt);
        ent->bupdate = bak_bupdate;

        CollideReview();

        if (!(flg_basenpc & 0x1)) {
            ent->collis = NULL;
        }

        DBG_PStatCont((en_npcperf)2);
        DBG_PStatOn((en_npcperf)1);
    }

    if ((ent->pflags & (k_XENT_HAS_VELOCITY | k_XENT_IS_MOVING)) &&
        xVec3Length2(&ent->frame->vel) > 0.1f) {
        NPC_spdBasedColFreq(this, dt);
    }

    if (hasgrav) {
        ent->pflags |= k_XENT_HAS_GRAVITY;
    }

    if (DBG_IsNormLog((en_npcdcat)7, 2)) {
        xDrawSetColor(g_BLUE);
        xBoundDraw(&ent->bound);
    }
}

void xNPCBasic::CollideReview()
{
    if (DBG_IsNormLog((en_npcdcat)8, 2)) {
        xDrawSetColor(g_LAVENDER);
        xBoundDraw(&bound);
    }
}

void xNPCBasic::NewTime(xScene*, F32)
{
    flg_basenpc &= ~0x4;
}

void NPC_entwrap_setup(xEnt* ent)
{
    xNPCBasic* npc = (xNPCBasic*)ent;
    npc->DBG_HaltOnMe(0, NULL);
    npc->Setup();
}

void NPC_entwrap_reset(xEnt* ent)
{
    xNPCBasic* npc = (xNPCBasic*)ent;
    npc->DBG_HaltOnMe(0, NULL);
    npc->Reset();
}

void NPC_entwrap_update(xEnt* ent, xScene* xscn, F32 dt_caller)
{
    xNPCBasic* npc;
    
    F32 dt = dt_caller;
    if (dt > 0.04f) dt = 0.025f;

    npc = (xNPCBasic*)ent;

    xEntBeginUpdate(npc, xscn, dt);

    npc->inUpdate = 1;
    npc->DBG_HaltOnMe(0, NULL);
    npc->DBG_PStatOn((en_npcperf)1);

    if (npc->isCulled) {
        npc->model->Flags &= (U16)~0x2;
    } else if (!(npc->flg_upward & 0x1)) {
        npc->model->Flags |= 0x2;
    }

    npc->Process(xscn, dt);
    npc->DBG_PStatCont((en_npcperf)1);
    npc->inUpdate = 0;

    xEntEndUpdate(npc, xscn, dt);

    npc->NewTime(xscn, dt);
}

void NPC_entwrap_bupdate(xEnt* ent, xVec3* pos)
{
    xNPCBasic* npc = (xNPCBasic*)ent;
    npc->BUpdate(pos);
}

void NPC_entwrap_move(xEnt* ent, xScene* xscn, F32 dt, xEntFrame* frm)
{
    xNPCBasic* npc = (xNPCBasic*)ent;
    npc->Move(xscn, dt, frm);
}

S32 NPC_entwrap_event(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget)
{
    xNPCBasic* npc = (xNPCBasic*)to;
    S32 used = 0;
    return npc->SysEvent(from, to, toEvent, toParam, toParamWidget, &used);
}

void NPC_entwrap_render(xEnt* ent)
{
    xNPCBasic* npc = (xNPCBasic*)ent;
    npc->Render();
}
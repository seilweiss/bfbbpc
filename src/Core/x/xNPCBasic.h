#pragma once

#include "xEnt.h"
#include "xFactory.h"

// Not present in PS2 DWARF, values unknown
enum en_npcdcat
{
};

// Not present in PS2 DWARF, values unknown
enum en_npcperf
{
};

struct xNPCBasic : xEnt, xFactoryInst
{
    void(*f_setup)(xEnt*);
    void(*f_reset)(xEnt*);
    S32 flg_basenpc : 16;
    S32 inUpdate : 8;
    S32 flg_upward : 8;
    S32 colFreq;
    S32 colFreqReset;
    U32 flg_colCheck : 8;
    U32 flg_penCheck : 8;
    U32 flg_unused : 16;
    S32 myNPCType;
    xEntShadow entShadow_embedded;
    xShadowSimpleCache simpShadow_embedded;

    xNPCBasic(S32 myType)
    {
        this->myNPCType = myType;
    }

    virtual void Init(xEntAsset* asset);
    virtual void PostInit() {}
    virtual void Setup() {}
    virtual void PostSetup() {}
    virtual void Reset();
    virtual void Process(xScene* xscn, F32 dt);
    virtual void BUpdate(xVec3* pos) { xEntDefaultBoundUpdate(this, pos); }
    virtual void NewTime(xScene* xscn, F32 dt);
    virtual void Move(xScene* xscn, F32 dt, xEntFrame* frm) {}
    virtual S32 SysEvent(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget, S32* handled) { return 1; }
    virtual void Render() { xEntRender(this); }

    S32 SelfType() const { return this->myNPCType; }

    virtual void Save(xSerial* xser) const {}
    virtual void Load(xSerial* xser) {}
    virtual void CollideReview();
    virtual U8 ColChkFlags() const { return 0; }
    virtual U8 ColPenFlags() const { return 0; }
    virtual U8 ColChkByFlags() const { return 0; }
    virtual U8 ColPenByFlags() const { return 0; }
    virtual U8 PhysicsFlags() const { return 0; }

    void RestoreColFlags()
    {
        this->flg_colCheck = ColChkFlags();
        this->flg_penCheck = ColPenFlags();
        this->chkby = ColChkByFlags();
        this->penby = ColPenByFlags();
        this->pflags = PhysicsFlags();
        this->colFreq = -1;
    }

    S32 DBG_IsNormLog(en_npcdcat, S32) { return 0; }
    void DBG_PStatOn(en_npcperf) {}
    void DBG_PStatCont(en_npcperf) {}
    void DBG_PStatClear() {}
    void DBG_HaltOnMe(U32, char*) {}
};

void NPC_entwrap_setup(xEnt* ent);
void NPC_entwrap_reset(xEnt* ent);
void NPC_entwrap_update(xEnt* ent, xScene* xscn, F32 dt_caller);
void NPC_entwrap_bupdate(xEnt* ent, xVec3* pos);
void NPC_entwrap_move(xEnt* ent, xScene* xscn, F32 dt, xEntFrame* frm);
S32 NPC_entwrap_event(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget);
void NPC_entwrap_render(xEnt* ent);
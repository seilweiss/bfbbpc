#pragma once

#include "xEnt.h"
#include "xFactory.h"

class xNPCBasic : public xEnt, public xFactoryInst
{
protected:
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

public:
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

    S32 SelfType() const { return myNPCType; }

    virtual void Save(xSerial* xser) const {}
    virtual void Load(xSerial* xser) {}
    virtual void CollideReview();
    virtual U8 ColChkFlags() const { return 0; }
    virtual U8 ColPenFlags() const { return 0; }
    virtual U8 ColChkByFlags() const { return 0; }
    virtual U8 ColPenByFlags() const { return 0; }
    virtual U8 PhysicsFlags() const { return 0; }
};
#pragma once

#include "xRMemData.h"
#include "xBase.h"
#include "xBehaviour.h"

enum en_psynote
{
    PSY_NOTE_HASRESUMED,
    PSY_NOTE_HASENTERED,
    PSY_NOTE_ANIMCHANGED,
    PSY_NOTE_NOMORE,
    PSY_NOTE_FORCE = FORCEENUMSIZEINT
};

struct xPSYNote
{
    xPSYNote() {}
    virtual void Notice(en_psynote note, xGoal* goal, void*) {}
};

enum en_pendtype
{
    PEND_TRAN_NONE,
    PEND_TRAN_SET,
    PEND_TRAN_PUSH,
    PEND_TRAN_POP,
    PEND_TRAN_POPTO,
    PEND_TRAN_POPALL,
    PEND_TRAN_SWAP,
    PEND_TRAN_INPROG,
    PEND_TRAN_NOMORE
};

enum PSY_BRAIN_STATUS
{
    PSY_STAT_BLANK,
    PSY_STAT_GROW,
    PSY_STAT_EXTEND,
    PSY_STAT_THINK,
    PSY_STAT_NOMORE,
    PSY_STAT_FORCE = FORCEENUMSIZEINT
};

struct xPsyche : RyzMemData
{
protected:
    xBase* clt_owner;
    xPSYNote* cb_notice;
    S32 flg_psyche;
    xGoal* goallist;
    xGoal* goalstak[5];
    F32 tmr_stack[5][1];
    S32 staktop;
    xGoal* pendgoal;
    en_pendtype pendtype;
    S32 gid_safegoal;
    void(*fun_remap)(S32*, en_trantype*);
    void* userContext;
    S32 cnt_transLastTimestep;
    PSY_BRAIN_STATUS psystat;
    xBase fakebase;

public:
    xBase* GetClient() { return clt_owner; }
};

void xBehaveMgr_Startup();
void xBehaveMgr_Shutdown();
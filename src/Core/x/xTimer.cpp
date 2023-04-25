#include "xTimer.h"

#include "xTimerAsset.h"
#include "xMath.h"
#include "zEvent.h"
#include "zTalkBox.h"
#include "zGlobals.h"

static F32 GetRandomizedTime(xTimerAsset* tasset) NONMATCH("https://decomp.me/scratch/b9eqj")
{
    U32 halfRangeMilli = (U32)(tasset->randomRange * 1000.0f);
    if (halfRangeMilli == 0) {
        return tasset->seconds;
    }

    S32 offset = xrand() % (halfRangeMilli * 2) - halfRangeMilli;
    F32 time = tasset->seconds + offset / 1000.0f;

    return time;
}

void xTimerInit(void* b, void* tasset)
{
    xTimerInit((xBase*)b, (xTimerAsset*)tasset);
}

static U32 sPauseTimerHash[20] = {
    0xBC345600, 0xBC345609,
    0xBC345683, 0xBC34568C,
    0xBC345706, 0xBC34570F,
    0xBC345789, 0xBC345792,
    0xBC34580C, 0xBC345815,
    0xBC34588F, 0xBC345898,
    0xBC345912, 0xBC34591B,
    0xBC345995, 0xBC34599E,
    0xBC345A18, 0xBC345A21,
    0xBC345A9B, 0xBC345AA4
};

static S32 xTimer_ObjIDIsPauseTimer(U32 id)
{
    if (id == 0xCB3F6340) return 1; // TRACK
    if (id >= 0x016FC9F0 && id <= 0x016FC9F9) return 1; // TRACK0-9
    
    // TRACK00-99
    S32 foo = (id >= 0xBC345600);
    S32 bar = (id <= 0xBC345AA4);
    if (foo && bar) {
        for (S32 i = 0; i < 10; i++) {
            if (id >= sPauseTimerHash[i*2] && id <= sPauseTimerHash[i*2+1]) {
                return 1;
            }
        }
    }

    return 0;
}

void xTimerInit(xBase* b, xTimerAsset* tasset)
{
    xTimer* t = (xTimer*)b;

    xBaseInit(t, tasset);

    t->eventFunc = xTimerEventCB;
    t->tasset = tasset;

    if (t->linkCount) {
        t->link = (xLinkAsset*)(t->tasset + 1);
    } else {
        t->link = NULL;
    }

    t->state = k_XTIMER_STATE_IDLE;
    t->secondsLeft = GetRandomizedTime(tasset);
    t->runsInPause = xTimer_ObjIDIsPauseTimer(t->id);
    t->flags = 0;
}

void xTimerReset(xTimer* t)
{
    xBaseReset(t, t->tasset);

    t->state = k_XTIMER_STATE_IDLE;
    t->secondsLeft = GetRandomizedTime(t->tasset);
    t->flags = 0;
}

void xTimerSave(xTimer* ent, xSerial* s)
{
    xBaseSave(ent, s);

    s->Write(ent->state);
    s->Write(ent->secondsLeft);
}

void xTimerLoad(xTimer* ent, xSerial* s)
{
    xBaseLoad(ent, s);

    s->Read(&ent->state);
    s->Read(&ent->secondsLeft);
}

S32 xTimerEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget)
{
    xTimer* t = (xTimer*)to;

    switch (toEvent) {
    case eEventRun:
        t->state = k_XTIMER_STATE_RUN;
        break;
    case eEventStop:
        if (t->state == k_XTIMER_STATE_RUN) {
            t->state = k_XTIMER_STATE_IDLE;
        }
        break;
    case eEventReset:
        xTimerReset(t);
        break;
    case eEventExpired:
        t->state = k_XTIMER_STATE_IDLE;
        break;
    case eEventTimerSet:
        t->secondsLeft = toParam[0];
        break;
    case eEventTimerAdd:
        t->secondsLeft += toParam[0];
        break;
    }

    return 1;
}

void xTimerUpdate(xBase* to, xScene* sc, F32 dt) NONMATCH("https://decomp.me/scratch/K63Mv")
{
    xTimer* t = (xTimer*)to;

    if (t->state != k_XTIMER_STATE_RUN) return;
    if ((t->flags & 0x1) && globals.player.ControlOff && ztalkbox::get_active()) return;

    t->secondsLeft -= dt;
    if (t->secondsLeft <= 0.0f) {
        zEntEvent(t, t, eEventExpired);
    }
}
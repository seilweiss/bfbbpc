#include "xCounter.h"

#include "xCounterAsset.h"
#include "xDebugTweak.h"
#include "zEvent.h"

namespace {
void add_tweaks(xCounter& counter) {}
}

void xCounterInit()
{
    xDebugRemoveTweak("Widgets|Counters");
}

void xCounterInit(void* b, void* asset)
{
    xCounterInit((xBase*)b, (xCounterAsset*)asset);
}

void xCounterInit(xBase* b, xCounterAsset* asset)
{
    xCounter* t = (xCounter*)b;

    xBaseInit(t, asset);

    t->eventFunc = xCounterEventCB;
    t->asset = asset;

    if (t->linkCount) {
        t->link = (xLinkAsset*)(t->asset + 1);
    } else {
        t->link = NULL;
    }

    t->state = k_XCOUNTER_STATE_IDLE;
    t->count = asset->count;
    t->counterFlags = 0;

    add_tweaks(*t);
}

void xCounterReset(xBase* b)
{
    xCounter* t = (xCounter*)b;

    xBaseInit(t, t->asset);

    if (t->linkCount) {
        t->link = (xLinkAsset*)(t->asset + 1);
    } else {
        t->link = NULL;
    }

    t->count = t->asset->count;
    t->state = k_XCOUNTER_STATE_IDLE;
}

void xCounterSave(xCounter* ent, xSerial* s)
{
    xBaseSave(ent, s);

    s->Write(ent->state);
    s->Write(ent->count);
}

void xCounterLoad(xCounter* ent, xSerial* s)
{
    xBaseLoad(ent, s);

    s->Read(&ent->state);
    s->Read(&ent->count);
}

S32 xCounterEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget)
{
    xCounter* t = (xCounter*)to;

    switch (toEvent) {
    case eEventDecrement:
    {
        if (t->state == k_XCOUNTER_STATE_EXPIRED) break;
        if ((t->counterFlags & 0x1) && t->count == 2) break;

        t->count--;
        if (t->count == 0) {
            zEntEvent(t, t, eEventExpired);
        } else if (t->count > 0 && t->count <= 20) {
            zEntEvent(t, t, eEventCount1 + t->count - 1);
        }

        break;
    }
    case eEventIncrement:
    {
        if (t->state == k_XCOUNTER_STATE_EXPIRED) break;

        t->count++;
        if (t->count == 0) {
            zEntEvent(t, t, eEventExpired);
        } else if (t->count > 0 && t->count <= 20) {
            zEntEvent(t, t, eEventCount1 + t->count - 1);
        }

        break;
    }
    case eEventReset:
    {
        t->state = k_XCOUNTER_STATE_IDLE;
        t->count = t->asset->count;
        break;
    }
    case eEventExpired:
    {
        if ((t->counterFlags & 0x1) && t->count == 2) break;

        t->count = 0;
        t->state = k_XCOUNTER_STATE_EXPIRED;

        break;
    }
    default:
    {
        if (t->state == k_XCOUNTER_STATE_EXPIRED) break;
        if (toEvent < eEventCount1 || toEvent > eEventCount20) break;

        S16 newCount = toEvent - (eEventCount1 - 1);

        if ((t->counterFlags & 0x1) && t->count == 2) break;

        t->count = newCount;

        break;
    }
    }

    return 1;
}
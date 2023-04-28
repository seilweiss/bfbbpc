#include "xGroup.h"

#include "xGroupAsset.h"
#include "xMemMgr.h"
#include "xEnt.h"
#include "zEvent.h"
#include "zScene.h"

void xGroupInit(void* b, void* asset)
{
    xGroupInit((xBase*)b, (xGroupAsset*)asset);
}

void xGroupInit(xBase* b, xGroupAsset* asset)
{
    xGroup* t = (xGroup*)b;

    xBaseInit(t, asset);

    t->eventFunc = xGroupEventCB;
    t->asset = asset;

    if (t->linkCount) {
        t->link = (xLinkAsset*)((U8*)t->asset + sizeof(xGroupAsset) + asset->itemCount * sizeof(U32));
    } else {
        t->link = NULL;
    }

    U32 count = xGroupGetCount(t);

    t->item = count ? (xBase**)xMALLOC(count * sizeof(xBase*)) : NULL;
    t->last_index = 0;
    t->flg_group = 0;
}

void xGroupSetup(xGroup* g)
{
    if (g->flg_group & 0x1) return;

    U32 count = xGroupGetCount(g);
    for (U32 i = 0; i < count; i++) {
        g->item[i] = xGroupFindItemPtr(g, i);
    }

    g->flg_group |= 0x1;
}

void xGroupSave(xGroup* ent, xSerial* s)
{
    xBaseSave(ent, s);
}

void xGroupLoad(xGroup* ent, xSerial* s)
{
    xBaseLoad(ent, s);
}

void xGroupReset(xGroup* ent)
{
    xBaseReset(ent, ent->asset);

    ent->last_index = 0;
}

S32 xGroupEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget)
{
    xGroup* g = (xGroup*)to;

    switch (toEvent) {
    case eEventReset:
        xGroupReset(g);
        break;
    case eEventDisableGroupContents:
        toEvent = eEventDisable;
        break;
    }

    S32 rand = -1;
    if (g->asset->groupFlags & 0x1) {
        rand = xrand() % g->asset->itemCount;
    } else if (g->asset->groupFlags & 0x2) {
        rand = g->last_index;
        g->last_index = (g->last_index + 1) % g->asset->itemCount;
    }

    switch (toEvent) {
    case eEventFastVisible:
    {
        for (S32 i = 0; i < g->asset->itemCount; i++) {
            if (rand == -1 || rand == i) {
                xBase* b = g->item[i];
                if (b) {
                    if (b->baseFlags & k_XBASE_IS_ENTITY) {
                        xEntShow((xEnt*)b);
                    } else {
                        zEntEvent(b, toEvent, toParam, toParamWidget);
                    }
                }
            }
        }
        return 1;
    }
    case eEventFastInvisible:
    {
        for (S32 i = 0; i < g->asset->itemCount; i++) {
            if (rand == -1 || rand == i) {
                xBase* b = g->item[i];
                if (b) {
                    if (b->baseFlags & k_XBASE_IS_ENTITY) {
                        xEntHide((xEnt*)b);
                    } else {
                        zEntEvent(b, toEvent, toParam, toParamWidget);
                    }
                }
            }
        }
        return 1;
    }
    default:
    {
        for (S32 i = 0; i < g->asset->itemCount; i++) {
            if (rand == -1 || rand == i) {
                xBase* b = g->item[i];
                if (b) {
                    zEntEvent(b, toEvent, toParam, toParamWidget);
                }
            }
        }
        return 1;
    }
    }
}

U32 xGroupGetCount(xGroup* g)
{
    return g->asset->itemCount;
}

xBase* xGroupGetItemPtr(xGroup* g, U32 index)
{
    if (!(g->flg_group & 0x1)) {
        xGroupSetup(g);
    }

    if (g->item) {
        return g->item[index];
    }

    return NULL;
}

xBase* xGroupFindItemPtr(xGroup* g, U32 index)
{
    U32* idx = (U32*)(g->asset + 1);
    return zSceneFindObject(idx[index]);
}

U32 xGroupGetItem(xGroup* g, U32 index)
{
    U32* idx = (U32*)(g->asset + 1);
    return idx[index];
}

U32 xGroup::get_any()
{
    if (asset->itemCount == 0) return 0;

    U32* idx = (U32*)(asset + 1);
    U32 id = idx[last_index];

    last_index = (last_index + 1) % asset->itemCount;

    return id;
}
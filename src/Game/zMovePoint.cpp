#include "zMovePoint.h"

#include "xMemMgr.h"
#include "zScene.h"
#include "zEvent.h"
#include "zNPCSupplement.h"

static zMovePoint* g_mvpt_list;
static S32 g_mvpt_cnt;

zMovePoint* zMovePoint_GetMemPool(S32 cnt) NONMATCH("https://decomp.me/scratch/QcCVd")
{
    g_mvpt_list = cnt ? (zMovePoint*)xMALLOC(cnt * sizeof(zMovePoint)) : NULL;
    g_mvpt_cnt = cnt;
    
    return g_mvpt_list;
}

void zMovePointInit(zMovePoint* m, xMovePointAsset* asset)
{
    xMovePointInit(m, asset);

    m->eventFunc = zMovePointEventCB;

    if (m->linkCount) {
        m->link = (xLinkAsset*)((U8*)asset + sizeof(xMovePointAsset) + asset->numPoints * sizeof(U32));
    } else {
        m->link = NULL;
    }
}

zMovePoint* zMovePoint_GetInst(S32 n)
{
    return &g_mvpt_list[n];
}

void zMovePointSetup(zMovePoint* mvpt, zScene* scn)
{
    xMovePointSetup(mvpt, scn);
}

zMovePoint* zMovePoint_From_xAssetID(U32 aid)
{
    zMovePoint* da_mvpt = NULL;
    for (S32 i = 0; i < g_mvpt_cnt; i++) {
        zMovePoint* m = &g_mvpt_list[i];
        if (m->asset->id == aid) {
            da_mvpt = m;
            break;
        }
    }
    return da_mvpt;
}

void zMovePointSave(zMovePoint* ent, xSerial* s)
{
    xMovePointSave(ent, s);
}

void zMovePointLoad(zMovePoint* ent, xSerial* s)
{
    xMovePointLoad(ent, s);
}

void zMovePointReset(zMovePoint* ent)
{
    xMovePointReset(ent);
}

S32 zMovePointEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget)
{
    zMovePoint* m = (zMovePoint*)to;

    switch (toEvent) {
    case eEventOn:
        m->on = 1;
        break;
    case eEventOff:
        m->on = 0;
        break;
    case eEventReset:
        zMovePointReset(m);
        break;
    case eEventArrive:
        break;
    case eEventMakeASplash:
        if (m->pos) {
            if (toParam[0] < 0.00001f) {
                NPCC_MakeASplash(m->pos, -1.0f);
            } else {
                NPCC_MakeASplash(m->pos, toParam[0]);
            }
        }
        break;
    }

    return 1;
}

F32 zMovePointGetNext(const zMovePoint* current, const zMovePoint* prev, zMovePoint** next, xVec3* hdng)
{
    return xMovePointGetNext(current, prev, (xMovePoint**)next, hdng);
}

const xVec3* zMovePointGetPos(const zMovePoint* m)
{
    return xMovePointGetPos(m);
}

F32 zMovePointGetDelay(const zMovePoint* m)
{
    return xMovePointGetDelay(m);
}
#pragma once

#include "xGroup.h"

typedef U32(*xUpdateCullEntCallback)(void*, void*);
typedef void(*xUpdateCullMgrActivateCallback)(void*);
typedef void(*xUpdateCullMgrDeactivateCallback)(void*);

struct xUpdateCullEnt
{
    U16 index;
    S16 groupIndex;
    xUpdateCullEntCallback cb;
    void* cbdata;
    xUpdateCullEnt* nextInGroup;
};

struct xUpdateCullGroup
{
    U32 active;
    U16 startIndex;
    U16 endIndex;
    xGroup* groupObject;
};

struct xUpdateCullMgr
{
    U32 entCount;
    U32 entActive;
    void** ent;
    xUpdateCullEnt** mgr;
    U32 mgrCount;
    U32 mgrCurr;
    xUpdateCullEnt* mgrList;
    U32 grpCount;
    xUpdateCullGroup* grpList;
    xUpdateCullMgrActivateCallback activateCB;
    xUpdateCullMgrDeactivateCallback deactivateCB;
};
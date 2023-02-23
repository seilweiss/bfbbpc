#pragma once

#include "xClumpColl.h"

#include <rwcore.h>
#include <rpworld.h>

struct xJSPNodeInfo
{
    S32 originalMatIndex;
    S32 nodeFlags;
};

struct xJSPHeader
{
    char idtag[4];
    U32 version;
    U32 jspNodeCount;
    RpClump* clump;
    xClumpCollBSPTree* colltree;
    xJSPNodeInfo* jspNodeList;
};

#ifdef GAMECUBE
struct xJSPHeaderGC : xJSPHeader
{
    U32 stripVecCount;
    RwV3d* stripVecList;
};

typedef struct xJSPHeaderGC xJSPHeaderEx;
#else
typedef struct xJSPHeader xJSPHeaderEx;
#endif
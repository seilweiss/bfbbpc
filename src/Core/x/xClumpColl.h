#pragma once

#include "types.h"

#include <rwcore.h>
#include <rpcollis.h>

struct xClumpCollBSPBranchNode
{
    U32 leftInfo;
    U32 rightInfo;
    F32 leftValue;
    F32 rightValue;
};

struct xClumpCollBSPVertInfo
{
    U16 atomIndex;
    U16 meshVertIndex;
};

struct xClumpCollBSPTriangle
{
    union
    {
        xClumpCollBSPVertInfo i;
        RwV3d* p;
    } v;
    U8 flags;
    U8 platData;
    U16 matIndex;
};

struct xClumpCollBSPTree
{
    U32 numBranchNodes;
    xClumpCollBSPBranchNode* branchNodes;
    U32 numTriangles;
    xClumpCollBSPTriangle* triangles;
};

struct xClumpCollV3dGradient
{
    F32 dydx;
    F32 dzdx;
    F32 dxdy;
    F32 dzdy;
    F32 dxdz;
    F32 dydz;
};

struct nodeInfo
{
    U32 type;
    U32 index;
};

typedef S32(*xClumpCollIntersectionCallback)(xClumpCollBSPTriangle*, void*);

extern U8 xClumpColl_FilterFlags;

xClumpCollBSPTree* xClumpColl_StaticBufferInit(void* data, U32);
void xClumpColl_InstancePointers(xClumpCollBSPTree* tree, RpClump* clump);
xClumpCollBSPTree* xClumpColl_ForAllBoxLeafNodeIntersections(xClumpCollBSPTree* tree, RwBBox* box, xClumpCollIntersectionCallback callBack, void* data);
xClumpCollBSPTree* xClumpColl_ForAllLineLeafNodeIntersections(xClumpCollBSPTree* tree, RwLine* line, xClumpCollV3dGradient* grad, xClumpCollIntersectionCallback callBack, void* data);
xClumpCollBSPTree* xClumpColl_ForAllCapsuleLeafNodeIntersections(xClumpCollBSPTree* tree, RwLine* line, F32 radius, xClumpCollV3dGradient* grad, xClumpCollIntersectionCallback callBack, void* data);
xClumpCollBSPTree* xClumpColl_ForAllIntersections(xClumpCollBSPTree* tree, RpIntersection* intersection, RpIntersectionCallBackWorldTriangle callBack, void* data);
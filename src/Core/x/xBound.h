#pragma once

#include "xMath3.h"
#include "xQuickCull.h"
#include "xCollide.h"

struct xBound
{
    xQCData qcd;
    U8 type;
    U8 pad[3];
    union
    {
        xSphere sph;
        xBBox box;
        xCylinder cyl;
    };
    xMat4x3* mat;
};

enum
{
    k_XBOUNDTYPE_NONE = 0,
    k_XBOUNDTYPE_SPHERE = 1,
    k_XBOUNDTYPE_BOX = 2,
    k_XBOUNDTYPE_CYL = 3,
    k_XBOUNDTYPE_OBB = 4
};

void xBoundUpdate(xBound* b);
void xBoundGetBox(xBox& box, const xBound& bound);
void xBoundGetSphere(xSphere& o, const xBound& bound);
void xBoundSphereHitsOBB(const xSphere* s, const xBox* b, const xMat4x3* m, xCollis* coll);
void xBoundHitsBound(const xBound* a, const xBound* b, xCollis* c);
void xRayHitsBound(const xRay3* r, const xBound* b, xCollis* c);
void xSphereHitsBound(const xSphere* o, const xBound* b, xCollis* c);
void xVecHitsBound(const xVec3* v, const xBound* b, xCollis* c);
void xBoundDraw(const xBound* b);

inline const xVec3* xBoundCenter(const xBound* b)
{
    return &b->sph.center;
}

inline xVec3* xBoundCenter(xBound* b)
{
    return &b->sph.center;
}
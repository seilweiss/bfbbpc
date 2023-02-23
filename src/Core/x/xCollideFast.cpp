#include "xCollideFast.h"

void xCollideFastInit(xScene* sc)
{
    iCollideFastInit(sc);
}

U32 xRayHitsSphereFast(const xRay3* r, const xSphere* s)
{
    xVec3 diff;
    xVec3Sub(&diff, &r->origin, &s->center);
    
    F32 c = xVec3Dot(&diff, &diff) - xsqr(s->r);
    if (c <= 0.0f) return 1;
    if ((r->flags & XRAY3_USE_MAX) && c > (2.0f * s->r + r->max_t) * r->max_t) return 0;
    
    F32 b = xVec3Dot(&diff, &r->dir);
    if (b >= 0.0f) return 0;
    if (xsqr(b) >= c) return 1;

    return 0;
}

U32 xRayHitsBoxFast(const xRay3* r, const xBox* b)
{
    xIsect isx;
    iBoxIsectRay(b, r, &isx);
    return (isx.penned <= 0.0f || isx.contained <= 0.0f);
}
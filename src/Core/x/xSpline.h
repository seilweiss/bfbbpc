#pragma once

#include "xVec3.h"

struct xCoef
{
    F32 a[4];
};

struct xCoef3
{
    xCoef x;
    xCoef y;
    xCoef z;
};

struct xSpline3
{
    U16 type;
    U16 flags;
    U32 N;
    U32 allocN;
    xVec3* points;
    F32* time;
    xVec3* p12;
    xVec3* bctrl;
    F32* knot;
    xCoef3* coef;
    U32 arcSample;
    F32* arcLength;
};

void xSpline3_EvalSeg(xSpline3* spl, F32 u, U32 deriv, xVec3* o);
F32 xSpline3_EvalArcApprox(xSpline3* spl, F32 s, U32 deriv, xVec3* o);
void xSpline3_ArcInit(xSpline3* spl, U32 sample);
xSpline3* xSpline3_Bezier(xVec3* points, F32* time, U32 numpoints, U32 numalloc, xVec3* p1, xVec3* p2);
void xSpline3_Update(xSpline3* spl);
void xSpline3_Catmullize(xSpline3* spl);

inline F32 xSpline3_ArcTotal(xSpline3* spl)
{
    return spl->arcLength[spl->N * spl->arcSample - 1];
}
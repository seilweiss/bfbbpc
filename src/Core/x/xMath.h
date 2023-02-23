#pragma once

#include "iMath.h"

#include <cmath>

#define HUGE 1e38f
#define EPSILON 1e-5f

#define PI 3.14159265358979323846f

#define xmin(a, b) ((a) < (b) ? (a) : (b))
#define xmax(a, b) ((a) > (b) ? (a) : (b))
#define xclamp(x, a, b) (xmax((a), xmin((x), (b))))
#define xlerp(a, b, t) ((a) + (t) * ((b) - (a)))

#define xsqr(x) ((x) * (x))
#define xabs(x) iabs(x)

#define xeq(a, b, e) (xabs((a) - (b)) <= (e))
#define xfeq0(x) (((x) >= -EPSILON) && ((x) <= EPSILON))

#define xdeg2rad(x) (F32)(PI * (x) / 180.0f)
#define xrad2deg(x) (F32)(180.0f * (x) / PI)

#define xsin(x) isin(x)
#define xcos(x) icos(x)
#define xtan(x) itan(x)

struct xFuncPiece
{
    F32 coef[5];
    F32 end;
    S32 order;
    xFuncPiece* next;
};

void xMathInit();
void xMathExit();
F32 xatof(const char* x);
void xsrand(U32 seed);
U32 xrand();
F32 xurand();
U32 xMathSolveQuadratic(F32 a, F32 b, F32 c, F32* x1, F32* x2);
U32 xMathSolveCubic(F32 a, F32 b, F32 c, F32 d, F32* x1, F32* x2, F32* x3);
F32 xAngleClamp(F32 a);
F32 xAngleClampFast(F32 a);
F32 xDangleClamp(F32 a);
void xAccelMove(F32& x, F32& v, F32 a, F32 dt, F32 endx, F32 maxv);
F32 xAccelMoveTime(F32 dx, F32 a, F32 minv, F32 maxv);
void xAccelMove(F32& x, F32& v, F32 a, F32 dt, F32 maxv);
void xAccelStop(F32& x, F32& v, F32 a, F32 dt);
F32 xFuncPiece_Eval(xFuncPiece* func, F32 param, xFuncPiece** iterator);
void xFuncPiece_EndPoints(xFuncPiece* func, F32 pi, F32 pf, F32 fi, F32 ff);
void xFuncPiece_ShiftPiece(xFuncPiece* shift, xFuncPiece* func, F32 newZero);

inline F32 xrmod(F32 ang)
{
    F32 frac = xrad2deg(1.0f/360.0f) * ang;
    if (frac < 0.0f) return (frac - std::ceilf(frac) + 1.0f) * (2*PI);
    if (frac >= 1.0f) return (frac - std::floorf(frac)) * (2*PI);
    return ang;
}

template <class T>
inline T range_limit(T v, T minv, T maxv)
{
    return v <= minv ? minv : v >= maxv ? maxv : v;
}

#include "xMathInlines.h"
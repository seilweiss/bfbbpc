#include "xMath.h"

#include <stdlib.h>

static S32 xmath_inited;
static S32 xmath_exited;
static U32 rndseed;

void xMathInit()
{
    if (xmath_inited) return;

    xmath_inited = 1;
    rndseed = 0;
}

void xMathExit()
{
    if (xmath_exited) return;

    xmath_exited = 1;
}

F32 xatof(const char* x)
{
    return (F32)atof(x);
}

void xsrand(U32 seed)
{
    rndseed = seed;
}

U32 xrand() NONMATCH("https://decomp.me/scratch/GRqHX")
{
    rndseed = rndseed * 1103515245 + 12345;
    return rndseed;
}

F32 xurand()
{
    return 2.3283064e-10f * xrand();
}

U32 xMathSolveQuadratic(F32 a, F32 b, F32 c, F32* x1, F32* x2) NONMATCH("https://decomp.me/scratch/GRqHX")
{
    F32 d, dx, p;
    
    if (a == 0.0f) {
        if (b == 0.0f) {
            return 0;
        }
        *x1 = -c / b;
        return 1;
    }
    
    d = xsqr(b) - 4.0f * a * c;
    if (d < 0.0f) {
        return 0;
    }

    p = 1.0f / (2.0f * a);
    *x1 = -b * p;

    if (d == 0.0f) {
        return 1;
    }

    *x2 = *x1;
    dx = p * xsqrt(d);

    if (a > 0.0f) {
        *x1 -= dx;
        *x2 += dx;
    } else {
        *x1 += dx;
        *x2 -= dx;
    }

    return 2;
}

U32 xMathSolveCubic(F32 a, F32 b, F32 c, F32 d, F32* x1, F32* x2, F32* x3) NONMATCH("https://decomp.me/scratch/GRqHX")
{
    /*
    F32 fA;
    F32 fB;
    F32 fOffset;
    F32 fDiscr;
    F32 fHalfB;
    F32 fTemp;
    F32 fDist;
    F32 fAngle;
    */
    
    if (a == 0.0f) {
        return xMathSolveQuadratic(b, c, d, x1, x2);
    }

    if (a != 1.0f) {
        F32 arecip = 1.0f / a;
        b *= arecip;
        c *= arecip;
        d *= arecip;
    }

    F32 f31 = (1.0f/3.0f) * b;
    F32 f11 = (1.0f/3.0f) * (3.0f * c - b * b);
    F32 f4 = (1.0f/27.0f) * (27.0f * d + (2.0f * (b * b * b) - (9.0f * b * c)));
    F32 f30 = f4 * f4 * 0.25f + (1.0f/27.0f) * (f11 * f11 * f11);
    F32 f28 = 0.5f * f4;

    if (xabs(f30) < 1e-6f) f30 = 0.0f;

    if (f30 > 0.0f) {
        F32 f29 = xsqrt(f30);
        F32 f1 = -f28 + f29;
        if (f1 >= 0.0f) {
            *x1 = xpow(f1, 1.0f/3.0f);
        } else {
            *x1 = -xpow(-f1, 1.0f/3.0f);
        }
        f1 = -f28 - f29;
        if (f1 >= 0.0f) {
            *x1 += xpow(f1, 1.0f/3.0f);
        } else {
            *x1 -= xpow(-f1, 1.0f/3.0f);
        }
        *x1 -= f31;
        return 1;
    }

    if (f30 < 0.0f) {
        F32 f29 = xsqrt(-1.0f/3.0f * f11);
        F32 f28_0 = (1.0f/3.0f) * xatan2(xsqrt(-f30), -f28);
        F32 fCos = icos(f28_0);
        F32 fSin = isin(f28_0);
        F32 f0 = 1.7320508f * fSin;
        *x1 = 2.0f * f29 * fCos - f31;
        *x2 = -f29 * (fCos + f0) - f31;
        *x3 = -f29 * (fCos - f0) - f31;
        return 3;
    }

    F32 f1;
    if (f28 >= 0.0f) {
        f1 = -xpow(f28, 1.0f/3.0f);
    } else {
        f1 = xpow(-f28, 1.0f/3.0f);
    }

    *x1 = 2.0f * f1 - f31;
    *x2 = -f1 - f31;
    return 2;
}

F32 xAngleClamp(F32 a) NONMATCH("https://decomp.me/scratch/GRqHX")
{
    F32 b = xfmod(a, 2*PI);
    if (b < 0.0f) b += 2*PI;
    return b;
}

F32 xAngleClampFast(F32 a)
{
    if (a < 0.0f) a += 2*PI;
    else if (a >= 2*PI) a -= 2*PI;
    return a;
}

F32 xDangleClamp(F32 a) NONMATCH("https://decomp.me/scratch/GRqHX")
{
    F32 b = xfmod(a, 2*PI);
    if (b >= PI) b -= 2*PI;
    else if (b < -PI) b += 2*PI;
    return b;
}

void xAccelMove(F32& x, F32& v, F32 a, F32 dt, F32 endx, F32 maxv) NONMATCH("https://decomp.me/scratch/GRqHX")
{
    F32 offset = endx - x;
    F32 t1 = (xabs(v) < 0.001f ||
             ((v < 0.0f ? 1 : 0) != (offset < 0.0f ? 1 : 0))) ?
             HUGE : (offset / v);
    F32 t2 = xabs(v / a);
    
    if (t1 < t2) a *= -1.0f;
    if (offset < 0.0f) a *= -1.0f;

    F32 oldv = v;
    F32 dv = a * dt;
    F32 newv = v + dv;

    F32 adx;
    if (xabs(newv) <= maxv) {
        v = newv;
        adx = 0.5f * dv * dt;
    } else if (xabs(v) <= maxv) {
        v = range_limit(newv, -maxv, maxv);
        if (oldv != v) {
            F32 diff = v - oldv;
            adx = 0.5f * diff * diff / a;
        } else {
            adx = 0.0f;
        }
    } else if ((dv < 0.0f ? 1 : 0) != (newv < 0.0f ? 1 : 0)) {
        v = newv;
        adx = 0.5f * dv * dt;
    } else {
        adx = 0.0f;
    }
    
    F32 dx = oldv * dt + adx;

    if (t1 > t2) {
        if ((offset < 0.0f ? 1 : 0) == (dx < 0.0f ? 1 : 0) && (xabs(dx) > xabs(offset))) {
            dx = endx;
            v = 0.0f;
        }
    } else {
        if ((offset < 0.0f ? 1 : 0) != (dx < 0.0f ? 1 : 0)) {
            dx = endx;
            v = 0.0f;
        }
    }

    x += dx;
}

F32 xAccelMoveTime(F32 dx, F32 a, F32 minv, F32 maxv) NONMATCH("https://decomp.me/scratch/GRqHX")
{
    /*
    F32 time;
    F32 atime;
    F32 adist;
    */
    
    F32 f5 = maxv / a;
    F32 f1 = dx * 0.5f;
    F32 f0 = 0.5f * a * f5 * f5;
    F32 f1_0;
    if (f0 < f1) {
        f1_0 = xsqrt(2.0f * f1 / a);
    } else {
        f1_0 = f5 + (f1 - f0) / maxv;
    }
    return 2.0f * f1_0;
}

void xAccelMove(F32& x, F32& v, F32 a, F32 dt, F32 maxv)
{
    if (xabs(v) > xabs(maxv)) {
        if (v < 0.0f) {
            if (a > 0.0f) a = -a;
        } else {
            if (a < 0.0f) a = -a;
        }
        a = -a;
    }

    if (a < 0.0f) {
        if (maxv > 0.0f) maxv = -maxv;
    } else {
        if (maxv < 0.0f) maxv = -maxv;
    }

    F32 diff = maxv - v;
    F32 dv = a * dt;
    if (xabs(diff) < xabs(dv)) {
        x += v * dt + 0.5f * diff * diff / a;
        v = maxv;
    } else {
        x += 0.5f * a * dt * dt + v * dt;
        v += dv;
    }
}

void xAccelStop(F32& x, F32& v, F32 a, F32 dt) NONMATCH("https://decomp.me/scratch/GRqHX")
{
    F32 oldv = v;
    
    if (xfeq0(v)) return;

    if (v < 0.0f) {
        if (a > 0.0f) a = -a;
    } else {
        if (a < 0.0f) a = -a;
    }
    a = -a;

    v += a * dt;

    if ((oldv < 0.0f ? 1 : 0) == (v < 0.0f ? 1 : 0)) {
        x += a * (0.5f * dt * dt) + (oldv * dt);
        return;
    }

    if (!xfeq0(a)) {
        x -= 0.5f * oldv * oldv / a;
    }

    v = 0.0f;
}

F32 xFuncPiece_Eval(xFuncPiece* func, F32 param, xFuncPiece** iterator) NONMATCH("https://decomp.me/scratch/GRqHX")
{
    while (func && func->end < param - EPSILON) {
        func = func->next;
    }

    F32 result;
    if (func) {
        result = func->coef[func->order];
        for (S32 i = func->order - 1; i >= 0; i--) {
            result = result * param + func->coef[i];
        }
        if (iterator) {
            *iterator = func;
        }
    } else {
        result = 0.0f;
        if (iterator) {
            *iterator = NULL;
        }
    }
    
    return result;
}

void xFuncPiece_EndPoints(xFuncPiece* func, F32 pi, F32 pf, F32 fi, F32 ff) NONMATCH("https://decomp.me/scratch/GRqHX")
{
    F32 fdiff = ff - fi;
    func->end = pf - pi;
    F32 xfinv = 1.0f / func->end;
    func->order = 1;
    func->coef[0] = fi;
    func->coef[1] = fdiff * xfinv;
    xFuncPiece_ShiftPiece(func, func, -pi);
}

void xFuncPiece_ShiftPiece(xFuncPiece* shift, xFuncPiece* func, F32 newZero) NONMATCH("https://decomp.me/scratch/GRqHX")
{
    S32 i, j;
    xFuncPiece temp;
    
    for (i = 0; i < func->order; i++) {
        temp.coef[i] = 0.0f;
    }

    temp.coef[func->order] = func->coef[func->order];

    for (i = func->order - 1; i >= 0; i--) {
        for (j = i; j < func->order; j++) {
            temp.coef[j] += newZero * temp.coef[j+1];
        }
        temp.coef[i] += func->coef[i];
    }

    shift->order = func->order;

    for (i = 0; i <= shift->order; i++) {
        shift->coef[i] = temp.coef[i];
    }

    shift->end = func->end - newZero;
}
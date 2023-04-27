#include "xSpline.h"

#include "xMemMgr.h"

#include <rwcore.h>
#include <string.h>

static F32 sBasisBezier[4][4] = {
    { -1,  3, -3,  1 },
    {  3, -6,  3,  0 },
    { -3,  3,  0,  0 },
    {  1,  0,  0,  0 }
};

static F32 sBasisHermite[4][4] = {
    {  2, -3,  0,  1 },
    {  1, -2,  1,  0 },
    {  1, -1,  0,  0 },
    { -2,  3,  0,  0 }
};

static void Tridiag_Solve(F32* a, F32* b, F32* c, xVec3* d, xVec3* x, S32 n)
{
    S32 j, idx;
    F32 beta;
    F32* gamma;
    xVec3* delta;

    gamma = (F32*)RwMalloc(n * sizeof(F32));
    delta = (xVec3*)RwMalloc(n * sizeof(xVec3));

    gamma[0] = c[0] / b[0];

    for (idx = 0; idx < 3; idx++) {
        ((F32*)delta)[idx] = ((F32*)d)[idx] / b[0];
    }

    for (j = 1; j < n; j++) {
        beta = b[j] - a[j] * gamma[j-1];
        gamma[j] = c[j] / beta;
        for (idx = 0; idx < 3; idx++) {
            ((F32*)&delta[j])[idx] = (((F32*)&d[j])[idx] - a[j] * ((F32*)&delta[j-1])[idx]) / beta;
        }
    }

    for (idx = 0; idx < 3; idx++) {
        ((F32*)&x[n-1])[idx] = ((F32*)&delta[n-1])[idx];
    }

    for (j = n-2; j >= 0; j--) {
        for (idx = 0; idx < 3; idx++) {
            ((F32*)&x[j])[idx] = ((F32*)&delta[j])[idx] - gamma[j] * ((F32*)&x[j+1])[idx];
        }
    }

    RwFree(gamma);
    RwFree(delta);
}

static void Interpolate_Bspline(xVec3* data, xVec3* control, F32* knots, U32 nodata) NONMATCH("https://decomp.me/scratch/LX0Id")
{
    U32 i;
    F32* alpha, *beta, *gamma;
    F32 t1, t2, t3, t4, t5;

    alpha = (F32*)RwMalloc(nodata * sizeof(F32));
    beta = (F32*)RwMalloc(nodata * sizeof(F32));
    gamma = (F32*)RwMalloc(nodata * sizeof(F32));

    alpha[0] = alpha[nodata-1] = 0.0f;
    beta[0] = beta[nodata-1] = 1.0f;
    gamma[0] = gamma[nodata-1] = 0.0f;

    for (i = 1; i < nodata-1; i++) {
        t1 = knots[i+1];
        t2 = knots[i+2];
        t3 = knots[i+3];
        t4 = knots[i+4];
        t5 = knots[i+5];
        
        alpha[i] = (t4-t3) * (t4-t3) / (t4-t1);
        beta[i] = (t3-t1) * (t4-t3) / (t4-t1) + (t5-t3) * (t3-t2) / (t5-t2);
        gamma[i] = (t3-t2) * (t3-t2) / (t5-t2);
        
        alpha[i] /= (t4-t2);
        beta[i] /= (t4-t2);
        gamma[i] /= (t4-t2);
    }

    Tridiag_Solve(alpha, beta, gamma, data, &control[1], nodata);
    
    control[0] = control[1];
    control[nodata+1] = control[nodata];

    RwFree(alpha);
    RwFree(beta);
    RwFree(gamma);
}

static F32 ArcLength3(xCoef3* coef, F64 ustart, F64 uend) NONMATCH("https://decomp.me/scratch/qvukF")
{
    U32 i;
    F64 A, B, C, D, E, h, sum, u;

    A = 9.0  * ((F64)coef->x.a[0]*coef->x.a[0] + (F64)coef->y.a[0]*coef->y.a[0] + (F64)coef->z.a[0]*coef->z.a[0]);
    B = 12.0 * ((F64)coef->x.a[0]*coef->x.a[1] + (F64)coef->y.a[0]*coef->y.a[1] + (F64)coef->z.a[0]*coef->z.a[1]);
    C = 6.0  * ((F64)coef->x.a[0]*coef->x.a[2] + (F64)coef->y.a[0]*coef->y.a[2] + (F64)coef->z.a[0]*coef->z.a[2]) +
        4.0  * ((F64)coef->x.a[1]*coef->x.a[1] + (F64)coef->y.a[1]*coef->y.a[1] + (F64)coef->z.a[1]*coef->z.a[1]);
    D = 4.0  * ((F64)coef->x.a[1]*coef->x.a[2] + (F64)coef->y.a[1]*coef->y.a[2] + (F64)coef->z.a[1]*coef->z.a[2]);
    E = 1.0  * ((F64)coef->x.a[2]*coef->x.a[2] + (F64)coef->y.a[2]*coef->y.a[2] + (F64)coef->z.a[2]*coef->z.a[2]);
    h = (uend-ustart) / 50.0;
    sum = 0.0;
    u = ustart + h;

    for (i = 2; i <= 50; i++) {
        if (i % 2) {
            sum += 2.0 * sqrt((((A * u + B) * u + C) * u + D) * u + E);
        } else {
            sum += 4.0 * sqrt((((A * u + B) * u + C) * u + D) * u + E);
        }
        u += h;
    }

    return (F32)(h * (sum + sqrt((((A * ustart + B) * ustart + C) * ustart + D) * ustart + E) + 
                            sqrt((((A * uend + B) * uend + C) * uend + D) * uend + E)) / 3.0);
}

static void EvalCoef3(xCoef3* coef, F32 u, U32 deriv, xVec3* o) NONMATCH("https://decomp.me/scratch/dtmNq")
{
    switch (deriv) {
    case 0:
        o->x = ((coef->x.a[0] * u + coef->x.a[1]) * u + coef->x.a[2]) * u + coef->x.a[3];
        o->y = ((coef->y.a[0] * u + coef->y.a[1]) * u + coef->y.a[2]) * u + coef->y.a[3];
        o->z = ((coef->z.a[0] * u + coef->z.a[1]) * u + coef->z.a[2]) * u + coef->z.a[3];
        break;
    case 1:
        o->x = (3.0f * coef->x.a[0] * u + 2.0f * coef->x.a[1]) * u + coef->x.a[2];
        o->y = (3.0f * coef->y.a[0] * u + 2.0f * coef->y.a[1]) * u + coef->y.a[2];
        o->z = (3.0f * coef->z.a[0] * u + 2.0f * coef->z.a[1]) * u + coef->z.a[2];
        break;
    case 2:
        o->x = 6.0f * coef->x.a[0] * u + 2.0f * coef->x.a[1];
        o->y = 6.0f * coef->y.a[0] * u + 2.0f * coef->y.a[1];
        o->z = 6.0f * coef->z.a[0] * u + 2.0f * coef->z.a[1];
        break;
    case 3:
        o->x = 6.0f * coef->x.a[0];
        o->y = 6.0f * coef->y.a[0];
        o->z = 6.0f * coef->z.a[0];
        break;
    default:
        o->x = 0.0f;
        o->y = 0.0f;
        o->z = 0.0f;
        break;
    }
}


static void BasisToCoef3(xCoef3* coef, F32(*N)[4], xVec3* p0, xVec3* p1, xVec3* p2, xVec3* p3)
{
    for (U32 c = 0; c < 4; c++) {
        coef->x.a[c] = N[0][c] * p0->x + N[1][c] * p1->x + N[2][c] * p2->x + N[3][c] * p3->x;
        coef->y.a[c] = N[0][c] * p0->y + N[1][c] * p1->y + N[2][c] * p2->y + N[3][c] * p3->y;
        coef->z.a[c] = N[0][c] * p0->z + N[1][c] * p1->z + N[2][c] * p2->z + N[3][c] * p3->z;
    }
}

static void CoefToUnity3(xCoef3* dest, xCoef3* coef, F32 t1, F32 t2) NONMATCH("https://decomp.me/scratch/3B05J")
{
    U32 i;
    F32 a, b, c, d;
    F32* p, *o;
    F32 dt;

    p = (F32*)coef;
    o = (F32*)dest;
    dt = t2 - t1;

    for (i = 0; i < 3; i++) {
        a = p[0] * dt * dt * dt;
        b = 3.0f * p[0] * dt * dt * t1 + p[1] * dt * dt;
        c = 3.0f * p[0] * dt * t1 * t1 + 2.0f * p[1] * dt * t1 + p[2] * dt;
        d = p[0] * t1 * t1 * t1 + p[1] * t1 * t1 + p[2] * t1 + p[3];

        o[0] = a;
        o[1] = b;
        o[2] = c;
        o[3] = d;

        p += 4;
        o += 4;
    }
}

static void BasisBspline(F32(*N)[4], F32* t) NONMATCH("https://decomp.me/scratch/CtTwz")
{
    U32 i, k, c;
    F32 d1, d2;

    N[0][3] = 0.0f,
        N[1][3] = 0.0f,
        N[2][3] = 0.0f,
        N[3][3] = 0.0f,
        N[4][3] = 0.0f,
        N[5][3] = 0.0f,
        N[6][3] = 0.0f;

    for (k = 2; k <= 4; k++) {
        for (i = 0; i < 8-k; i++) {
            F32 Ntemp[4] = {};
            
            d1 = t[i+k-1] - t[i];
            if (d1 != 0.0f) d1 = 1.0f / d1;

            d2 = t[i+k] - t[i+1];
            if (d2 != 0.0f) d2 = 1.0f / d2;

            for (c = 5-k; c <= 3; c++) {
                Ntemp[c-1] += d1 * N[i][c] - d2 * N[i+1][c];
                Ntemp[c] += d1 * (-t[i] * N[i][c]) + d2 * (t[i+k] * N[i+1][c]);
            }

            N[i][0] = Ntemp[0];
            N[i][1] = Ntemp[1];
            N[i][2] = Ntemp[2];
            N[i][3] = Ntemp[3];
        }
    }
}

static F32 ClampBspline(xSpline3* spl, F32 u)
{
    if (u < 0.0f) u = 0.0f;
    if (u > spl->knot[spl->N+3]) u = spl->knot[spl->N+3];
    return u;
}

static U32 SegBspline(xSpline3* spl, F32 u) NONMATCH("https://decomp.me/scratch/pwLfO")
{
    U32 min, max, mid;

    min = 3;
    max = spl->N + 3;

    while (min + 1 != max) {
        mid = (max+min)/2;
        if (spl->knot[(max+min)/2] >= u) max = mid;
        else min = mid;
    }

    return min - 3;
}

static void EvalBspline3(xSpline3* spl, F32 u, U32 deriv, xVec3* o)
{
    U32 seg;
    F32 N[7][4];
    xCoef3 coef;

    u = ClampBspline(spl, u);
    seg = SegBspline(spl, u);

    BasisBspline(N, &spl->knot[seg]);
    BasisToCoef3(&coef, N, spl->bctrl + seg,
                           spl->bctrl + seg + 1,
                           spl->bctrl + seg + 2,
                           spl->bctrl + seg + 3);
    EvalCoef3(&coef, u, deriv, o);
}

static xCoef3* CoefSeg3(xSpline3* spl, U32 seg, xCoef3* tempCoef)
{
    F32 N[7][4];

    switch (spl->type) {
    case 1:
        return spl->coef + seg;
    case 2:
        BasisToCoef3(tempCoef, sBasisHermite, spl->points + seg,
                                              spl->p12 + seg*2,
                                              spl->p12 + seg*2 + 1,
                                              spl->points + seg + 1);
        break;
    case 3:
        BasisToCoef3(tempCoef, sBasisBezier, spl->points + seg,
                                             spl->p12 + seg*2,
                                             spl->p12 + seg*2 + 1,
                                             spl->points + seg + 1);
        break;
    case 4:
        BasisBspline(N, spl->knot + seg);
        BasisToCoef3(tempCoef, N, spl->bctrl + seg,
                                  spl->bctrl + seg + 1,
                                  spl->bctrl + seg + 2,
                                  spl->bctrl + seg + 3);
        CoefToUnity3(tempCoef, tempCoef, spl->knot[seg+3], spl->knot[seg+4]);
        break;
    }

    return tempCoef;
}

void xSpline3_EvalSeg(xSpline3* spl, F32 u, U32 deriv, xVec3* o) NONMATCH("https://decomp.me/scratch/xsilj")
{
    xCoef3 tempCoef;
    F32 flr;
    U32 seg;

    if (spl->type == 4) {
        EvalBspline3(spl, u, deriv, o);
        return;
    }

    if (u < 0.0f) u = 0.0f;

    flr = std::floorf(u);
    seg = (U32)flr;

    if (seg >= spl->N) {
        u = 1.0f;
        seg = spl->N - 1;
    } else {
        u -= flr;
    }

    switch (spl->type) {
    case 1:
        EvalCoef3(spl->coef + seg, u, deriv, o);
        break;
    case 2:
        BasisToCoef3(&tempCoef, sBasisHermite, spl->points + seg,
                                               spl->p12 + seg*2,
                                               spl->p12 + seg*2 + 1,
                                               spl->points + seg + 1);
        EvalCoef3(&tempCoef, u, deriv, o);
        break;
    case 3:
        BasisToCoef3(&tempCoef, sBasisBezier, spl->points + seg,
                                              spl->p12 + seg*2,
                                              spl->p12 + seg*2 + 1,
                                              spl->points + seg + 1);
        EvalCoef3(&tempCoef, u, deriv, o);
        break;
    }
}

static F32 ArcEvalIterate(xSpline3* spl, F32 s, U32 deriv, xVec3* o, U32 iterations) NONMATCH("https://decomp.me/scratch/3ze86")
{
    xCoef3* coef;
    xCoef3 tempCoef;
    F32 umin, umax, smin, smax, sseg, utest, arctest;
    S32 min, max, test, seg, segmul, sampTot;

    sampTot = spl->arcSample * spl->N;
    min = -1;
    max = sampTot - 1;

    while (min + 1 != max) {
        test = (min+max)>>1;
        if (s > spl->arcLength[test]) min = test;
        else max = test;
    }

    seg = max / (S32)spl->arcSample;
    segmul = seg * spl->arcSample;
    umin = (F32)(max-segmul) / spl->arcSample;
    umax = (F32)(max+1-segmul) / spl->arcSample;
    smin = (max-1 >= 0) ? spl->arcLength[max-1] : 0.0f;
    smax = spl->arcLength[max];
    sseg = (segmul-1 >= 0) ? spl->arcLength[segmul-1] : 0.0f;
    coef = CoefSeg3(spl, seg, &tempCoef);

    if (s <= smin) {
        EvalCoef3(coef, umin, deriv, o);
        return seg + umin;
    }

    if (s >= smax) {
        EvalCoef3(coef, umax, deriv, o);
        return seg + umax;
    }

    s -= sseg;
    smin -= sseg;
    smax -= sseg;

    while (iterations) {
        utest = 0.5f * (umax-umin) + umin;
        arctest = ArcLength3(coef, 0.0, utest);

        if (s > arctest) {
            umin = utest;
            smin = arctest;
        } else {
            umax = utest;
            smax = arctest;
        }

        iterations--;
    }

    if ((smax-smin) == 0.0f) {
        utest = umin;
    } else {
        utest = umin + (umax-umin) * (s-smin) / (smax-smin);
        if (utest < 0.0f) utest = 0.0f;
        else if (utest > 1.0f) utest = 1.0f;
    }

    EvalCoef3(coef, utest, deriv, o);
    return seg + utest;
}

F32 xSpline3_EvalArcApprox(xSpline3* spl, F32 s, U32 deriv, xVec3* o)
{
    if (spl->arcLength) {
        return ArcEvalIterate(spl, s, deriv, o, 0);
    }

    xSpline3_EvalSeg(spl, s, deriv, o);
    return s;
}

void xSpline3_ArcInit(xSpline3* spl, U32 sample) NONMATCH("https://decomp.me/scratch/SSVy3")
{
    U32 i, seg, oldalloc;
    F32 len, arcsum;
    xCoef3 tempCoef;
    xCoef3* coef;

    len = 0.0f;

    if (sample < 1) sample = 1;
    spl->arcSample = sample;

    oldalloc = spl->arcLength ? spl->allocN * spl->arcSample : 0;
    if (oldalloc < spl->N * sample) {
        spl->arcLength = (F32*)xMALLOC(spl->arcSample * spl->allocN * sizeof(F32));
    }

    arcsum = 0.0f;

    for (seg = 0; seg < spl->N; seg++) {
        coef = CoefSeg3(spl, seg, &tempCoef);
        for (i = 0; i < sample; i++) {
            len = ArcLength3(coef, 0.0, (F32)(i+1)/sample);
            spl->arcLength[i+seg*sample] = arcsum + len;
        }
        arcsum += len;
    }
}

static xSpline3* AllocSpline3(xVec3* points, F32* time, U32 numpoints, U32 numalloc, U32 flags, U32 type)
{
    xSpline3* spl = (xSpline3*)xMALLOC(sizeof(xSpline3));

    if (numalloc < numpoints) numalloc = numpoints;

    spl->type = type;
    spl->flags = flags;
    spl->N = numpoints - 1;
    spl->allocN = numalloc - 1;
    spl->p12 = NULL;
    spl->bctrl = NULL;
    spl->knot = NULL;
    spl->coef = NULL;
    spl->arcSample = 0;
    spl->arcLength = NULL;
    
    spl->points = (xVec3*)xMALLOC((spl->allocN + 1) * sizeof(xVec3));
    memcpy(spl->points, points, (spl->N + 1) * sizeof(xVec3));

    if (time) {
        spl->time = (F32*)xMALLOC((spl->allocN + 1) * sizeof(F32));
        memcpy(spl->time, time, (spl->N + 1) * sizeof(F32));
    } else {
        spl->time = NULL;
    }

    return spl;
}

xSpline3* xSpline3_Bezier(xVec3* points, F32* time, U32 numpoints, U32 numalloc, xVec3* p1, xVec3* p2) NONMATCH("https://decomp.me/scratch/9dmnP")
{
    xSpline3* spl = AllocSpline3(points, time, numpoints, numalloc, 0, 3);

    spl->p12 = (xVec3*)xMALLOC(spl->allocN * 2 * sizeof(xVec3));

    if (!p1 || !p2) {
        xSpline3_Catmullize(spl);
    } else {
        for (U32 i = 0; i < spl->N; i++) {
            spl->p12[i*2] = p1[i];
            spl->p12[i*2+1] = p2[i];
        }
    }

    return spl;
}

void xSpline3_Update(xSpline3* spl)
{
    if (spl->type == 4) {
        Interpolate_Bspline(spl->points, spl->bctrl, spl->knot, spl->N + 1);
    }

    if (spl->arcLength) {
        xSpline3_ArcInit(spl, spl->arcSample);
    }
}

void xSpline3_Catmullize(xSpline3* spl)
{
    xSpline3_Update(spl);
}
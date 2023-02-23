#pragma once

#include "iMath3.h"

#include "xMath.h"
#include "xVec3.h"

#include <string.h>

struct xVec4
{
    F32 x;
    F32 y;
    F32 z;
    F32 w;
};

struct xLine3
{
    xVec3 p1;
    xVec3 p2;
};

struct xSphere
{
    xVec3 center;
    F32 r;
};

struct xCylinder
{
    xVec3 center;
    F32 r;
    F32 h;
};

struct xCapsule
{
    xVec3 start;
    xVec3 end;
    F32 r;
};

struct xBox
{
    xVec3 upper;
    xVec3 lower;
};

struct xBBox
{
    xVec3 center;
    xBox box;
};

struct xMat3x3
{
    xVec3 right;
    S32 flags;
    xVec3 up;
    U32 pad1;
    xVec3 at;
    U32 pad2;
};

struct xMat4x3 : xMat3x3
{
    xVec3 pos;
    U32 pad3;
};

struct xQuat
{
    xVec3 v;
    F32 s;
};

struct xRot
{
    xVec3 axis;
    F32 angle;
};

struct xRay3
{
    xVec3 origin;
    xVec3 dir;
    F32 min_t;
    F32 max_t;
    S32 flags;
};

#define XRAY3_USE_MIN (1<<10)
#define XRAY3_USE_MAX (1<<11)

struct xIsect
{
    U32 flags;
    F32 penned;
    F32 contained;
    F32 lapped;
    xVec3 point;
    xVec3 norm;
    F32 dist;
};

#define XISECT_0x1000000 0x1000000
#define XISECT_0x2000000 0x2000000
#define XISECT_0x4000000 0x4000000
#define XISECT_0x8000000 0x8000000
#define XISECT_0x10000000 0x10000000
#define XISECT_0x1F000000 0x1F000000
#define XISECT_0x20000000 0x20000000
#define XISECT_0x40000000 0x40000000
#define XISECT_0x80000000 0x80000000

extern xVec3 g_O3;
extern xVec3 g_X3;
extern xVec3 g_Y3;
extern xVec3 g_Z3;
extern xVec3 g_NX3;
extern xVec3 g_NY3;
extern xVec3 g_NZ3;
extern xVec3 g_Onez;
extern xMat4x3 g_I3;
extern xQuat g_IQ;

void xMath3Init();
void xLine3VecDist2(const xVec3* p1, const xVec3* p2, const xVec3* v, xIsect* isx);
S32 xPointInBox(const xBox* b, const xVec3* p);
void xBoxInitBoundOBB(xBox* o, const xBox* b, const xMat4x3* m);
void xBoxInitBoundCapsule(xBox* b, const xCapsule* c);
void xBoxFromCone(xBox& box, const xVec3& center, const xVec3& dir, F32 dist, F32 r1, F32 r2);
void xMat3x3Normalize(xMat3x3* o, const xMat3x3* m);
void xMat3x3GetEuler(const xMat3x3* m, xVec3* a);
void xMat4x3MoveLocalRight(xMat4x3* m, F32 mag);
void xMat4x3MoveLocalUp(xMat4x3* m, F32 mag);
void xMat4x3MoveLocalAt(xMat4x3* m, F32 mag);
F32 xMat3x3LookVec(xMat3x3* m, const xVec3* at);
void xMat3x3Euler(xMat3x3* m, const xVec3* ypr);
void xMat3x3Euler(xMat3x3* m, F32 yaw, F32 pitch, F32 roll);
void xMat3x3RotC(xMat3x3* m, F32 _x, F32 _y, F32 _z, F32 t);
void xMat3x3RotX(xMat3x3* m, F32 t);
void xMat3x3RotY(xMat3x3* m, F32 t);
void xMat3x3RotZ(xMat3x3* m, F32 t);
void xMat3x3ScaleC(xMat3x3* m, F32 x, F32 y, F32 z);
void xMat3x3RMulRotY(xMat3x3* o, const xMat3x3* m, F32 t);
void xMat3x3Transpose(xMat3x3* o, const xMat3x3* m);
void xMat3x3Mul(xMat3x3* o, const xMat3x3* a, const xMat3x3* b);
void xMat3x3LMulVec(xVec3* o, const xMat3x3* m, const xVec3* v);
void xMat3x3Tolocal(xVec3* o, const xMat3x3* m, const xVec3* v);
void xMat4x3Rot(xMat4x3* m, const xVec3* a, F32 t, const xVec3* p);
void xMat4x3Mul(xMat4x3* o, const xMat4x3* a, const xMat4x3* b);
void xQuatFromMat(xQuat* q, const xMat3x3* m);
void xQuatFromAxisAngle(xQuat* q, const xVec3* a, F32 t);
void xQuatToMat(const xQuat* q, xMat3x3* m);
void xQuatToAxisAngle(const xQuat* q, xVec3* a, F32* t);
F32 xQuatNormalize(xQuat* o, const xQuat* q);
void xQuatSlerp(xQuat* o, const xQuat* a, const xQuat* b, F32 t);
void xQuatMul(xQuat* o, const xQuat* a, const xQuat* b);
void xQuatDiff(xQuat* o, const xQuat* a, const xQuat* b);

inline void xBoxUnion(xBox& box, const xBox& a, const xBox& b)
{
    box.upper.x = xmax(a.upper.x, b.upper.x);
    box.upper.y = xmax(a.upper.y, b.upper.y);
    box.upper.z = xmax(a.upper.z, b.upper.z);
    box.lower.x = xmin(a.lower.x, b.lower.x);
    box.lower.y = xmin(a.lower.y, b.lower.y);
    box.lower.z = xmin(a.lower.z, b.lower.z);
}

inline void xBoxFromLine(xBox& box, const xLine3& line)
{
    box.upper.x = xmax(line.p1.x, line.p2.x);
    box.upper.y = xmax(line.p1.y, line.p2.y);
    box.upper.z = xmax(line.p1.z, line.p2.z);
    box.lower.x = xmin(line.p1.x, line.p2.x);
    box.lower.y = xmin(line.p1.y, line.p2.y);
    box.lower.z = xmin(line.p1.z, line.p2.z);
}

inline void xBoxFromRay(xBox& box, const xRay3& ray)
{
    xLine3 line;

    if (ray.flags & XRAY3_USE_MIN) {
        xVec3 delta;
        delta.x = ray.dir.x * ray.min_t;
        delta.y = ray.dir.y * ray.min_t;
        delta.z = ray.dir.z * ray.min_t;
        line.p1.x = ray.origin.x + delta.x;
        line.p1.y = ray.origin.y + delta.y;
        line.p1.z = ray.origin.z + delta.z;
    } else {
        line.p1.x = ray.origin.x;
        line.p1.y = ray.origin.y;
        line.p1.z = ray.origin.z;
    }

    if (ray.flags & XRAY3_USE_MAX) {
        F32 len;
        if (ray.flags & XRAY3_USE_MIN) {
            len = ray.max_t - ray.min_t;
        } else {
            len = ray.max_t;
        }
        line.p2.x = ray.dir.x * len;
        line.p2.y = ray.dir.y * len;
        line.p2.z = ray.dir.z * len;
    } else {
        line.p2.x = ray.dir.x;
        line.p2.y = ray.dir.y;
        line.p2.z = ray.dir.z;
    }

    line.p2.x = line.p1.x + line.p2.x;
    line.p2.y = line.p1.y + line.p2.y;
    line.p2.z = line.p1.z + line.p2.z;

    xBoxFromLine(box, line);
}

inline void xBoxFromCircle(xBox& box, const xVec3& center, const xVec3& dir, F32 r)
{
    xVec3 var_30 = {};
    var_30.x = r * xsqrt(1.0f - xsqr(dir.x));
    var_30.y = r * xsqrt(1.0f - xsqr(dir.y));
    var_30.z = r * xsqrt(1.0f - xsqr(dir.z));

    box.upper = center + var_30;
    box.lower = center - var_30;
}

inline void xMat3x3Copy(xMat3x3* o, const xMat3x3* m)
{
    memcpy(o, m, sizeof(xMat3x3));
}

inline void xMat3x3SMul(xMat3x3* o, const xMat3x3* m, F32 s)
{
    xVec3SMul(&o->right, &m->right, s);
    xVec3SMul(&o->up, &m->up, s);
    xVec3SMul(&o->at, &m->at, s);
    o->flags = 0;
}

inline void xMat3x3Identity(xMat3x3* m)
{
    xMat3x3Copy(m, &g_I3);
}

inline F32 xMat3x3LookVec3(xMat3x3& mat, const xVec3& at)
{
    F32 mag = at.length();
    if (xfeq0(mag)) {
        mat = (xMat3x3)g_I3;
        return 0.0f;
    }

    mat.at = at;
    mat.at *= 1.0f / mag;

    F32 ax = xabs(mat.at.x);
    F32 ay = xabs(mat.at.y);
    F32 az = xabs(mat.at.z);
    if (ax < ay && ax < az) {
        mat.right.assign(0.0f, mat.at.z, -mat.at.y);
    } else if (ay < az) {
        mat.right.assign(-mat.at.z, 0.0f, mat.at.x);
    } else {
        mat.right.assign(mat.at.y, -mat.at.x, 0.0f);
    }

    mat.right.normalize();
    mat.up = mat.right.cross(mat.at);

    return mag;
}

inline F32 xMat3x3LookAt(xMat3x3* m, const xVec3* from, const xVec3* to)
{
    xVec3 at;
    xVec3Sub(&at, to, from);
    return xMat3x3LookVec(m, &at);
}

inline void xMat3x3Rot(xMat3x3* m, const xVec3* a, F32 t)
{
    xMat3x3RotC(m, a->x, a->y, a->z, t);
}

inline void xMat3x3Scale(xMat3x3* m, const xVec3* s)
{
    xMat3x3ScaleC(m, s->x, s->y, s->z);
}

static inline void xMat3x3RMulVec(xVec3* o, const xMat3x3* m, const xVec3* v)
{
    F32 x = m->right.x * v->x + m->up.x * v->y + m->at.x * v->z;
    F32 y = m->right.y * v->x + m->up.y * v->y + m->at.y * v->z;
    F32 z = m->right.z * v->x + m->up.z * v->y + m->at.z * v->z;
    o->x = x, o->y = y, o->z = z;
}

inline void xMat3x3MulRotC(xMat3x3* o, xMat3x3* m, F32 x, F32 y, F32 z, F32 t)
{
    xMat3x3 temp;
    xMat3x3RotC(&temp, x, y, z, t);
    xMat3x3Mul(o, m, &temp);
}

inline void xMat4x3Copy(xMat4x3* o, const xMat4x3* m)
{
    memcpy(o, m, sizeof(xMat4x3));
}

inline void xMat4x3Identity(xMat4x3* m)
{
    xMat4x3Copy(m, &g_I3);
}

inline void xMat4x3RotC(xMat4x3* m, F32 x, F32 y, F32 z, F32 t)
{
    xMat3x3RotC(m, x, y, z, t);
    xVec3Copy(&m->pos, &g_O3);
}

inline void xMat4x3OrthoInv(xMat4x3* o, const xMat4x3* m)
{
    xMat3x3Transpose(o, m);

    xVec3 tinv;
    xMat3x3RMulVec(&tinv, o, &m->pos);
    xVec3Inv(&o->pos, &tinv);
}

inline void xMat4x3Toworld(xVec3* o, const xMat4x3* m, const xVec3* v)
{
    xMat3x3RMulVec(o, m, v);
    o->x += m->pos.x, o->y += m->pos.y, o->z += m->pos.z;
}

inline void xMat4x3Tolocal(xVec3* o, const xMat4x3* m, const xVec3* v)
{
    o->x = v->x - m->pos.x, o->y = v->y - m->pos.y, o->z = v->z - m->pos.z;
    xMat3x3Tolocal(o, m, o);
}

inline void xQuatCopy(xQuat* o, const xQuat* q)
{
    o->s = q->s;
    o->v.x = q->v.x;
    o->v.y = q->v.y;
    o->v.z = q->v.z;
}

inline U32 xQuatEquals(const xQuat* a, const xQuat* b)
{
    return a->s == b->s && xVec3Equals(&a->v, &b->v);
}

inline void xQuatAdd(xQuat* o, const xQuat* a, const xQuat* b)
{
    o->s = a->s + b->s;
    xVec3Add(&o->v, &a->v, &b->v);
}

inline void xQuatSMul(xQuat* o, const xQuat* q, F32 s)
{
    o->s = q->s * s;
    xVec3SMul(&o->v, &q->v, s);
}

inline F32 xQuatDot(const xQuat* a, const xQuat* b)
{
    return a->s * b->s + xVec3Dot(&a->v, &b->v);
}

inline F32 xQuatLength2(const xQuat* q)
{
    return xQuatDot(q, q);
}

inline F32 xQuatGetAngle(const xQuat* q)
{
    if (q->s > 0.99999f) return 0.0f;
    if (q->s < -0.99999f) return 2*PI;
    return 2.0f * xacos(q->s);
}

inline void xQuatConj(xQuat* o, const xQuat* q)
{
    o->s = q->s;
    xVec3Inv(&o->v, &q->v);
}

inline void xQuatFlip(xQuat* o, const xQuat* q)
{
    o->s = -q->s;
    xVec3Inv(&o->v, &q->v);
}

inline void xRotCopy(xRot* o, const xRot* r)
{
    o->axis.x = r->axis.x;
    o->axis.y = r->axis.y;
    o->axis.z = r->axis.z;
    o->angle = r->angle;
}
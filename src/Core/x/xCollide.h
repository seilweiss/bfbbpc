#pragma once

#include "iCollide.h"

#include "xQuickCull.h"

#include <rwcore.h>
#include <rpcollis.h>
#include <rpcollbsptree.h>

struct xEnt;
struct xScene;

union xiMat4x3Union
{
    xMat4x3 xm;
    RwMatrix im;
};

struct xCollis
{
    struct tri_data
    {
        U32 index;
        F32 r;
        F32 d;
    };

    U32 flags;
    U32 oid;
    void* optr;
    xModelInstance* mptr;
    F32 dist;
    xVec3 norm;
    xVec3 tohit;
    xVec3 depen;
    xVec3 hdng;
    union
    {
        struct
        {
            F32 t;
            F32 u;
            F32 v;
        } tuv;
        tri_data tri;
    };
};

#define k_HIT_IT ((U32)(1 << 0))
#define k_HIT_0x2 ((U32)(1 << 1))
#define k_HIT_0x4 ((U32)(1 << 2))
#define k_HIT_0x8 ((U32)(1 << 3))
#define k_HIT_0x10 ((U32)(1 << 4))
#define k_HIT_0x100 ((U32)(1 << 8))
#define k_HIT_0x200 ((U32)(1 << 9))
#define k_HIT_0x400 ((U32)(1 << 10))
#define k_HIT_0x800 ((U32)(1 << 11))
#define k_HIT_0xF00 (k_HIT_0x100 | k_HIT_0x200 | k_HIT_0x400 | k_HIT_0x800)
#define k_HIT_CALC_HDNG ((U32)(1 << 12))
#define k_HIT_CALC_TRI ((U32)(1 << 13))
#define k_HIT_0x20000 ((U32)(1 << 17))

struct xParabola
{
    xVec3 initPos;
    xVec3 initVel;
    F32 gravity;
    F32 minTime;
    F32 maxTime;
};

struct xSweptSphere
{
    xVec3 start;
    xVec3 end;
    F32 radius;
    F32 dist;
    xiMat4x3Union basis;
    xiMat4x3Union invbasis;
    xBox box;
    xQCData qcd;
    F32 boxsize;
    F32 curdist;
    xVec3 contact;
    xVec3 polynorm;
    U32 oid;
    void* optr;
    xModelInstance* mptr;
    S32 hitIt;
    xVec3 worldPos;
    xVec3 worldContact;
    xVec3 worldNormal;
    xVec3 worldTangent;
    xVec3 worldPolynorm;
};

typedef enum _xCollsIdx
{
    k_XCOLLS_IDX_FLOOR,
    k_XCOLLS_IDX_CEIL,
    k_XCOLLS_IDX_FRONT,
    k_XCOLLS_IDX_LEFT,
    k_XCOLLS_IDX_REAR,
    k_XCOLLS_IDX_RIGHT,
    k_XCOLLS_IDX_COUNT
} xCollsIdx;

xCollsIdx xCollideGetCollsIdx(const xCollis* coll, const xVec3* tohit, const xMat3x3* mat);
void xCollideInit(xScene* sc);
U32 xSphereHitsSphere(const xSphere* a, const xSphere* b, xCollis* coll);
U32 xSphereHitsBox(const xSphere* a, const xBox* b, xCollis* coll);
U32 xSphereHitsOBB_nu(const xSphere* s, const xBox* b, const xMat4x3* m, xCollis* coll);
U32 xSphereHitsModel(const xSphere* b, const xModelInstance* m, xCollis* coll);
void xParabolaRecenter(xParabola* p, F32 newZeroT);
S32 xParabolaHitsEnv(xParabola* p, const xEnv* env, xCollis* colls);
U32 xBoxHitsSphere(const xBox* a, const xSphere* b, xCollis* coll);
U32 xBoxHitsObb(const xBox* a, const xBox* b, const xMat4x3* mat, xCollis* coll);
void xCollideCalcTri(xCollis::tri_data& tri, const xModelInstance& model, const xVec3& center, const xVec3& heading);
xVec3 xCollisTriHit(const xCollis::tri_data& tri, const xModelInstance& model);
RpCollBSPTree* _rpCollBSPTreeForAllCapsuleLeafNodeIntersections(RpCollBSPTree* tree, RwLine* line, F32 radius, RpV3dGradient* grad, S32(*callBack)(S32, S32, void*), void* data);
void xSweptSpherePrepare(xSweptSphere* sws, xVec3* start, xVec3* end, F32 radius);
void xSweptSphereGetResults(xSweptSphere* sws);
S32 xSweptSphereToTriangle(xSweptSphere* sws, xVec3* v0, xVec3* v1, xVec3* v2);
S32 xSweptSphereToSphere(xSweptSphere* sws, xSphere* sph);;
S32 xSweptSphereToBox(xSweptSphere* sws, xBox* box, xMat4x3* mat);
S32 xSweptSphereToEnv(xSweptSphere* sws, xEnv* env);
S32 xSweptSphereToModel(xSweptSphere* sws, RpAtomic* model, RwMatrix* mat);
S32 xSweptSphereToScene(xSweptSphere* sws, xScene* sc, xEnt* mover, U8 collType);
S32 xSweptSphereToStatDyn(xSweptSphere* sws, xScene* sc, xEnt* mover, U8 collType);
S32 xSweptSphereToNPC(xSweptSphere* sws, xScene* sc, xEnt* mover, U8 collType);
bool xSphereHitsCapsule(const xVec3& center, F32 radius, const xVec3& v1, const xVec3& v2, F32 width);
bool xSphereHitsBound(const xSphere& o, const xBound& b);
bool xSphereHitsVCylinder(const xVec3& sc, F32 sr, const xVec3& cc, F32 cr, F32 ch);
bool xSphereHitsVCircle(const xVec3& sc, F32 sr, const xVec3& cc, F32 cr);

inline void xParabolaEvalPos(const xParabola* p, xVec3* pos, F32 time)
{
    xVec3Copy(pos, &p->initPos);
    xVec3AddScaled(pos, &p->initVel, time);
    pos->y -= 0.5f * p->gravity * time * time;
}

inline void xParabolaEvalVel(const xParabola* p, xVec3* vel, F32 time)
{
    xVec3Copy(vel, &p->initVel);
    vel->y -= p->gravity * time;
}

inline bool xSphereHitsSphere(const xVec3& loc1, F32 r1, const xVec3& loc2, F32 r2)
{
    F32 dist2 = (loc2 - loc1).length2();
    F32 max_dist = r1 + r2;
    return dist2 <= xsqr(max_dist);
}

inline bool xSphereHitsSphere(const xSphere& o1, const xSphere& o2)
{
    return xSphereHitsSphere(o1.center, o1.r, o2.center, o2.r);
}

inline bool xSphereHitsBox(const xVec3& c, F32 r, const xBox& b)
{
    return c.x + r >= b.lower.x &&
        c.y + r >= b.lower.y &&
        c.z + r >= b.lower.z &&
        c.x - r <= b.upper.x &&
        c.y - r <= b.upper.y &&
        c.z - r <= b.upper.z;
}

inline bool xSphereHitsBox(const xSphere& o, const xBox& b) WIP
{
    return xSphereHitsBox(o.center, o.r, b);
}

inline bool xSphereHitsOBB(const xVec3& c, F32 r, const xBox& b, const xMat4x3& mat)
{
    xVec3 lc;
    xMat4x3Tolocal(&lc, &mat, &c);
    return xSphereHitsBox(lc, r, b);
}

inline bool xSphereHitsOBB(const xSphere& o, const xBox& b, const xMat4x3& mat) WIP
{
    return xSphereHitsOBB(o.center, o.r, b, mat);
}

inline bool xSphereHitsVCircle(const xSphere& o, const xVec3& cc, F32 cr)
{
    return xSphereHitsVCircle(o.center, o.r, cc, cr);
}
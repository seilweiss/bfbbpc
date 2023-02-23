#include "iCollide.h"

#include "iTime.h"

#include "xCollide.h"
#include "xClumpColl.h"
#include "xEnv.h"
#include "xModel.h"

static S32 sCollidingJSP;
static F32 cbath;
static xRay3 cbray;
static const xMat3x3* cbmat;
static RpIntersection cbisx_local;
static U8 cbnumcs;
static U8 cbmaxcs;
static U8 FLOOR;
static U8 NEXT2;
static U8 OTHER;
iTime collide_rwtime;
S32 collide_rwct;
F32 collide_rwtime_secs;

void iCollideInit(xScene* sc) {}

enum Dimension
{
    XDIM,
    YDIM,
    ZDIM
};

S32 PointWithinTriangle(xVec3* _pt, xVec3** _tri, xVec3* _normal)
{
    RwV3d* pt = (RwV3d*)_pt;
    RwV3d** tri = (RwV3d**)_tri;
    RwV3d* normal = (RwV3d*)_normal;
    Dimension dimension;
    F32 absX = xabs(normal->x);
    F32 absY = xabs(normal->y);
    F32 absZ = xabs(normal->z);
    S32 inside = 0;
    S32 i;
    S32 j;

    dimension = (absZ > absY ? (absZ > absX ? ZDIM : XDIM) : (absY > absX ? YDIM : XDIM));
    switch (dimension) {
    case XDIM:
        for (i = 0, j = 2; i < 3; j = i++) {
            if ((tri[i]->y <= pt->y && pt->y < tri[j]->y) ||
                (tri[j]->y <= pt->y && pt->y < tri[i]->y)) {
                if (pt->z < tri[i]->z + ((pt->y - tri[i]->y) * (tri[j]->z - tri[i]->z)) / (tri[j]->y - tri[i]->y)) {
                    inside = !inside;
                }
            }
        }
        break;
    case YDIM:
        for (i = 0, j = 2; i < 3; j = i++) {
            if ((tri[i]->z <= pt->z && pt->z < tri[j]->z) ||
                (tri[j]->z <= pt->z && pt->z < tri[i]->z)) {
                if (pt->x < tri[i]->x + ((pt->z - tri[i]->z) * (tri[j]->x - tri[i]->x)) / (tri[j]->z - tri[i]->z)) {
                    inside = !inside;
                }
            }
        }
        break;
    case ZDIM:
        for (i = 0, j = 2; i < 3; j = i++) {
            if ((tri[i]->y <= pt->y && pt->y < tri[j]->y) ||
                (tri[j]->y <= pt->y && pt->y < tri[i]->y)) {
                if (pt->x < tri[i]->x + ((pt->y - tri[i]->y) * (tri[j]->x - tri[i]->x)) / (tri[j]->y - tri[i]->y)) {
                    inside = !inside;
                }
            }
        }
        break;
    }

    return inside;
}

void FindNearestPointOnLine(xVec3* _result, xVec3* _point, xVec3* _start, xVec3* _end)
{
    RwV3d* result = (RwV3d*)_result;
    RwV3d* point = (RwV3d*)_point;
    RwV3d* start = (RwV3d*)_start;
    RwV3d* end = (RwV3d*)_end;
    F32 mu;
    RwV3d line;
    RwV3d candidate;
    
    RwV3dSub(&line, end, start);
    mu = RwV3dDotProduct(point, &line) - RwV3dDotProduct(start, &line);

    if (mu <= 0.0f) {
        candidate = *start;
    } else {
        F32 lineLength2 = RwV3dDotProduct(&line, &line);
        if (mu < lineLength2) {
            mu /= lineLength2;
            RwV3dScale(&candidate, &line, mu);
            RwV3dAdd(&candidate, &candidate, start);
        } else {
            candidate = *end;
        }
    }

    *result = candidate;
}

static void properSphereIsectTri(const xVec3* center, F32 radius, xVec3* tohit, F32* dist_ptr, RpCollisionTriangle* tri)
{
    xVec3 projPoint;
    F32 dist2plane;
    F32 dist;
    F32 dist2;
    F32 radius2;
    U32 i;
    xVec3 vertClosestPoint;
    F32 vertDist2;
    xVec3 temp;

    dist = *dist_ptr;
    dist2plane = xVec3Dot((xVec3*)&tri->normal, (xVec3*)&tri->point) - xVec3Dot((xVec3*)&tri->normal, center);

    xVec3SMul(&projPoint, (xVec3*)&tri->normal, dist2plane);
    xVec3Copy(tohit, &projPoint);
    xVec3AddTo(&projPoint, center);

    if (PointWithinTriangle(&projPoint, (xVec3**)tri->vertices, (xVec3*)&tri->normal)) {
        dist = xabs(dist2plane);
    } else {
        dist2 = xsqr(dist);
        radius2 = xsqr(radius);
        for (i = 0; i < 3; i++) {
            FindNearestPointOnLine(&vertClosestPoint, &projPoint, (xVec3*)tri->vertices[i], (xVec3*)tri->vertices[(i+1)%3]);
            xVec3Sub(&temp, &vertClosestPoint, center);
            vertDist2 = xVec3Length2(&temp);
            if (vertDist2 < dist2 && vertDist2 < radius2) {
                dist2 = vertDist2;
                xVec3Copy(tohit, &temp);
            }
        }
        dist = xsqrt(dist2);
    }

    *dist_ptr = dist;
}

RpCollisionTriangle* sphereHitsEnvCB(RpIntersection* isx, RpWorldSector* sector, RpCollisionTriangle* tri, RwReal dist, void* data)
{
    xCollis* coll = (xCollis*)data;
    F32 radius = isx->t.sphere.radius;
    xVec3* center = (xVec3*)&isx->t.sphere.center;
    xVec3 tohit;
    F32 scale;

    if (coll->flags == 0) {
        coll->flags |= k_HIT_IT;
        return NULL;
    }

    dist = HUGE;
    properSphereIsectTri(center, radius, &tohit, &dist, tri);

    if (dist < 0.0f) return tri;
    if (dist == 0.0f) return tri;
    if (dist >= radius) return tri;
    if (dist >= coll->dist) return tri;

    coll->dist = dist;
    if (coll->flags & k_HIT_0x200) {
        xVec3Copy(&coll->norm, (xVec3*)&tri->normal);
    }
    if (coll->flags & k_HIT_0x400) {
        xVec3Copy(&coll->tohit, &tohit);
    }
    if (coll->flags & k_HIT_0x800) {
        scale = 1.0f - radius / coll->dist;
        scale -= EPSILON;
        xVec3Copy(&coll->depen, &tohit);
        xVec3SMulBy(&coll->depen, scale);
    }
    if (coll->flags & k_HIT_CALC_HDNG) {
        xVec3Normalize(&coll->hdng, &tohit);
    }

    coll->flags |= k_HIT_IT;
    return tri;
}

#define COS45 0.70710678118f

static RpCollisionTriangle* sphereHitsEnv3CB(RpIntersection* isx, RpWorldSector* sector, RpCollisionTriangle* tri, RwReal dist, void* data) NONMATCH("https://decomp.me/scratch/9mFeH")
{
    xCollis* colls;
    F32 radius;
    xVec3* center;
    xVec3 tohit;
    xVec3 hdng;
    U8 idx;
    F32 dot;
    F32 odot;
    F32 ndot;
    F32 scale;

    colls = (xCollis*)data;
    radius = isx->t.sphere.radius;
    center = (xVec3*)&isx->t.sphere.center;
    dist = HUGE;
    properSphereIsectTri(center, radius, &tohit, &dist, tri);

    if (dist == 0.0f) return tri;
    if (dist > radius) return tri;

    if (xsqr(tohit.y) > xsqr(tohit.x) + xsqr(tohit.z)) {
        if (FLOOR == 255) {
            idx = cbnumcs++;
            FLOOR = idx;
        } else if (tohit.y < 0.0f) {
            if (colls[FLOOR].hdng.y > 0.0f ||
                dist < colls[FLOOR].dist ||
                (xabs(dist - colls[FLOOR].dist) < 0.001f && tri->normal.y > colls[FLOOR].norm.y)) {
                idx = FLOOR;
            } else {
                return tri;
            }
        } else {
            if (colls[FLOOR].hdng.y > 0.0f &&
                (dist < colls[FLOOR].dist ||
                    (xabs(dist - colls[FLOOR].dist) < 0.001f && tri->normal.y > colls[FLOOR].norm.y))) {
                idx = FLOOR;
            } else {
                return tri;
            }
        }

        xVec3SMul(&hdng, &tohit, 1.0f / dist);
    } else if (NEXT2 == 255) {
        if (OTHER == 255) {
            idx = cbnumcs++;
            OTHER = idx;
            xVec3SMul(&hdng, &tohit, 1.0f / dist);
        } else {
            xVec3SMul(&hdng, &tohit, 1.0f / dist);
            dot = xVec3Dot(&hdng, &colls[OTHER].hdng);
            if (xabs(dot) > COS45) {
                if (dist < colls[OTHER].dist) {
                    idx = OTHER;
                } else {
                    return tri;
                }
            } else {
                idx = cbnumcs++;
                NEXT2 = idx;
                if (dist < colls[OTHER].dist) {
                    idx = OTHER;
                    OTHER = NEXT2;
                    NEXT2 = idx;
                    idx = OTHER;
                }
            }
        }
    } else {
        if (dist > colls[NEXT2].dist) {
            return tri;
        }

        xVec3SMul(&hdng, &tohit, 1.0f / dist);
        odot = xVec3Dot(&hdng, &colls[OTHER].hdng);
        if (dist < colls[OTHER].dist) {
            if (xabs(odot) > COS45) {
                ndot = xVec3Dot(&hdng, &colls[NEXT2].hdng);
                if (xabs(ndot) > COS45) {
                    if (NEXT2 < OTHER) {
                        OTHER = NEXT2;
                    }
                    idx = OTHER;
                    NEXT2 = 255;
                    cbnumcs--;
                    colls[cbnumcs].flags &= ~k_HIT_IT;
                    colls[cbnumcs].dist = HUGE;
                } else {
                    idx = OTHER;
                }
            } else {
                idx = NEXT2;
            }
        } else if (xabs(odot) > COS45) {
            return tri;
        } else {
            idx = NEXT2;
        }
    }

    colls[idx].dist = dist;
    xVec3Copy(&colls[idx].hdng, &hdng);

    if (colls[0].flags & k_HIT_0x200) {
        xVec3Copy(&colls[idx].norm, (xVec3*)&tri->normal);
    }
    if (colls[0].flags & k_HIT_0x400) {
        xVec3Copy(&colls[idx].tohit, &tohit);
    }
    if (colls[0].flags & k_HIT_0x800) {
        scale = dist - radius;
        xVec3Copy(&colls[idx].depen, &hdng);
        xVec3SMulBy(&colls[idx].depen, scale);
    }
    
    colls[idx].flags |= k_HIT_IT;

    if (sector) {
        colls[idx].oid = sector->polygons[tri->index].matIndex;
    } else if (sCollidingJSP) {
        colls[idx].oid = ((xClumpCollBSPTriangle*)tri->index)->matIndex;
    } else {
        colls[idx].oid = tri->index;
    }
    colls[idx].tri.index = tri->index;

    if (cbnumcs == cbmaxcs) return NULL;

    return tri;
}

static RpCollisionTriangle* sphereHitsEnv4CB(RpIntersection* isx, RpWorldSector* sector, RpCollisionTriangle* tri, RwReal dist, void* data) NONMATCH("https://decomp.me/scratch/GxUhu")
{
    xCollis* colls;
    F32 radius;
    xVec3* center;
    xVec3 tohit;
    xCollis temp;
    U8 idx;
    xCollis* c;
    F32 ddist;

    colls = (xCollis*)data;
    radius = isx->t.sphere.radius;
    center = (xVec3*)&isx->t.sphere.center;
    dist = HUGE;
    properSphereIsectTri(center, radius, &tohit, &dist, tri);

    if (dist == 0.0f) return tri;
    if (dist > radius) return tri;

    if (sCollidingJSP) {
        if (((xClumpCollBSPTriangle*)tri->index)->flags & 0x10) {
            temp.flags = k_HIT_0x20000;
        } else {
            temp.flags = 0;
        }
    } else {
        temp.flags = 0;
    }
    temp.optr = NULL;
    idx = xCollideGetCollsIdx(&temp, &tohit, cbmat);
    c = colls + idx;
    ddist = dist - c->dist;
    if (ddist < 0.0f || (ddist < 0.001f && c->norm.y > tri->normal.y)) {
    } else {
        return tri;
    }

    c->dist = dist;
    c->tohit = tohit;
    c->norm.x = tri->normal.x;
    c->norm.y = tri->normal.y;
    c->norm.z = tri->normal.z;
    c->tri.index = tri->index;
    c->flags |= k_HIT_IT;

    if (sCollidingJSP) {
        if (((xClumpCollBSPTriangle*)tri->index)->flags & 0x10) {
            c->flags |= k_HIT_0x20000;
        }
    }

    if (sector) {
        c->oid = sector->polygons[tri->index].matIndex;
    } else if (sCollidingJSP) {
        c->oid = ((xClumpCollBSPTriangle*)tri->index)->matIndex;
    } else {
        c->oid = tri->index;
    }

    return tri;
}

static RpCollisionTriangle* sphereHitsModel3CB(RpIntersection* isx, RpCollisionTriangle* tri, RwReal dist, void* data)
{
    return sphereHitsEnv3CB(&cbisx_local, NULL, tri, dist, data);
}

static RpCollisionTriangle* rayHitsEnvCB(RpIntersection* isx, RpWorldSector* sector, RpCollisionTriangle* tri, RwReal dist, void* data)
{
    xCollis* coll = (xCollis*)data;

    dist *= cbray.max_t;
    if (dist >= coll->dist) return tri;

    if (sector) {
        coll->oid = sector->polygons[tri->index].matIndex;
    } else if (sCollidingJSP) {
        coll->oid = ((xClumpCollBSPTriangle*)tri->index)->matIndex;
        if (((xClumpCollBSPTriangle*)tri->index)->flags & 0x10) {
            coll->flags |= k_HIT_0x20000;
        } else {
            coll->flags &= ~k_HIT_0x20000;
        }
    } else {
        coll->oid = tri->index;
    }

    coll->dist = dist;
    coll->flags |= k_HIT_IT;
    coll->tri.index = tri->index;
    if (coll->flags & k_HIT_0x200) {
        coll->norm = *(xVec3*)&tri->normal;
    }

    return tri;
}

static RpCollisionTriangle* rayHitsEnvBackwardCB(RpIntersection* isx, RpWorldSector* sector, RpCollisionTriangle* tri, RwReal dist, void* data)
{
    xCollis* coll = (xCollis*)data;

    dist = 1.0f - dist;
    dist *= cbray.max_t;
    if (dist >= coll->dist) return tri;

    if (sector) {
        coll->oid = sector->polygons[tri->index].matIndex;
    } else if (sCollidingJSP) {
        coll->oid = ((xClumpCollBSPTriangle*)tri->index)->matIndex;
    } else {
        coll->oid = tri->index;
    }

    coll->dist = dist;
    coll->flags |= k_HIT_IT;
    coll->tri.index = tri->index;

    return tri;
}

static RpCollisionTriangle* rayHitsModelCB(RpIntersection* isx, RpCollisionTriangle* tri, RwReal dist, void* data)
{
    return rayHitsEnvCB(isx, NULL, tri, dist, data);
}

static RpCollisionTriangle* rayHitsModelBackwardCB(RpIntersection* isx, RpCollisionTriangle* tri, RwReal dist, void* data)
{
    return rayHitsEnvBackwardCB(isx, NULL, tri, dist, data);
}

U32 iSphereHitsEnv(const xSphere* b, const xEnv* env, xCollis* coll) NONMATCH("https://decomp.me/scratch/locZH")
{
    RpIntersection isx;
    isx.type = rpINTERSECTSPHERE;

    memcpy(&isx.t.sphere, (RwSphere*)b, sizeof(RwSphere));

    coll->flags &= ~k_HIT_IT;
    coll->dist = HUGE;

    if (env->geom->jsp) {
        sCollidingJSP = 1;
        xClumpColl_ForAllIntersections(env->geom->jsp->colltree, &isx, sphereHitsEnvCB, coll);
        sCollidingJSP = 0;
    } else {
        RpCollisionWorldForAllIntersections(env->geom->world, &isx, sphereHitsEnvCB, coll);
        if (env->geom->collision) {
            RpCollisionWorldForAllIntersections(env->geom->collision, &isx, sphereHitsEnvCB, coll);
        }
    }

    return coll->flags & k_HIT_IT;
}

S32 iSphereHitsEnv3(const xSphere* b, const xEnv* env, xCollis* colls, U8 ncolls, F32 sth) NONMATCH("https://decomp.me/scratch/rAKfe")
{
    RpIntersection isx;
    isx.type = rpINTERSECTSPHERE;

    memcpy(&isx.t.sphere, (RwSphere*)b, sizeof(RwSphere));
    
    U8 idx = 0;
    while (idx < ncolls) {
        colls[idx].flags &= ~k_HIT_IT;
        colls[idx].dist = HUGE;
        idx++;
    }

    cbath = sth;
    cbnumcs = 0;
    cbmaxcs = ncolls;
    FLOOR = NEXT2 = OTHER = 0xFF;

    if (env->geom->jsp) {
        sCollidingJSP = 1;
        xClumpColl_ForAllIntersections(env->geom->jsp->colltree, &isx, sphereHitsEnv3CB, colls);
        sCollidingJSP = 0;
    } else {
        RpCollisionWorldForAllIntersections(env->geom->world, &isx, sphereHitsEnv3CB, colls);
        if (env->geom->collision) {
            RpCollisionWorldForAllIntersections(env->geom->collision, &isx, sphereHitsEnv3CB, colls);
        }
    }

    return cbnumcs;
}

S32 iSphereHitsEnv4(const xSphere* b, const xEnv* env, const xMat3x3* mat, xCollis* colls) NONMATCH("https://decomp.me/scratch/hNEbf")
{
    RpIntersection isx;
    isx.type = rpINTERSECTSPHERE;

    memcpy(&isx.t.sphere, (RwSphere*)b, sizeof(RwSphere));

    cbmat = mat;
    
    xCollis* c = colls;
    xCollis* cend = colls + 6;

    if (env->geom->jsp) {
        sCollidingJSP = 1;
        xClumpColl_ForAllIntersections(env->geom->jsp->colltree, &isx, sphereHitsEnv4CB, colls);
        sCollidingJSP = 0;
    } else {
        RpCollisionWorldForAllIntersections(env->geom->world, &isx, sphereHitsEnv4CB, colls);
    }

    for (c = colls; c < cend; c++) {
        if (c->flags & k_HIT_IT) {
            c->oid += 0x10000;
        }
    }

    if (env->geom->collision) {
        RpCollisionWorldForAllIntersections(env->geom->collision, &isx, sphereHitsEnv4CB, colls);
        for (c = colls; c < cend; c++) {
            if (c->flags & k_HIT_IT) {
                if (c->oid < 0x10000) {
                    c->oid += 0x10000;
                } else {
                    c->oid -= 0x10000;
                }
            }
        }
    }
    
    S32 numcs = 0;
    for (c = colls; c < cend; c++) {
        if (c->flags & k_HIT_IT) {
            numcs++;
            
            F32 s = 1.0f / c->dist;
            c->hdng.x = c->tohit.x * s;
            c->hdng.y = c->tohit.y * s;
            c->hdng.z = c->tohit.z * s;
            
            F32 f2 = xmin(0.0f, c->dist - b->r);
            c->depen.x = c->hdng.x * f2;
            c->depen.y = c->hdng.y * f2;
            c->depen.z = c->hdng.z * f2;

            if (xVec3Dot(&c->hdng, &c->norm) > 0.0f) {
                c->norm.x = -c->norm.x;
                c->norm.y = -c->norm.y;
                c->norm.z = -c->norm.z;
            }
        }
    }

    return numcs;
}

S32 iSphereHitsModel3(const xSphere* b, const xModelInstance* m, xCollis* colls, U8 ncolls, F32 sth) NONMATCH("https://decomp.me/scratch/qyJ42")
{
    RpIntersection isx;
    U8 idx;
    U8 i;
    
    if (m->Flags & 0x800) {
        xModelAnimCollApply(*m);
    }

    for (i = 0; i < ncolls; i++) {
        if (colls[i].flags & k_HIT_CALC_TRI) {
            colls[i].flags |= k_HIT_0x400;
        }
    }

    isx.type = rpINTERSECTSPHERE;

    memcpy(&isx.t.sphere, (RwSphere*)b, sizeof(RwSphere));

    xMat4x3* mat = (xMat4x3*)m->Mat;
    RwFrame* frame = RpAtomicGetFrame(m->Data);
    RwFrameTransform(frame, (RwMatrix*)mat, rwCOMBINEREPLACE);

    F32 mscale = xVec3Length(&mat->right);

    xMat3x3 mnormal;
    xMat3x3Normalize(&mnormal, mat);
    xMat4x3Tolocal((xVec3*)&cbisx_local.t, mat, &b->center);

    cbisx_local.t.sphere.radius = b->r / mscale;

    for (idx = 0; idx < ncolls; idx++) {
        colls[idx].flags &= ~k_HIT_IT;
        colls[idx].dist = HUGE;
    }

    cbath = sth;
    cbnumcs = 0;
    cbmaxcs = ncolls;
    FLOOR = NEXT2 = OTHER = 0xFF;
    
    iTime t0 = iTimeGet();
    RpAtomicForAllIntersections(m->Data, &isx, sphereHitsModel3CB, colls);
    iTime t1 = iTimeGet();

    collide_rwtime += t1 - t0;
    collide_rwct++;
    collide_rwtime_secs = iTimeDiffSec(collide_rwtime) / collide_rwct;

    if (colls[0].flags & (k_HIT_0xF00 | k_HIT_CALC_HDNG)) {
        for (idx = 0; idx < cbnumcs; idx++) {
            if (colls[idx].flags & k_HIT_IT) {
                if (colls[idx].flags & k_HIT_CALC_TRI) {
                    xCollideCalcTri(colls[idx].tri, *m, *(xVec3*)&cbisx_local.t.sphere.center, colls[idx].tohit);
                }
                xMat3x3RMulVec(&colls[idx].tohit, mat, &colls[idx].tohit);
                xMat3x3RMulVec(&colls[idx].depen, mat, &colls[idx].depen);
                xMat3x3RMulVec(&colls[idx].hdng, &mnormal, &colls[idx].hdng);
                xMat3x3RMulVec(&colls[idx].norm, &mnormal, &colls[idx].norm);
                colls[idx].dist *= mscale;
            }
        }
    }

    if (m->Flags & 0x800) {
        xModelAnimCollRestore(*m);
    }

    return cbnumcs;
}

U32 iRayHitsEnv(const xRay3* r, const xEnv* env, xCollis* coll) NONMATCH("https://decomp.me/scratch/5Xy01")
{
    RpIntersection isx;
    isx.type = rpINTERSECTLINE;

    if (r->flags & XRAY3_USE_MIN) {
        xVec3 delta;
        delta.x = r->dir.x * r->min_t,
            delta.y = r->dir.y * r->min_t,
            delta.z = r->dir.z * r->min_t;
        isx.t.line.start.x = r->origin.x + delta.x,
            isx.t.line.start.y = r->origin.y + delta.y,
            isx.t.line.start.z = r->origin.z + delta.z;
    } else {
        isx.t.line.start.x = r->origin.x,
            isx.t.line.start.y = r->origin.y,
            isx.t.line.start.z = r->origin.z;
    }

    if (r->flags & XRAY3_USE_MAX) {
        F32 len;
        if (r->flags & XRAY3_USE_MIN) {
            len = r->max_t - r->min_t;
        } else {
            len = r->max_t;
        }
        isx.t.line.end.x = r->dir.x * len,
            isx.t.line.end.y = r->dir.y * len,
            isx.t.line.end.z = r->dir.z * len;
    } else {
        isx.t.line.end.x = r->dir.x,
            isx.t.line.end.y = r->dir.y,
            isx.t.line.end.z = r->dir.z;
    }

    isx.t.line.end.x = isx.t.line.start.x + isx.t.line.end.x,
        isx.t.line.end.y = isx.t.line.start.y + isx.t.line.end.y,
        isx.t.line.end.z = isx.t.line.start.z + isx.t.line.end.z;

    coll->flags &= ~k_HIT_IT;
    coll->dist = HUGE;

    cbray = *r;
    if (!(r->flags & XRAY3_USE_MAX)) {
        cbray.max_t = 1.0f;
    }
    if (r->flags & XRAY3_USE_MIN) {
        cbray.max_t -= cbray.min_t;
    }

    if (env->geom->jsp) {
        sCollidingJSP = 1;
        xClumpColl_ForAllIntersections(env->geom->jsp->colltree, &isx, rayHitsEnvCB, coll);
        sCollidingJSP = 0;
    } else {
        RpCollisionWorldForAllIntersections(env->geom->world, &isx, rayHitsEnvCB, coll);
        if (env->geom->collision) {
            RpCollisionWorldForAllIntersections(env->geom->collision, &isx, rayHitsEnvCB, coll);
        }
    }

    RwV3d temp = isx.t.line.start;
    isx.t.line.start = isx.t.line.end;
    isx.t.line.end = temp;

    RpCollisionWorldForAllIntersections(env->geom->world, &isx, rayHitsEnvBackwardCB, coll);
    if (env->geom->collision) {
        RpCollisionWorldForAllIntersections(env->geom->collision, &isx, rayHitsEnvBackwardCB, coll);
    }

    if (r->flags & XRAY3_USE_MIN) {
        coll->dist += cbray.min_t;
    }

    return coll->flags & k_HIT_IT;
}

U32 iRayHitsModel(const xRay3* r, const xModelInstance* m, xCollis* coll) NONMATCH("https://decomp.me/scratch/Jk8BU")
{
    if (m->Flags & 0x800) {
        xModelAnimCollApply(*m);
    }
    
    RpIntersection isx;
    isx.type = rpINTERSECTLINE;

    if (r->flags & XRAY3_USE_MIN) {
        xVec3 delta;
        delta.x = r->dir.x * r->min_t,
            delta.y = r->dir.y * r->min_t,
            delta.z = r->dir.z * r->min_t;
        isx.t.line.start.x = r->origin.x + delta.x,
            isx.t.line.start.y = r->origin.y + delta.y,
            isx.t.line.start.z = r->origin.z + delta.z;
    } else {
        isx.t.line.start.x = r->origin.x,
            isx.t.line.start.y = r->origin.y,
            isx.t.line.start.z = r->origin.z;
    }

    if (r->flags & XRAY3_USE_MAX) {
        F32 len;
        if (r->flags & XRAY3_USE_MIN) {
            len = r->max_t - r->min_t;
        } else {
            len = r->max_t;
        }
        isx.t.line.end.x = r->dir.x * len,
            isx.t.line.end.y = r->dir.y * len,
            isx.t.line.end.z = r->dir.z * len;
    } else {
        isx.t.line.end.x = r->dir.x,
            isx.t.line.end.y = r->dir.y,
            isx.t.line.end.z = r->dir.z;
    }

    isx.t.line.end.x = isx.t.line.start.x + isx.t.line.end.x,
        isx.t.line.end.y = isx.t.line.start.y + isx.t.line.end.y,
        isx.t.line.end.z = isx.t.line.start.z + isx.t.line.end.z;

    cbray = *r;

    if (!(r->flags & XRAY3_USE_MAX)) {
        cbray.max_t = 1.0f;
    }

    if (r->flags & XRAY3_USE_MIN) {
        cbray.max_t -= cbray.min_t;
    }
    
    xMat4x3* mat = (xMat4x3*)m->Mat;
    RwFrame* frame = RpAtomicGetFrame(m->Data);
    RwFrameTransform(frame, (RwMatrix*)mat, rwCOMBINEREPLACE);
    
    F32 mscale = xVec3Length(&mat->right);
    cbray.max_t /= mscale;

    coll->flags &= ~k_HIT_IT;
    coll->dist = HUGE;

    if (coll->flags & k_HIT_CALC_TRI) {
        coll->flags |= k_HIT_0x400;
    }

    RpAtomicForAllIntersections(m->Data, &isx, rayHitsModelCB, coll);
    
    RwV3d temp = isx.t.line.start;
    isx.t.line.start = isx.t.line.end;
    isx.t.line.end = temp;

    RpAtomicForAllIntersections(m->Data, &isx, rayHitsModelBackwardCB, coll);

    coll->dist *= mscale;

    if (r->flags & XRAY3_USE_MIN) {
        coll->dist += cbray.min_t;
    }

    if ((coll->flags & k_HIT_CALC_TRI) && (coll->flags & k_HIT_IT)) {
        xVec3 center;
        xMat4x3Tolocal(&center, (xMat4x3*)m->Mat, &r->origin);
        
        xVec3 heading;
        xMat4x3Tolocal(&heading, (xMat4x3*)m->Mat, &coll->tohit);

        xCollideCalcTri(coll->tri, *m, center, heading);
    }

    if (m->Flags & 0x800) {
        xModelAnimCollRestore(*m);
    }

    return coll->flags & k_HIT_IT;
}

void iSphereForModel(xSphere* o, const xModelInstance* m)
{
    RpAtomic* imodel = m->Data;
    RpGeometry* geom = imodel->geometry;
    
    if (!geom->triangles) {
        o->center.x = imodel->boundingSphere.center.x;
        o->center.y = imodel->boundingSphere.center.y;
        o->center.z = imodel->boundingSphere.center.z;
        o->r = imodel->boundingSphere.radius;
        return;
    }

    if (geom->numMorphTargets <= 1) {
        if (geom->numMorphTargets < 1) {
            return;
        }
    }
    
    RpMorphTarget* mtgt = RpGeometryGetMorphTarget(geom, 0);

    iSphereInitBoundVec(o, (xVec3*)&mtgt->verts[0]);
    for (U16 idx = 1; idx < geom->numVertices; idx++) {
        iSphereBoundVec(o, o, (xVec3*)&mtgt->verts[idx]);
    }
}

void iBoxForModel(xBox* o, const xModelInstance* m)
{
    iBoxForModelLocal(o, m);
    xVec3AddTo(&o->upper, (xVec3*)&m->Mat->pos);
    xVec3AddTo(&o->lower, (xVec3*)&m->Mat->pos);
}

void iBoxForModelLocal(xBox* o, const xModelInstance* m)
{
    RpAtomic* imodel = m->Data;
    RpGeometry* geom = imodel->geometry;
    
    if (!geom->triangles) {
        o->lower.x = imodel->boundingSphere.center.x - imodel->boundingSphere.radius;
        o->lower.y = imodel->boundingSphere.center.y - imodel->boundingSphere.radius;
        o->lower.z = imodel->boundingSphere.center.z - imodel->boundingSphere.radius;
        o->upper.x = imodel->boundingSphere.center.x + imodel->boundingSphere.radius;
        o->upper.y = imodel->boundingSphere.center.y + imodel->boundingSphere.radius;
        o->upper.z = imodel->boundingSphere.center.z + imodel->boundingSphere.radius;
        return;
    }

    if (geom->numMorphTargets <= 1) {
        if (geom->numMorphTargets < 1) {
            return;
        }
    }
    
    RpMorphTarget* mtgt = RpGeometryGetMorphTarget(geom, 0);

    iBoxInitBoundVec(o, (xVec3*)&mtgt->verts[0]);
    for (U16 idx = 1; idx < geom->numVertices; idx++) {
        iBoxBoundVec(o, o, (xVec3*)&mtgt->verts[idx]);
    }
}
#include "xBound.h"

#include "xDebug.h"
#include "xDraw.h"

#define MAKE_BOUND_MASK(a,b) ((a)|((b)<<3))

void xBoundUpdate(xBound* b)
{
    if (b->type == k_XBOUNDTYPE_BOX)
    {
        xVec3Center(&b->box.center, &b->box.box.lower, &b->box.box.upper);
    }
    else if (b->type == k_XBOUNDTYPE_OBB)
    {
        xVec3Center(&b->box.center, &b->box.box.lower, &b->box.box.upper);
        xMat4x3Toworld(&b->box.center, b->mat, &b->box.center);
    }
    if (b->type != k_XBOUNDTYPE_NONE)
    {
        xQuickCullForBound(&b->qcd, b);
    }
}

void xBoundGetBox(xBox& box, const xBound& bound)
{
    switch (bound.type)
    {
    case k_XBOUNDTYPE_SPHERE:
    {
        const xSphere& o = bound.sph;
        box.upper.assign(o.center.x + o.r, o.center.y + o.r, o.center.z + o.r);
        box.lower.assign(o.center.x - o.r, o.center.y - o.r, o.center.z - o.r);
        break;
    }
    case k_XBOUNDTYPE_BOX: box = bound.box.box;
        break;
    case k_XBOUNDTYPE_OBB:
        xBoxInitBoundOBB(&box, &bound.box.box, bound.mat);
        break;
    default: xASSERTFAILMSG("Unsupported bound type.");
    }
}

void xBoundGetSphere(xSphere& o, const xBound& bound)
{
    switch (bound.type)
    {
    case k_XBOUNDTYPE_SPHERE:
        o = bound.sph;
        break;
    case k_XBOUNDTYPE_BOX:
        o.center = bound.box.center;
        o.r = (bound.box.box.upper - bound.box.center).length();
        break;
    case k_XBOUNDTYPE_OBB:
    {
        const xMat4x3& mat = *bound.mat;
        xVec3 v = (bound.box.box.upper - bound.box.box.lower) * 0.5f;
        F32 r2 = xsqr(v.x) * mat.right.length2() + xsqr(v.y) * mat.up.length2() + xsqr(v.z) * mat.at.length2();
        o.r = xsqrt(r2);
        o.center = bound.box.center;
        break;
    }
    default:
        xASSERTFAILMSG("Unsupported bound type.");
    }
}

void xBoundSphereHitsOBB(const xSphere* s, const xBox* b, const xMat4x3* m, xCollis* coll)
{
    xSphereHitsOBB_nu(s, b, m, coll);
}

void xBoundHitsBound(const xBound* a, const xBound* b, xCollis* c)
{
    xASSERT(a);
    xASSERT(b);
    xASSERT(c);

    if (!xQuickCullIsects(&a->qcd, &b->qcd))
    {
        c->flags &= ~k_HIT_IT;
        return;
    }

    switch (MAKE_BOUND_MASK(a->type, b->type))
    {
    case MAKE_BOUND_MASK(k_XBOUNDTYPE_SPHERE, k_XBOUNDTYPE_SPHERE):
        xSphereHitsSphere(&a->sph, &b->sph, c);
        break;
    case MAKE_BOUND_MASK(k_XBOUNDTYPE_SPHERE, k_XBOUNDTYPE_OBB):
        xBoundSphereHitsOBB(&a->sph, &b->box.box, b->mat, c);
        break;
    case MAKE_BOUND_MASK(k_XBOUNDTYPE_SPHERE, k_XBOUNDTYPE_BOX):
        xSphereHitsBox(&a->sph, &b->box.box, c);
        break;
    case MAKE_BOUND_MASK(k_XBOUNDTYPE_BOX, k_XBOUNDTYPE_SPHERE):
        xBoxHitsSphere(&a->box.box, &b->sph, c);
        break;
    case MAKE_BOUND_MASK(k_XBOUNDTYPE_BOX, k_XBOUNDTYPE_OBB):
        xBoxHitsObb(&a->box.box, &b->box.box, b->mat, c);
        break;
    case MAKE_BOUND_MASK(k_XBOUNDTYPE_OBB, k_XBOUNDTYPE_SPHERE):
    case MAKE_BOUND_MASK(k_XBOUNDTYPE_OBB, k_XBOUNDTYPE_OBB):
    case MAKE_BOUND_MASK(k_XBOUNDTYPE_OBB, k_XBOUNDTYPE_BOX):
    case MAKE_BOUND_MASK(k_XBOUNDTYPE_SPHERE, k_XBOUNDTYPE_CYL):
    case MAKE_BOUND_MASK(k_XBOUNDTYPE_CYL, k_XBOUNDTYPE_SPHERE):
    case MAKE_BOUND_MASK(k_XBOUNDTYPE_BOX, k_XBOUNDTYPE_BOX): // bug?
    case MAKE_BOUND_MASK(k_XBOUNDTYPE_BOX, k_XBOUNDTYPE_CYL):
    case MAKE_BOUND_MASK(k_XBOUNDTYPE_OBB, k_XBOUNDTYPE_CYL):
    default:
        DBprintf(DBML_DBG, "xBoundHitsBound: unsupported collision type - bound %d vs [ANY]\n", a->type);
    }
}

static void xBoundOBBIsectRay(const xBox* b, const xMat4x3* m, const xRay3* r, xIsect* isect) NONMATCH("https://decomp.me/scratch/cdowj")
{
    xASSERT(b);
    xASSERT(m);
    xASSERT(r);
    xASSERT(isect);
    
    xRay3 xfr;
    xBox sbox;
    xVec3 scale;
    xMat4x3 mnormal;

    xVec3NormalizeMacro(&mnormal.right, &m->right, &scale.x);
    xVec3NormalizeMacro(&mnormal.up, &m->up, &scale.y);
    xVec3NormalizeMacro(&mnormal.at, &m->at, &scale.z);
    mnormal.pos = m->pos;
    sbox.upper.x = b->upper.x * scale.x, sbox.upper.y = b->upper.y * scale.y, sbox.upper.z = b->upper.z * scale.z;
    sbox.lower.x = b->lower.x * scale.x, sbox.lower.y = b->lower.y * scale.y, sbox.lower.z = b->lower.z * scale.z;
    xMat4x3Tolocal(&xfr.origin, &mnormal, &r->origin);
    xMat3x3Tolocal(&xfr.dir, &mnormal, &r->dir);

    xfr.min_t = r->min_t;
    xfr.max_t = r->max_t;
    xfr.flags = r->flags;
    iBoxIsectRay(&sbox, &xfr, isect);
}

void xRayHitsBound(const xRay3* r, const xBound* b, xCollis* c)
{
    xASSERT(r);
    xASSERT(b);
    xASSERT(c);

    xIsect isect;

    if (b->type == k_XBOUNDTYPE_SPHERE)
    {
        iSphereIsectRay(&b->sph, r, &isect);
    }
    else if (b->type == k_XBOUNDTYPE_OBB)
    {
        xBoundOBBIsectRay(&b->box.box, b->mat, r, &isect);
    }
    else if (b->type == k_XBOUNDTYPE_BOX)
    {
        iBoxIsectRay(&b->box.box, r, &isect);
    }
    else
    {
        DBprintf(DBML_DBG, "xRayHitsBound: unsupported bound type %d\n", b->type);
    }

    if (isect.penned <= 0.0f)
    {
        c->flags |= k_HIT_IT;
        c->dist = isect.dist;
        return;
    }

    c->flags &= ~k_HIT_IT;
}

void xSphereHitsBound(const xSphere* o, const xBound* b, xCollis* c)
{
    xASSERT(o != 0); xASSERT(b != 0); xASSERT(c != 0);
    switch (b->type)
    {
    case k_XBOUNDTYPE_SPHERE:
        xSphereHitsSphere(o, &b->sph, c);
        break;
    case k_XBOUNDTYPE_OBB:
        xBoundSphereHitsOBB(o, &b->box.box, b->mat, c);
        break;
    case k_XBOUNDTYPE_BOX:
        xSphereHitsBox(o, &b->box.box, c);
        break;
    default:
        DBprintf(DBML_DBG, "xBoundHitsBound: unsupported collision type - sphere bound vs %d\n", b->type);
    }
}

void xVecHitsBound(const xVec3* v, const xBound* b, xCollis* c)
{
    xASSERT(v);
    xASSERT(b);
    xASSERT(c);
    
    xIsect isect;

    if (b->type == k_XBOUNDTYPE_SPHERE)
    {
        iSphereIsectVec(&b->sph, v, &isect);
    }
    else if (b->type == k_XBOUNDTYPE_OBB)
    {
        xVec3 lv; xMat4x3Tolocal(&lv, b->mat, v);
        iBoxIsectVec(&b->box.box, &lv, &isect);
    }
    else if (b->type == k_XBOUNDTYPE_BOX)
    {
        iBoxIsectVec(&b->box.box, v, &isect);
    }
    else
    {
        DBprintf(DBML_DBG, "xVecHitsBound: unsupported bound type %d\n", b->type);
    }

    if (isect.penned <= 0.0f)
    {
        c->flags |= k_HIT_IT;
    }
    else
    {
        c->flags &= ~k_HIT_IT;
    }
}

void xBoundDraw(const xBound* b)
{
    if (b->type == k_XBOUNDTYPE_SPHERE)
    {
        xDrawSphere2(&b->sph, 12);
    }
    else if (b->type == k_XBOUNDTYPE_OBB)
    {
        xDrawOBB(&b->box.box, b->mat);
    }
    else if (b->type == k_XBOUNDTYPE_BOX)
    {
        xDrawBox(&b->box.box);
    }
    else
    {
        DBprintf(DBML_DBG, "xBoundDraw: unsupported bound type %d\n", b->type);
    }
}
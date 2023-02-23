#include "iMath3.h"

#include "xMath3.h"
#include "xMath2.h"

#include <string.h>

void iMath3Init()
{
}

void iSphereIsectVec(const xSphere* s, const xVec3* v, xIsect* isx)
{
    xVec3Sub(&isx->norm, v, &s->center);
    isx->dist = xVec3Length(&isx->norm);
    isx->penned = isx->dist - s->r;
}

void iSphereIsectRay(const xSphere* s, const xRay3* r, xIsect* isx) NONMATCH("https://decomp.me/scratch/5KhTZ")
{
    if (!(r->flags & XRAY3_USE_MIN)) {
        ((xRay3*)r)->min_t = 0.0f;
    }

    if (!(r->flags & XRAY3_USE_MAX)) {
        ((xRay3*)r)->max_t = 1.0f;
    }

    xVec3Sub(&isx->norm, &r->origin, &s->center);

    F32 t_in;
    F32 t_out;
    U32 num = xMathSolveQuadratic(xVec3Dot(&r->dir, &r->dir),
                                  2.0f * xVec3Dot(&isx->norm, &r->dir),
                                  xVec3Dot(&isx->norm, &isx->norm) - xsqr(s->r),
                                  &t_in, &t_out);

    if (num == 0) {
        isx->penned = 1.0f;
        isx->contained = 1.0f;
    } else if (num == 1) {
        if (t_in < r->min_t || t_in > r->max_t) {
            isx->dist = t_in;
            isx->penned = 1.0f;
            isx->contained = 1.0f;
        } else {
            isx->dist = t_in;
            isx->penned = -1.0f;
            isx->contained = 1.0f;
        }
    } else if (t_in < r->min_t) {
        if (t_out < r->min_t) {
            isx->dist = t_out;
            isx->penned = 1.0f;
            isx->contained = 1.0f;
        } else {
            isx->dist = t_out;
            isx->penned = -1.0f;
            isx->contained = -1.0f;
        }
    } else if (t_in <= r->max_t) {
        isx->dist = t_in;
        isx->penned = -1.0f;
        isx->contained = 1.0f;
    } else {
        isx->dist = t_in;
        isx->penned = 1.0f;
        isx->contained = 1.0f;
    }
}

void iSphereIsectSphere(const xSphere* s, const xSphere* p, xIsect* isx)
{
    xVec3Sub(&isx->norm, &p->center, &s->center);
    isx->dist = xVec3Length(&isx->norm);
    isx->penned = isx->dist - p->r - s->r;
    isx->contained = isx->dist - s->r;
}

void iSphereInitBoundVec(xSphere* s, const xVec3* v)
{
    xVec3Copy(&s->center, v);
    s->r = EPSILON;
}

void iSphereBoundVec(xSphere* o, const xSphere* s, const xVec3* v)
{
    xIsect isx;
    F32 scale;
    xSphere temp;
    xSphere* tp;
    U32 usetemp = (o == s);

    iSphereIsectVec(s, v, &isx);

    if (isx.penned <= 0.0f) {
        if (!usetemp) {
            memcpy(o, s, sizeof(xSphere));
        }
    } else {
        if (usetemp) {
            tp = &temp;
        } else {
            tp = o;
        }
        xVec3Copy(&tp->center, &isx.norm);
        scale = (isx.dist - s->r) / (2.0f * isx.dist);
        xVec3SMul(&tp->center, &tp->center, scale);
        xVec3Add(&tp->center, &tp->center, &s->center);
        tp->r = 0.5f * (isx.dist + s->r);
        if (usetemp) {
            memcpy(o, tp, sizeof(xSphere));
        }
    }
}

void iCylinderIsectVec(const xCylinder* c, const xVec3* v, xIsect* isx)
{
    F32 a = c->center.y - c->h;
    F32 b = c->center.y + c->h;
    if (v->y >= a && v->y <= b &&
        xVec2Dist(c->center.x, c->center.z, v->x, v->z) <= c->r) {
        isx->penned = -1.0f;
    } else {
        isx->penned = 1.0f;
    }
}

void iBoxVecDist(const xBox* box, const xVec3* v, xIsect* isx) NONMATCH("https://decomp.me/scratch/PkLg1")
{
    if (v->x < box->lower.x) {
        if (v->y < box->lower.y) {
            if (v->z < box->lower.z) {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = box->lower.z - v->z;
                isx->flags &= ~XISECT_0x1F000000;
                isx->flags |= XISECT_0x80000000;
            } else if (v->z <= box->upper.z) {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = 0.0f;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x1000000;
                isx->flags |= XISECT_0x40000000;
            } else {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x2000000;
                isx->flags |= XISECT_0x80000000;
            }
        } else if (v->y <= box->upper.y) {
            if (v->z < box->lower.z) {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = 0.0f;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x1000000 | XISECT_0x2000000;
                isx->flags |= XISECT_0x40000000;
            } else if (v->z <= box->upper.z) {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = 0.0f;
                isx->norm.z = 0.0f;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x4000000;
                isx->flags |= XISECT_0x20000000;
            } else {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = 0.0f;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x1000000 | XISECT_0x4000000;
                isx->flags |= XISECT_0x40000000;
            }
        } else {
            if (v->z < box->lower.z) {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x2000000 | XISECT_0x4000000;
                isx->flags |= XISECT_0x80000000;
            } else if (v->z <= box->upper.z) {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = 0.0f;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x1000000 | XISECT_0x2000000 | XISECT_0x4000000;
                isx->flags |= XISECT_0x40000000;
            } else {
                isx->norm.x = box->lower.x - v->x;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x8000000;
                isx->flags |= XISECT_0x80000000;
            }
        }
    } else if (v->x <= box->upper.x) {
        if (v->y < box->lower.y) {
            if (v->z < box->lower.z) {
                isx->norm.x = 0.0f;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x1000000 | XISECT_0x8000000;
                isx->flags |= XISECT_0x40000000;
            } else if (v->z <= box->upper.z) {
                isx->norm.x = 0.0f;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = 0.0f;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x2000000 | XISECT_0x8000000;
                isx->flags |= XISECT_0x20000000;
            } else {
                isx->norm.x = 0.0f;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x1000000 | XISECT_0x2000000 | XISECT_0x8000000;
                isx->flags |= XISECT_0x40000000;
            }
        } else if (v->y <= box->upper.y) {
            if (v->z < box->lower.z) {
                isx->norm.x = 0.0f;
                isx->norm.y = 0.0f;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x4000000 | XISECT_0x8000000;
                isx->flags |= XISECT_0x20000000;
            } else if (v->z <= box->upper.z) {
            } else {
                isx->norm.x = 0.0f;
                isx->norm.y = 0.0f;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x2000000 | XISECT_0x4000000 | XISECT_0x8000000;
                isx->flags |= XISECT_0x20000000;
            }
        } else {
            if (v->z < box->lower.z) {
                isx->norm.x = 0.0f;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x1000000 | XISECT_0x2000000 | XISECT_0x4000000 | XISECT_0x8000000;
                isx->flags |= XISECT_0x40000000;
            } else if (v->z <= box->upper.z) {
                isx->norm.x = 0.0f;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = 0.0f;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x10000000;
                isx->flags |= XISECT_0x20000000;
            } else {
                isx->norm.x = 0.0f;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x1000000 | XISECT_0x10000000;
                isx->flags |= XISECT_0x40000000;
            }
        }
    } else {
        if (v->y < box->lower.y) {
            if (v->z < box->lower.z) {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x2000000 | XISECT_0x10000000;
                isx->flags |= XISECT_0x80000000;
            } else if (v->z <= box->upper.z) {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = 0.0f;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x1000000 | XISECT_0x2000000 | XISECT_0x10000000;
                isx->flags |= XISECT_0x40000000;
            } else {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = box->lower.y - v->y;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x4000000 | XISECT_0x10000000;
                isx->flags |= XISECT_0x80000000;
            }
        } else if (v->y <= box->upper.y) {
            if (v->z < box->lower.z) {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = 0.0f;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x1000000 | XISECT_0x4000000 | XISECT_0x10000000;
                isx->flags |= XISECT_0x40000000;
            } else if (v->z <= box->upper.z) {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = 0.0f;
                isx->norm.z = 0.0f;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x2000000 | XISECT_0x4000000 | XISECT_0x10000000;
                isx->flags |= XISECT_0x20000000;
            } else {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = 0.0f;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x1000000 | XISECT_0x2000000 | XISECT_0x4000000 | XISECT_0x10000000;
                isx->flags |= XISECT_0x40000000;
            }
        } else {
            if (v->z < box->lower.z) {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = box->lower.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x8000000 | XISECT_0x10000000;
                isx->flags |= XISECT_0x80000000;
            } else if (v->z <= box->upper.z) {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = 0.0f;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x1000000 | XISECT_0x8000000 | XISECT_0x10000000;
                isx->flags |= XISECT_0x40000000;
            } else {
                isx->norm.x = box->upper.x - v->x;
                isx->norm.y = box->upper.y - v->y;
                isx->norm.z = box->upper.z - v->z;
                isx->flags = (isx->flags & ~XISECT_0x1F000000) | XISECT_0x2000000 | XISECT_0x8000000 | XISECT_0x10000000;
                isx->flags |= XISECT_0x80000000;
            }
        }
    }

    isx->dist = xVec3Length(&isx->norm);
}

void iBoxIsectVec(const xBox* b, const xVec3* v, xIsect* isx)
{
    if (v->x >= b->lower.x && v->x <= b->upper.x &&
        v->y >= b->lower.y && v->y <= b->upper.y &&
        v->z >= b->lower.z && v->z <= b->upper.z) {
        isx->penned = -1.0f;
    } else {
        isx->penned = 1.0f;
    }
}

static U32 ClipPlane(F32 denom, F32 numer, F32* t_in, F32* t_out)
{
    if (denom > 0.0f) {
        if (numer > denom * *t_out) {
            return 0;
        }

        if (numer > denom * *t_in) {
            *t_in = numer / denom;
        }

        return 1;
    }

    if (denom < 0.0f) {
        if (numer > denom * *t_in) {
            return 0;
        }

        if (numer > denom * *t_out) {
            *t_out = numer / denom;
        }

        return 1;
    }

    if (numer <= 0.0f) {
        return 1;
    }

    return 0;
}

static U32 ClipBox(const xVec3* E, const xVec3* P, const xVec3* D, F32* t_in, F32* t_out)
{
    return ClipPlane(D->x, -P->x - E->x, t_in, t_out) &&
           ClipPlane(-D->x, P->x - E->x, t_in, t_out) &&
           ClipPlane(D->y, -P->y - E->y, t_in, t_out) &&
           ClipPlane(-D->y, P->y - E->y, t_in, t_out) &&
           ClipPlane(D->z, -P->z - E->z, t_in, t_out) &&
           ClipPlane(-D->z, P->z - E->z, t_in, t_out);
}

void iBoxIsectRay(const xBox* b, const xRay3* r, xIsect* isx) NONMATCH("https://decomp.me/scratch/zalOG")
{
    if (!(r->flags & XRAY3_USE_MIN)) {
        ((xRay3*)r)->min_t = 0.0f;
    }

    if (!(r->flags & XRAY3_USE_MAX)) {
        ((xRay3*)r)->max_t = 1.0f;
    }

    F32 t_in = -HUGE;
    F32 t_out = HUGE;

    xVec3 E;
    E.x = b->upper.x - b->lower.x, E.y = b->upper.y - b->lower.y, E.z = b->upper.z - b->lower.z;
    E.x *= 0.5f, E.y *= 0.5f, E.z *= 0.5f;

    xVec3 P;
    P.x = b->lower.x + b->upper.x, P.y = b->lower.y + b->upper.y, P.z = b->lower.z + b->upper.z;
    P.x *= -0.5f, P.y *= -0.5f, P.z *= -0.5f;
    P.x += r->origin.x, P.y += r->origin.y, P.z += r->origin.z;

    if (ClipBox(&E, &P, &r->dir, &t_in, &t_out)) {
        if (t_in < r->min_t) {
            if (t_out < r->min_t) {
                isx->dist = t_out;
                isx->penned = 1.0f;
                isx->contained = 1.0f;
            } else {
                isx->dist = t_out;
                isx->penned = -1.0f;
                isx->contained = -1.0f;
            }
        } else {
            if (t_in <= r->max_t) {
                isx->dist = t_in;
                isx->penned = -1.0f;
                isx->contained = 1.0f;
            } else {
                isx->dist = t_in;
                isx->penned = 1.0f;
                isx->contained = 1.0f;
            }
        }
    } else {
        isx->penned = 1.0f;
        isx->contained = 1.0f;
    }
}

void iBoxIsectSphere(const xBox* box, const xSphere* p, xIsect* isx) NONMATCH("https://decomp.me/scratch/J6qYG")
{
    U32 xcode;
    U32 ycode;
    U32 zcode;
    U32 xcm;
    U32 ycm;
    U32 zcm;

    xcode = (p->center.x - p->r < box->lower.x) ?
                ((p->center.x + p->r < box->lower.x) ? 2 :
                    ((p->center.x + p->r > box->upper.x) ? 0 : 1)) :
                ((p->center.x - p->r > box->upper.x) ? 5 :
                    ((p->center.x + p->r > box->upper.x) ? 4 : 3));
    xcm = xcode % 3;
    if (xcm == 2) {
        isx->penned = 1.0f;
        return;
    }

    ycode = (p->center.y - p->r < box->lower.y) ?
                ((p->center.y + p->r < box->lower.y) ? 2 :
                    ((p->center.y + p->r > box->upper.y) ? 0 : 1)) :
                ((p->center.y - p->r > box->upper.y) ? 5 :
                    ((p->center.y + p->r > box->upper.y) ? 4 : 3));
    ycm = ycode % 3;
    if (ycm == 2) {
        isx->penned = 1.0f;
        return;
    }

    zcode = (p->center.z - p->r < box->lower.z) ?
                ((p->center.z + p->r < box->lower.z) ? 2 :
                    ((p->center.z + p->r > box->upper.z) ? 0 : 1)) :
                ((p->center.z - p->r > box->upper.z) ? 5 :
                    ((p->center.z + p->r > box->upper.z) ? 4 : 3));
    zcm = zcode % 3;
    if (zcm == 2) {
        isx->penned = 1.0f;
        return;
    }

    iBoxIsectVec(box, &p->center, isx);
    if (isx->penned < 0.0f) {
        xVec3 bc;
        xVec3Add(&bc, &box->lower, &box->upper);
        xVec3SMulBy(&bc, 0.5f);
        xVec3Sub(&isx->norm, &p->center, &bc);
        isx->dist = xVec3Length(&isx->norm);
        isx->contained = -1.0f;
    } else {
        isx->flags = xcode | (ycode << 4) | (zcode << 8);
        iBoxVecDist(box, &p->center, isx);
        isx->penned = isx->dist - p->r;
        isx->contained = 1.0f;
    }
}

void iBoxInitBoundVec(xBox* b, const xVec3* v)
{
    xVec3Copy(&b->lower, v);
    xVec3Copy(&b->upper, v);
}

void iBoxBoundVec(xBox* o, const xBox* b, const xVec3* v)
{
    xVec3Init(&o->lower,
              xmin(v->x, b->lower.x),
              xmin(v->y, b->lower.y),
              xmin(v->z, b->lower.z));
    xVec3Init(&o->upper,
              xmax(v->x, b->upper.x),
              xmax(v->y, b->upper.y),
              xmax(v->z, b->upper.z));
}
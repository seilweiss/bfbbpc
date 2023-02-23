#include "xMath3.h"

xVec3 g_O3 = { 0.0f, 0.0f, 0.0f };
xVec3 g_X3 = { 1.0f, 0.0f, 0.0f };
xVec3 g_Y3 = { 0.0f, 1.0f, 0.0f };
xVec3 g_Z3 = { 0.0f, 0.0f, 1.0f };
xVec3 g_NX3 = { -1.0f, 0.0f, 0.0f };
xVec3 g_NY3 = { 0.0f, -1.0f, 0.0f };
xVec3 g_NZ3 = { 0.0f, 0.0f, -1.0f };
xVec3 g_Onez = { 1.0f, 1.0f, 1.0f };
xMat4x3 g_I3;
xQuat g_IQ = { 0.0f, 0.0f, 0.0f, 1.0f };

void xMath3Init() NONMATCH("https://decomp.me/scratch/zknoT")
{
    iMath3Init();

    g_I3.right.x = g_X3.x;
    g_I3.right.y = g_X3.y;
    g_I3.right.z = g_X3.z;
    g_I3.up.x = g_Y3.x;
    g_I3.up.y = g_Y3.y;
    g_I3.up.z = g_Y3.z;
    g_I3.at.x = g_Z3.x;
    g_I3.at.y = g_Z3.y;
    g_I3.at.z = g_Z3.z;
    g_I3.pos.x = g_O3.x;
    g_I3.pos.y = g_O3.y;
    g_I3.pos.z = g_O3.z;
}

void xLine3VecDist2(const xVec3* p1, const xVec3* p2, const xVec3* v, xIsect* isx)
{
    xVec3 ldir;
    xVec3Sub(&ldir, p2, p1);
    xVec3Sub(&isx->norm, v, p1);

    F32 ldirdotlv = xVec3Dot(&ldir, &isx->norm);
    if (ldirdotlv <= 0.0f) {
        isx->dist = xVec3Length2(&isx->norm);
        return;
    }

    F32 ldirlen2 = xVec3Length2(&ldir);
    if (ldirdotlv >= ldirlen2) {
        xVec3Sub(&isx->norm, v, p2);
        isx->dist = xVec3Length2(&isx->norm);
        return;
    }

    F32 lvlen2 = xVec3Length2(&isx->norm);
    isx->dist = lvlen2 - xsqr(ldirdotlv) / ldirlen2;
}

S32 xPointInBox(const xBox* b, const xVec3* p)
{
    return p->x >= b->lower.x &&
        p->x <= b->upper.x &&
        p->y >= b->lower.y &&
        p->y <= b->upper.y &&
        p->z >= b->lower.z &&
        p->z <= b->upper.z;
}

void xBoxInitBoundOBB(xBox* o, const xBox* b, const xMat4x3* m)
{
    xVec3 boxcent;
    boxcent.x = 0.5f * (b->lower.x + b->upper.x);
    boxcent.y = 0.5f * (b->lower.y + b->upper.y);
    boxcent.z = 0.5f * (b->lower.z + b->upper.z);

    F32 xmax = xabs(m->right.x * (b->upper.x - boxcent.x));
    F32 ymax = xabs(m->right.y * (b->upper.x - boxcent.x));
    F32 zmax = xabs(m->right.z * (b->upper.x - boxcent.x));

    xmax += xabs(m->up.x * (b->upper.y - boxcent.y));
    ymax += xabs(m->up.y * (b->upper.y - boxcent.y));
    zmax += xabs(m->up.z * (b->upper.y - boxcent.y));

    xmax += xabs(m->at.x * (b->upper.z - boxcent.z));
    ymax += xabs(m->at.y * (b->upper.z - boxcent.z));
    zmax += xabs(m->at.z * (b->upper.z - boxcent.z));

    xMat4x3Toworld(&boxcent, m, &boxcent);

    o->lower.x = boxcent.x - xmax;
    o->lower.y = boxcent.y - ymax;
    o->lower.z = boxcent.z - zmax;
    o->upper.x = boxcent.x + xmax;
    o->upper.y = boxcent.y + ymax;
    o->upper.z = boxcent.z + zmax;
}

void xBoxInitBoundCapsule(xBox* b, const xCapsule* c)
{
    if (c->start.x < c->end.x) {
        b->lower.x = c->start.x - c->r;
        b->upper.x = c->end.x + c->r;
    } else {
        b->lower.x = c->end.x - c->r;
        b->upper.x = c->start.x + c->r;
    }

    if (c->start.y < c->end.y) {
        b->lower.y = c->start.y - c->r;
        b->upper.y = c->end.y + c->r;
    } else {
        b->lower.y = c->end.y - c->r;
        b->upper.y = c->start.y + c->r;
    }

    if (c->start.z < c->end.z) {
        b->lower.z = c->start.z - c->r;
        b->upper.z = c->end.z + c->r;
    } else {
        b->lower.z = c->end.z - c->r;
        b->upper.z = c->start.z + c->r;
    }
}

void xBoxFromCone(xBox& box, const xVec3& center, const xVec3& dir, F32 dist, F32 r1, F32 r2)
{
    xBoxFromCircle(box, center, dir, r1);

    xBox temp;
    xBoxFromCircle(temp, center + dir * dist, dir, r2);

    xBoxUnion(box, box, temp);
}

void xMat3x3Normalize(xMat3x3* o, const xMat3x3* m)
{
    xVec3Normalize(&o->right, &m->right);
    xVec3Normalize(&o->up, &m->up);
    xVec3Normalize(&o->at, &m->at);
}

void xMat3x3GetEuler(const xMat3x3* m, xVec3* a)
{
    F32 pitch, yaw, roll;

    pitch = -xasin(m->at.y);
    if (pitch < PI/2) {
        if (pitch > -PI/2) {
            yaw = xatan2(m->at.x, m->at.z);
            roll = xatan2(m->right.y, m->up.y);
        } else {
            yaw = -xatan2(-m->up.x, m->right.x);
            roll = 0.0f;
        }
    } else {
        yaw = xatan2(-m->up.x, m->right.x);
        roll = 0.0f;
    }

    a->x = yaw;
    a->y = pitch;
    a->z = roll;
}

void xMat4x3MoveLocalRight(xMat4x3* m, F32 mag)
{
    m->pos.x += m->right.x * mag;
    m->pos.y += m->right.y * mag;
    m->pos.z += m->right.z * mag;
}

void xMat4x3MoveLocalUp(xMat4x3* m, F32 mag)
{
    m->pos.x += m->up.x * mag;
    m->pos.y += m->up.y * mag;
    m->pos.z += m->up.z * mag;
}

void xMat4x3MoveLocalAt(xMat4x3* m, F32 mag)
{
    m->pos.x += m->at.x * mag;
    m->pos.y += m->at.y * mag;
    m->pos.z += m->at.z * mag;
}

F32 xMat3x3LookVec(xMat3x3* m, const xVec3* at) NONMATCH("https://decomp.me/scratch/pa9WN")
{
    F32 vec_len = xVec3Normalize(&m->at, at);
    xVec3Inv(&m->at, &m->at);

    if (xabs(1.0f - m->at.y) < EPSILON) {
        m->right.x = 1.0f;
        m->right.y = 0.0f;
        m->right.z = 0.0f;
        m->up.x = 0.0f;
        m->up.y = 0.0f;
        m->up.z = 1.0f;
        m->at.x = 0.0f;
        m->at.y = -1.0f;
        m->at.z = 0.0f;
        return vec_len;
    }

    if (xabs(1.0f + m->at.y) < EPSILON) {
        m->right.x = -1.0f;
        m->right.y = 0.0f;
        m->right.z = 0.0f;
        m->up.x = 0.0f;
        m->up.y = 0.0f;
        m->up.z = -1.0f;
        m->at.x = 0.0f;
        m->at.y = 1.0f;
        m->at.z = 0.0f;
        return vec_len;
    }

    if (xabs(at->z) < EPSILON && xabs(at->x) < EPSILON) {
        m->right.x = 1.0f;
        m->right.y = 0.0f;
        m->right.z = 0.0f;
        m->up.x = 0.0f;
        m->up.y = 1.0f;
        m->up.z = 0.0f;
        m->at.x = 0.0f;
        m->at.y = 0.0f;
        m->at.z = 1.0f;
        return 0.0f;
    }

    m->right.x = m->at.z;
    m->right.y = 0.0f;
    m->right.z = -m->at.x;

    xVec3Normalize(&m->right, &m->right);
    xVec3Cross(&m->up, &m->at, &m->right);
    xVec3Cross(&m->right, &m->up, &m->at);
    m->flags = 0;

    return vec_len;
}

void xMat3x3Euler(xMat3x3* m, const xVec3* ypr)
{
    xMat3x3Euler(m, ypr->x, ypr->y, ypr->z);
}

void xMat3x3Euler(xMat3x3* m, F32 yaw, F32 pitch, F32 roll)
{
    F32 sy = isin(yaw);
    F32 cy = icos(yaw);
    F32 sp = isin(pitch);
    F32 cp = icos(pitch);
    F32 sr = isin(roll);
    F32 cr = icos(roll);
    m->right.x = cy * cr + sy * sp * sr;
    m->right.y = cp * sr;
    m->right.z = -sy * cr + cy * sp * sr;
    m->up.x = -cy * sr + sy * sp * cr;
    m->up.y = cp * cr;
    m->up.z = sy * sr + cy * sp * cr;
    m->at.x = sy * cp;
    m->at.y = -sp;
    m->at.z = cy * cp;
    m->flags = 0;
}

void xMat3x3RotC(xMat3x3* m, F32 _x, F32 _y, F32 _z, F32 t)
{
    if (t == 0.0f) {
        xMat3x3Identity(m);
        return;
    }

    F32 a = icos(t);
    F32 b = isin(t);
    F32 c = 1.0f - a;

    m->right.x = c * _x * _x + a;
    m->right.y = b * _z + c * _x * _y;
    m->right.z = -b * _y + c * _z * _x;
    m->up.x = -b * _z + c * _x * _y;
    m->up.y = c * _y * _y + a;
    m->up.z = b * _x + c * _y * _z;
    m->at.x = b * _y + c * _z * _x;
    m->at.y = -b * _x + c * _y * _z;
    m->at.z = c * _z * _z + a;
    m->flags = 0;
}

void xMat3x3RotX(xMat3x3* m, F32 t)
{
    F32 a = icos(t);
    F32 b = isin(t);
    xVec3Copy(&m->right, &g_X3);
    xVec3Init(&m->up, 0.0f, a, b);
    xVec3Init(&m->at, 0.0f, -b, a);
    m->flags = 0;
}

void xMat3x3RotY(xMat3x3* m, F32 t)
{
    F32 a = icos(t);
    F32 b = isin(t);
    xVec3Init(&m->right, a, 0.0f, -b);
    xVec3Copy(&m->up, &g_Y3);
    xVec3Init(&m->at, b, 0.0f, a);
    m->flags = 0;
}

void xMat3x3RotZ(xMat3x3* m, F32 t)
{
    F32 a = icos(t);
    F32 b = isin(t);
    xVec3Init(&m->right, a, b, 0.0f);
    xVec3Init(&m->up, -b, a, 0.0f);
    xVec3Copy(&m->at, &g_Z3);
    m->flags = 0;
}

void xMat3x3ScaleC(xMat3x3* m, F32 x, F32 y, F32 z)
{
    xVec3Init(&m->right, x, 0.0f, 0.0f);
    xVec3Init(&m->up, 0.0f, y, 0.0f);
    xVec3Init(&m->at, 0.0f, 0.0f, z);
    m->flags = 0;
}

void xMat3x3RMulRotY(xMat3x3* o, const xMat3x3* m, F32 t)
{
    F32 a = icos(t);
    F32 b = isin(t);
    if (o == m) {
        F32 temp;
        temp = a * o->right.x + b * o->right.z;
        o->right.z = a * o->right.z - b * o->right.x;
        o->right.x = temp;
        temp = a * o->up.x + b * o->up.z;
        o->up.z = a * o->up.z - b * o->up.x;
        o->up.x = temp;
        temp = a * o->at.x + b * o->at.z;
        o->at.z = a * o->at.z - b * o->at.x;
        o->at.x = temp;
    } else {
        o->right.x = a * m->right.x + b * m->right.z;
        o->right.y = m->right.y;
        o->right.z = a * m->right.z - b * m->right.x;
        o->up.x = a * m->up.x + b * m->up.z;
        o->up.y = m->up.y;
        o->up.z = a * m->up.z - b * m->up.x;
        o->at.x = a * m->at.x + b * m->at.z;
        o->at.y = m->at.y;
        o->at.z = a * m->at.z - b * m->at.x;
        o->flags = 0;
    }
}

void xMat3x3Transpose(xMat3x3* o, const xMat3x3* m)
{
    if (o == m) {
        {
            F32 temp = o->right.y;
            o->right.y = o->up.x;
            o->up.x = temp;
        }
        {
            F32 temp = o->right.z;
            o->right.z = o->at.x;
            o->at.x = temp;
        }
        {
            F32 temp = o->up.z;
            o->up.z = o->at.y;
            o->at.y = temp;
        }
    } else {
        o->right.x = m->right.x;
        o->right.y = m->up.x;
        o->right.z = m->at.x;
        o->up.x = m->right.y;
        o->up.y = m->up.y;
        o->up.z = m->at.y;
        o->at.x = m->right.z;
        o->at.y = m->up.z;
        o->at.z = m->at.z;
        o->flags = 0;
    }
}

// Matches in Ratatouille: https://decomp.me/scratch/JyXkz
void xMat3x3Mul(xMat3x3* o, const xMat3x3* a, const xMat3x3* b) NONMATCH("https://decomp.me/scratch/kJvZe")
{
    xMat3x3 temp;
    xMat3x3* tp;
    U32 usetemp;

    usetemp = (o == a || o == b);
    if (usetemp) {
        tp = &temp;
    } else {
        tp = o;
    }

    tp->right.x = a->right.x * b->right.x + a->right.y * b->up.x + a->right.z * b->at.x;
    tp->right.y = a->right.x * b->right.y + a->right.y * b->up.y + a->right.z * b->at.y;
    tp->right.z = a->right.x * b->right.z + a->right.y * b->up.z + a->right.z * b->at.z;
    tp->up.x = a->up.x * b->right.x + a->up.y * b->up.x + a->up.z * b->at.x;
    tp->up.y = a->up.x * b->right.y + a->up.y * b->up.y + a->up.z * b->at.y;
    tp->up.z = a->up.x * b->right.z + a->up.y * b->up.z + a->up.z * b->at.z;
    tp->at.x = a->at.x * b->right.x + a->at.y * b->up.x + a->at.z * b->at.x;
    tp->at.y = a->at.x * b->right.y + a->at.y * b->up.y + a->at.z * b->at.y;
    tp->at.z = a->at.x * b->right.z + a->at.y * b->up.z + a->at.z * b->at.z;
    tp->flags = 0;

    if (usetemp) {
        xMat3x3Copy(o, tp);
    }
}

void xMat3x3LMulVec(xVec3* o, const xMat3x3* m, const xVec3* v)
{
    F32 x = m->right.x * v->x + m->right.y * v->y + m->right.z * v->z;
    F32 y = m->up.x * v->x + m->up.y * v->y + m->up.z * v->z;
    F32 z = m->at.x * v->x + m->at.y * v->y + m->at.z * v->z;
    o->x = x, o->y = y, o->z = z;
}

void xMat3x3Tolocal(xVec3* o, const xMat3x3* m, const xVec3* v)
{
    F32 sx2 = xsqr(m->right.x) + xsqr(m->right.y) + xsqr(m->right.z);
    F32 sy2 = xsqr(m->up.x) + xsqr(m->up.y) + xsqr(m->up.z);
    F32 sz2 = xsqr(m->at.x) + xsqr(m->at.y) + xsqr(m->at.z);
    xMat3x3LMulVec(o, m, v);
    o->x /= sx2, o->y /= sy2, o->z /= sz2;
}

void xMat4x3Rot(xMat4x3* m, const xVec3* a, F32 t, const xVec3* p)
{
    xMat3x3RotC(m, a->x, a->y, a->z, t);
    xVec3Copy(&m->pos, p);

    xMat4x3 temp;
    xMat3x3Identity(&temp);
    xVec3Inv(&temp.pos, p);
    xMat4x3Mul(m, &temp, m);
}

void xMat4x3Mul(xMat4x3* o, const xMat4x3* a, const xMat4x3* b)
{
    xVec3 pos;
    xMat4x3Toworld(&pos, b, &a->pos);
    xMat3x3Mul(o, a, b);
    xVec3Copy(&o->pos, &pos);
}

// Matching in Ratatouille, minus debug stuff: https://decomp.me/scratch/VthMZ
void xQuatFromMat(xQuat* q, const xMat3x3* m) NONMATCH("https://decomp.me/scratch/p76fl")
{
    F32* mp = (F32*)m;
    F32* qvp = (F32*)q;
    F32 tr = m->right.x + m->up.y + m->at.z;
    F32 root;

    if (tr > 0.0f) {
        root = xsqrt(1.0f + tr);
        q->s = 0.5f * root;
        root = 0.5f / root;
        q->v.x = root * (m->at.y - m->up.z);
        q->v.y = root * (m->right.z - m->at.x);
        q->v.z = root * (m->up.x - m->right.y);
    } else {
        static S32 nxt[3] = { 1, 2, 0 };
        
        S32 i = 0;
        if (mp[5] > mp[0]) i = 1;
        if (mp[10] > mp[i*5]) i = 2;
        
        S32 j = nxt[i];
        S32 k = nxt[j];

        root = xsqrt(mp[i*5] - mp[j*5] - mp[k*5] + 1.0f);
        if (xabs(root) < EPSILON) {
            xQuatCopy(q, &g_IQ);
            return;
        }

        qvp[i] = 0.5f * root;
        root = 0.5f / root;
        q->s = root * (mp[j+k*4] - mp[k+j*4]);
        qvp[j] = root * (mp[i+j*4] + mp[j+i*4]);
        qvp[k] = root * (mp[i+k*4] + mp[k+i*4]);
        
        if (q->s < 0.0f) {
            xQuatFlip(q, q);
        }
    }
}

void xQuatFromAxisAngle(xQuat* q, const xVec3* a, F32 t)
{
    if (t == 0.0f) {
        xQuatCopy(q, &g_IQ);
        return;
    }

    F32 t_2 = 0.5f * t;
    F32 st_2 = isin(t_2);

    q->s = icos(t_2);

    xVec3SMul(&q->v, a, st_2);
}

// Matching in Ratatouille, minus debug stuff: https://decomp.me/scratch/3BoQj
void xQuatToMat(const xQuat* q, xMat3x3* m) NONMATCH("https://decomp.me/scratch/AKCBu")
{
    F32 tx = 2.0f * q->v.x;
    F32 ty = 2.0f * q->v.y;
    F32 tz = 2.0f * q->v.z;
    F32 tsx = tx * q->s;
    F32 tsy = ty * q->s;
    F32 tsz = tz * q->s;
    F32 txx = tx * q->v.x;
    F32 txy = ty * q->v.x;
    F32 txz = tz * q->v.x;
    F32 tyy = ty * q->v.y;
    F32 tyz = tz * q->v.y;
    F32 tzz = tz * q->v.z;
    m->right.x = 1.0f - tyy - tzz;
    m->right.y = txy - tsz;
    m->right.z = txz + tsy;
    m->up.x = txy + tsz;
    m->up.y = 1.0f - tzz - txx;
    m->up.z = tyz - tsx;
    m->at.x = txz - tsy;
    m->at.y = tyz + tsx;
    m->at.z = 1.0f - txx - tyy;
    m->flags = 0;
}

void xQuatToAxisAngle(const xQuat* q, xVec3* a, F32* t)
{
    *t = 2.0f * xacos(q->s);
    xVec3Normalize(a, &q->v);
}

F32 xQuatNormalize(xQuat* o, const xQuat* q)
{
    F32 one_len, len, len2;
    len2 = xQuatLength2(q);
    if (len2 == 1.0f) {
        if (o != q) {
            xQuatCopy(o, q);
        }
        return 1.0f;
    }

    if (len2 == 0.0f) {
        if (o != q) {
            xQuatCopy(o, &g_IQ);
        }
        return 0.0f;
    }

    len = xsqrt(len2);
    one_len = 1.0f / len;
    xQuatSMul(o, q, one_len);
    return len;
}

void xQuatSlerp(xQuat* o, const xQuat* a, const xQuat* b, F32 t)
{
    F32 asph, bsph, one_sintheta, theta, abdot;
    xQuat aerp, berp, b2;

    abdot = xQuatDot(a, b);
    if (abdot < 0.0f) {
        abdot = -abdot;
        b2.v.x = -b->v.x, b2.v.y = -b->v.y, b2.v.z = -b->v.z, b2.s = -b->s;
        b = &b2;
    }

    if (abdot >= 0.999f) {
        bsph = t;
        asph = 1.0f - t;
    } else {
        theta = xacos(abdot);
        one_sintheta = 1.0f / isin(theta);
        asph = one_sintheta * isin((1.0f - t) * theta);
        bsph = one_sintheta * isin(t * theta);
    }

    xQuatSMul(&aerp, a, asph);
    xQuatSMul(&berp, b, bsph);
    xQuatAdd(o, &aerp, &berp);
    xQuatNormalize(o, o);
}

void xQuatMul(xQuat* o, const xQuat* a, const xQuat* b)
{
    F32 _s = a->s * b->s - a->v.x * b->v.x - a->v.y * b->v.y - a->v.z * b->v.z;
    F32 _x = a->s * b->v.x + a->v.x * b->s + a->v.y * b->v.z - a->v.z * b->v.y;
    F32 _y = a->s * b->v.y + a->v.y * b->s + a->v.z * b->v.x - a->v.x * b->v.z;
    F32 _z = a->s * b->v.z + a->v.z * b->s + a->v.x * b->v.y - a->v.y * b->v.x;
    o->v.x = _x, o->v.y = _y, o->v.z = _z, o->s = _s;
    xQuatNormalize(o, o);
}

void xQuatDiff(xQuat* o, const xQuat* a, const xQuat* b)
{
    xQuatConj(o, a);
    xQuatMul(o, o, b);
    if (o->s < 0.0f) {
        xQuatFlip(o, o);
    }
}
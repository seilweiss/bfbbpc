#define xVec3Center(o, a, b)\
    xVec3Add((o), (a), (b));\
    xVec3SMul((o), (o), 0.5f)

#define xVec3DistMacro(a, b, dist)                                                                 \
MACROSTART                                                                                         \
    F32 dx__ = (a)->x - (b)->x;                                                                    \
    F32 dy__ = (a)->y - (b)->y;                                                                    \
    F32 dz__ = (a)->z - (b)->z;                                                                    \
    *(dist) = xsqrt(xsqr(dx__) + xsqr(dy__) + xsqr(dz__));                                         \
MACROEND

#define xVec3Dist2Macro(a, b, dist)                                                                \
MACROSTART                                                                                         \
    F32 dx__ = (a)->x - (b)->x;                                                                    \
    F32 dy__ = (a)->y - (b)->y;                                                                    \
    F32 dz__ = (a)->z - (b)->z;                                                                    \
    *(dist) = xsqr(dx__) + xsqr(dy__) + xsqr(dz__);                                                \
MACROEND

#define xVec3NormalizeMacro(o, v, len)                                                             \
MACROSTART                                                                                         \
    F32 len2 = xsqr((v)->x) + xsqr((v)->y) + xsqr((v)->z);                                         \
    if (xeq(len2, 1.0f, EPSILON))                                                                    \
    {                                                                                              \
        (o)->x = (v)->x;                                                                           \
        (o)->y = (v)->y;                                                                           \
        (o)->z = (v)->z;                                                                           \
        *(len) = 1.0f;                                                                             \
    }                                                                                              \
    else if (xeq(len2, 0.0f, EPSILON))                                                               \
    {                                                                                              \
        (o)->x = 0.0f;                                                                             \
        (o)->y = 1.0f;                                                                             \
        (o)->z = 0.0f;                                                                             \
        *(len) = 0.0f;                                                                             \
    }                                                                                              \
    else                                                                                           \
    {                                                                                              \
        *(len) = xsqrt(len2);                                                                      \
        F32 len_inv = 1.0f / *(len);                                                               \
        (o)->x = (v)->x * len_inv;                                                                 \
        (o)->y = (v)->y * len_inv;                                                                 \
        (o)->z = (v)->z * len_inv;                                                                 \
    }                                                                                              \
MACROEND

#define xVec3NormalizeFastMacro(o, v, len)                                                         \
MACROSTART                                                                                         \
    F32 len2 = xsqr((v)->x) + xsqr((v)->y) + xsqr((v)->z);                                         \
    if (xeq(len2, 1.0f, EPSILON))                                                                    \
    {                                                                                              \
        (o)->x = (v)->x;                                                                           \
        (o)->y = (v)->y;                                                                           \
        (o)->z = (v)->z;                                                                           \
        *(len) = 1.0f;                                                                             \
    }                                                                                              \
    else if (xeq(len2, 0.0f, EPSILON))                                                               \
    {                                                                                              \
        (o)->x = 0.0f;                                                                             \
        (o)->y = 1.0f;                                                                             \
        (o)->z = 0.0f;                                                                             \
        *(len) = 0.0f;                                                                             \
    }                                                                                              \
    else                                                                                           \
    {                                                                                              \
        xsqrtfast(*(len), len2);                                                                   \
        F32 len_inv = 1.0f / *(len);                                                               \
        (o)->x = (v)->x * len_inv;                                                                 \
        (o)->y = (v)->y * len_inv;                                                                 \
        (o)->z = (v)->z * len_inv;                                                                 \
    }                                                                                              \
MACROEND

#define xVec3NormalizeDistMacro(o, a, b, dist)                                                     \
MACROSTART                                                                                         \
    F32 dx__ = (b)->x - (a)->x;                                                                    \
    F32 dy__ = (b)->y - (a)->y;                                                                    \
    F32 dz__ = (b)->z - (a)->z;                                                                    \
    F32 dist2 = xsqr(dx__) + xsqr(dy__) + xsqr(dz__);                                              \
    if (xeq(dist2, 1.0f, EPSILON))                                                                   \
    {                                                                                              \
        (o)->x = dx__;                                                                             \
        (o)->y = dy__;                                                                             \
        (o)->z = dz__;                                                                             \
        *(dist) = 1.0f;                                                                            \
    }                                                                                              \
    else if (xeq(dist2, 0.0f, EPSILON))                                                              \
    {                                                                                              \
        (o)->x = 0.0f;                                                                             \
        (o)->y = 1.0f;                                                                             \
        (o)->z = 0.0f;                                                                             \
        *(dist) = 0.0f;                                                                            \
    }                                                                                              \
    else                                                                                           \
    {                                                                                              \
        *(dist) = xsqrt(dist2);                                                                    \
        F32 dist_inv = 1.0f / *(dist);                                                             \
        (o)->x = dx__ * dist_inv;                                                                  \
        (o)->y = dy__ * dist_inv;                                                                  \
        (o)->z = dz__ * dist_inv;                                                                  \
    }                                                                                              \
MACROEND

#define xVec3NormalizeDistMacro2(ox, oy, oz, a, b, dist)                                                     \
MACROSTART                                                                                         \
    F32 dx__ = (b)->x - (a)->x;                                                                    \
    F32 dy__ = (b)->y - (a)->y;                                                                    \
    F32 dz__ = (b)->z - (a)->z;                                                                    \
    F32 dist2 = xsqr(dx__) + xsqr(dy__) + xsqr(dz__);                                              \
    if (xeq(dist2, 1.0f, 1e-5f))                                                                   \
    {                                                                                              \
        *(ox) = dx__;                                                                             \
        *(oy) = dy__;                                                                             \
        *(oz) = dz__;                                                                             \
        *(dist) = 1.0f;                                                                            \
    }                                                                                              \
    else if (xeq(dist2, 0.0f, 1e-5f))                                                              \
    {                                                                                              \
        *(ox) = 0.0f;                                                                             \
        *(oy) = 1.0f;                                                                             \
        *(oz) = 0.0f;                                                                             \
        *(dist) = 0.0f;                                                                            \
    }                                                                                              \
    else                                                                                           \
    {                                                                                              \
        *(dist) = xsqrt(dist2);                                                                    \
        F32 dist_inv = 1.0f / *(dist);                                                             \
        *(ox) = dx__ * dist_inv;                                                                  \
        *(oy) = dy__ * dist_inv;                                                                  \
        *(oz) = dz__ * dist_inv;                                                                  \
    }                                                                                              \
MACROEND

#define xVec3NormalizeDistXZMacro(o, a, b, dist)                                                   \
MACROSTART                                                                                         \
    F32 dx__ = (b)->x - (a)->x;                                                                    \
    F32 dz__ = (b)->z - (a)->z;                                                                    \
    F32 dist2 = xsqr(dx__) + xsqr(dz__);                                                           \
    if (xeq(dist2, 1.0f, EPSILON))                                                                   \
    {                                                                                              \
        (o)->x = dx__;                                                                             \
        (o)->z = dz__;                                                                             \
        *(dist) = 1.0f;                                                                            \
    }                                                                                              \
    else if (xeq(dist2, 0.0f, EPSILON))                                                              \
    {                                                                                              \
        (o)->x = 0.0f;                                                                             \
        (o)->z = 0.0f;                                                                             \
        *(dist) = 0.0f;                                                                            \
    }                                                                                              \
    else                                                                                           \
    {                                                                                              \
        *(dist) = xsqrt(dist2);                                                                    \
        F32 dist_inv = 1.0f / *(dist);                                                             \
        (o)->x = dx__ * dist_inv;                                                                  \
        (o)->z = dz__ * dist_inv;                                                                  \
    }                                                                                              \
MACROEND

inline void xVec3Init(xVec3* v, F32 _x, F32 _y, F32 _z)
{
    v->x = _x, v->y = _y, v->z = _z;
}

inline U32 xVec3Equals(const xVec3* a, const xVec3* b)
{
    return xeq(a->x, b->x, 0.0f) && xeq(a->y, b->y, 0.0f) && xeq(a->z, b->z, 0.0f);
}

inline void xVec3Add(xVec3* o, const xVec3* a, const xVec3* b)
{
    o->x = a->x + b->x, o->y = a->y + b->y, o->z = a->z + b->z;
}

inline void xVec3AddTo(xVec3* a, const xVec3* b)
{
    a->x += b->x, a->y += b->y, a->z += b->z;
}

inline void xVec3AddScaled(xVec3* a, const xVec3* b, F32 scale)
{
    a->x += b->x * scale, a->y += b->y * scale, a->z += b->z * scale;
}

inline void xVec3Sub(xVec3* o, const xVec3* a, const xVec3* b)
{
    o->x = a->x - b->x, o->y = a->y - b->y, o->z = a->z - b->z;
}

inline void xVec3SubFrom(xVec3* a, const xVec3* b)
{
    a->x -= b->x, a->y -= b->y, a->z -= b->z;
}

inline void xVec3SMul(xVec3* o, const xVec3* v, F32 s)
{
    o->x = v->x * s, o->y = v->y * s, o->z = v->z * s;
}

inline void xVec3SMulBy(xVec3* v, F32 s)
{
    v->x *= s, v->y *= s, v->z *= s;
}

inline void xVec3Cross(xVec3* o, const xVec3* a, const xVec3* b)
{
    o->x = a->y * b->z - b->y * a->z, o->y = a->z * b->x - b->z * a->x, o->z = a->x * b->y - b->x * a->y;
}

inline F32 xVec3Length(const xVec3* v)
{
    return xsqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

inline F32 xVec3LengthFast(F32 _x, F32 _y, F32 _z)
{
    F32 len;
    xsqrtfast(len, xsqr(_x) + xsqr(_y) + xsqr(_z));
    return len;
}

inline F32 xVec3Length2(const xVec3* v)
{
    return v->x * v->x + v->y * v->y + v->z * v->z;
}

inline void xVec3Inv(xVec3* o, const xVec3* v)
{
    o->x = -v->x, o->y = -v->y, o->z = -v->z;
}

inline F32 xVec3Hdng(xVec3* hdng, const xVec3* a, const xVec3* b) NONMATCH("https://decomp.me/scratch/zAnXF")
{
    F32 d;
    xVec3NormalizeDistMacro(hdng, a, b, &d);
    return d;
}

inline F32 xVec3Dist(const xVec3* a, const xVec3* b)
{
    F32 d;
    xVec3DistMacro(a, b, &d);
    return d;
}

inline F32 xVec3Dist2(const xVec3* a, const xVec3* b)
{
    F32 d;
    xVec3Dist2Macro(a, b, &d);
    return d;
}

inline void xVec3Lerp(xVec3* o, const xVec3* a, const xVec3* b, F32 f)
{
    o->x = a->x + f * (b->x - a->x), o->y = a->y + f * (b->y - a->y), o->z = a->z + f * (b->z - a->z);
}
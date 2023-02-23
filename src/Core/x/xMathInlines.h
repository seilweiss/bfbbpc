inline F32 xsqrt(F32 x)
{
    return std::sqrtf(x);
}

inline F32 xexp(F32 x) 
{
    return std::expf(x);
}

inline F32 xpow(F32 x, F32 y)
{
    return std::powf(x, y);
}

inline F32 xfmod(F32 x, F32 y)
{
    return std::fmodf(x, y);
}

inline F32 xacos(F32 x)
{
    return std::acosf(x);
}

inline F32 xasin(F32 x)
{
    return std::asinf(x);
}

inline F32 xatan2(F32 y, F32 x)
{
    return xAngleClampFast(std::atan2f(y, x));
}

inline void xsqrtfast(F32& o, F32 fVal)
{
    o = std::sqrtf(fVal);
}
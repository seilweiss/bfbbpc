#include "xVec3.h"

xVec3 xVec3::m_Null = { 0.0f, 0.0f, 0.0f };
xVec3 xVec3::m_Ones = { 1.0f, 1.0f, 1.0f };
xVec3 xVec3::m_UnitAxisX = { 1.0f, 0.0f, 0.0f };
xVec3 xVec3::m_UnitAxisY = { 0.0f, 1.0f, 0.0f };
xVec3 xVec3::m_UnitAxisZ = { 0.0f, 0.0f, 1.0f };

F32 xVec3Normalize(xVec3* o, const xVec3* v) NONMATCH("https://decomp.me/scratch/cyARR")
{
    F32 len;
    xVec3NormalizeMacro(o, v, &len);
    return len;
}

F32 xVec3NormalizeFast(xVec3* o, const xVec3* v) NONMATCH("https://decomp.me/scratch/iRNBb")
{
    F32 len;
    xVec3NormalizeFastMacro(o, v, &len);
    return len;
}

void xVec3Copy(xVec3* o, const xVec3* v)
{
    o->x = v->x, o->y = v->y, o->z = v->z;
}

F32 xVec3Dot(const xVec3* vec1, const xVec3* vec2)
{
    return vec1->x * vec2->x + vec1->y * vec2->y + vec1->z * vec2->z;
}
#pragma once

#include "xMath.h"

struct xVec3
{
    F32 x;
    F32 y;
    F32 z;

    static xVec3 m_Null;
    static xVec3 m_Ones;
    static xVec3 m_UnitAxisX;
    static xVec3 m_UnitAxisY;
    static xVec3 m_UnitAxisZ;

    xVec3& assign(F32 x, F32 y, F32 z) { this->x = x, this->y = y, this->z = z; return *this; }
    xVec3& operator+=(const xVec3& v) { x += v.x, y += v.y, z += v.z; return *this; }
    xVec3& operator+=(F32 f) { x += f, y += f, z += f; return *this; }
    xVec3 operator+(const xVec3& v) const { xVec3 temp = *this; temp += v; return temp; }
    xVec3 operator+(F32 f) const { xVec3 temp = *this; temp += f; return temp; }
    xVec3& operator-=(const xVec3& v) { x -= v.x, y -= v.y, z -= v.z; return *this; }
    xVec3& operator-=(F32 f) { x -= f, y -= f, z -= f; return *this; }
    xVec3 operator-(const xVec3& v) const { xVec3 temp = *this; temp -= v; return temp; }
    xVec3 operator-(F32 f) const { xVec3 temp = *this; temp -= f; return temp; }
    xVec3& operator*=(const xVec3& v) { x *= v.x, y *= v.y, z *= v.z; return *this; }
    xVec3& operator*=(F32 f) { x *= f, y *= f, z *= f; return *this; }
    xVec3 operator*(const xVec3& v) const { xVec3 temp = *this; temp *= v; return temp; }
    xVec3 operator*(F32 f) const { xVec3 temp = *this; temp *= f; return temp; }
    xVec3& operator/=(const xVec3& v) { x /= v.x, y /= v.y, z /= v.z; return *this; }
    xVec3& operator/=(F32 f) { F32 i = 1.0f / f; x *= i, y *= i, z *= i; return *this; }
    xVec3 operator/(const xVec3& v) const { xVec3 temp = *this; temp /= v; return temp; }
    xVec3 operator/(F32 f) const { xVec3 temp = *this; temp /= f; return temp; }
    xVec3& operator=(F32 f) { x = f, y = f, z = f; return *this; }
    xVec3 cross(const xVec3& c) const { xVec3 v = { 0, 0, 0 }; v.x = y * c.z - c.y * z, v.y = z * c.x - c.z * x, v.z = x * c.y - c.x * y; return v; }
    F32 dot(const xVec3& c) const { return x * c.x + y * c.y + z * c.z; }
    F32 length() const { return xsqrt(length2()); }
    F32 length2() const { return x * x + y * y + z * z; }
    xVec3& invert() { x = -x, y = -y, z = -z; return *this; }
    xVec3& normalize() { *this /= length(); return *this; }
    xVec3& set_abs() { x = xabs(x), y = xabs(y), z = xabs(z); return *this; }
    xVec3 get_abs() const { xVec3 temp = *this; return temp.set_abs(); }

    void safe_normalize(const xVec3& safety)
    {
        F32 len = length2();
        if (len < 1e-4f)
            *this = safety;
        else
            *this *= 1.0f / xsqrt(len);
    }

    void right_normalize() { safe_normalize(m_UnitAxisX); }
};

F32 xVec3Normalize(xVec3* o, const xVec3* v);
F32 xVec3NormalizeFast(xVec3* o, const xVec3* v);
void xVec3Copy(xVec3* o, const xVec3* v);
F32 xVec3Dot(const xVec3* vec1, const xVec3* vec2);

#include "xVec3Inlines.h"
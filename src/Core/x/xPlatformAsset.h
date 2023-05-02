#pragma once

#include "xMath3.h"

struct xPlatformERData
{
    S32 nodata;
};

struct xPlatformOrbitData
{
    S32 nodata;
};

struct xPlatformSplineData
{
    S32 nodata;
};

struct xPlatformMPData
{
    S32 nodata;
};

struct xPlatformMechData
{
    S32 nodata;
};

struct xPlatformPenData
{
    S32 nodata;
};

struct xPlatformConvBeltData
{
    F32 speed;
};

struct xPlatformFallingData
{
    F32 speed;
    U32 bustModelID;
};

struct xPlatformFRData
{
    F32 fspeed;
    F32 rspeed;
    F32 ret_delay;
    F32 post_ret_delay;
};

struct xPlatformBreakawayData
{
    F32 ba_delay;
    U32 bustModelID;
    F32 reset_delay;
    U32 breakflags;
};

struct xPlatformSpringboardData
{
    F32 jmph[3];
    F32 jmpbounce;
    U32 animID[3];
    xVec3 jmpdir;
    U32 springflags;
};

struct xPlatformTeeterData
{
    F32 itilt;
    F32 maxtilt;
    F32 invmass;
};

struct xPlatformPaddleData
{
    S32 startOrient;
    S32 countOrient;
    F32 orientLoop;
    F32 orient[6];
    U32 paddleFlags;
    F32 rotateSpeed;
    F32 accelTime;
    F32 decelTime;
    F32 hubRadius;
};

struct xPlatformFMData
{
    S32 nothingyet;
};

struct xPlatformAsset
{
    U8 type;
    U8 pad;
    U16 flags;
    union
    {
        xPlatformERData er;
        xPlatformOrbitData orb;
        xPlatformSplineData spl;
        xPlatformMPData mp;
        xPlatformMechData mech;
        xPlatformPenData pen;
        xPlatformConvBeltData cb;
        xPlatformFallingData fall;
        xPlatformFRData fr;
        xPlatformBreakawayData ba;
        xPlatformSpringboardData sb;
        xPlatformTeeterData teet;
        xPlatformPaddleData paddle;
        xPlatformFMData fm;
    };
};
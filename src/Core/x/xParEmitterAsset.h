#pragma once

#include "xBaseAsset.h"
#include "xMath3.h"

struct xPECircle
{
    F32 radius;
    F32 deflection;
    xVec3 dir;
};

typedef struct _tagEmitSphere
{
    F32 radius;
} xPESphere;

typedef struct _tagEmitRect
{
    F32 x_len;
    F32 z_len;
} xPERect;

typedef struct _tagEmitLine
{
    xVec3 pos1;
    xVec3 pos2;
    F32 radius;
} xPELine;

typedef struct _tagEmitVolume
{
    U32 emit_volumeID;
} xPEVolume;

typedef struct _tagEmitOffsetPoint
{
    xVec3 offset;
} xPEOffsetPoint;

struct xPEVCyl
{
    F32 height;
    F32 radius;
    F32 deflection;
};

struct xPEEntBone
{
    U8 flags;
    U8 type;
    U8 bone;
    U8 pad1;
    xVec3 offset;
    F32 radius;
    F32 deflection;
};

struct xPEEntBound
{
    U8 flags;
    U8 type;
    U8 pad1;
    U8 pad2;
    F32 expand;
    F32 deflection;
};

struct xParEmitterAsset : xBaseAsset
{
    U8 emit_flags;
    U8 emit_type;
    U16 pad;
    U32 propID;
    union
    {
        xPECircle e_circle;
        xPESphere e_sphere;
        xPERect e_rect;
        xPELine e_line;
        xPEVolume e_volume;
        xPEOffsetPoint e_offsetp;
        xPEVCyl e_vcyl;
        xPEEntBone e_entbone;
        xPEEntBound e_entbound;
    };
    U32 attachToID;
    xVec3 pos;
    xVec3 vel;
    F32 vel_angle_variation;
    U32 cull_mode;
    F32 cull_dist_sqr;
};

struct xParInterp
{
    F32 val[2];
    U32 interp;
    F32 freq;
    F32 oofreq;
};

struct xParEmitterPropsAsset : xBaseAsset
{
    U32 parSysID;
    union
    {
        xParInterp rate;
        xParInterp value[1];
    };
    xParInterp life;
    xParInterp size_birth;
    xParInterp size_death;
    xParInterp color_birth[4];
    xParInterp color_death[4];
    xParInterp vel_scale;
    xParInterp vel_angle;
    xVec3 vel;
    U32 emit_limit;
    F32 emit_limit_reset_time;
};
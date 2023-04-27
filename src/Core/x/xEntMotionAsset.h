#pragma once

#include "xVec3.h"

struct xEntMotionERData
{
    xVec3 ret_pos;
    xVec3 ext_dpos;
    F32 ext_tm;
    F32 ext_wait_tm;
    F32 ret_tm;
    F32 ret_wait_tm;
};

struct xEntMotionOrbitData
{
    xVec3 center;
    F32 w;
    F32 h;
    F32 period;
};

struct xEntMotionSplineData
{
    S32 unknown;
};

struct xEntMotionMPData
{
    U32 flags;
    U32 mp_id;
    F32 speed;
};

struct xEntMotionMechData
{
    U8 type;
    U8 flags;
    U8 sld_axis;
    U8 rot_axis;
    F32 sld_dist;
    F32 sld_tm;
    F32 sld_acc_tm;
    F32 sld_dec_tm;
    F32 rot_dist;
    F32 rot_tm;
    F32 rot_acc_tm;
    F32 rot_dec_tm;
    F32 ret_delay;
    F32 post_ret_delay;
};

enum
{
    k_XENTMOTIONMECH_SLIDE,
    k_XENTMOTIONMECH_ROT,
    k_XENTMOTIONMECH_SLIDE_ROT,
    k_XENTMOTIONMECH_SLIDE_THEN_ROT,
    k_XENTMOTIONMECH_ROT_THEN_SLIDE
};

enum
{
    k_XENTMOTIONMECH_RETURNS = (1<<0),
    k_XENTMOTIONMECH_ONCE = (1<<1),
};

struct xEntMotionPenData
{
    U8 flags;
    U8 plane;
    U8 pad[2];
    F32 len;
    F32 range;
    F32 period;
    F32 phase;
};

struct xEntMotionAsset
{
    U8 type;
    U8 use_banking;
    U16 flags;
    union
    {
        xEntMotionERData er;
        xEntMotionOrbitData orb;
        xEntMotionSplineData spl;
        xEntMotionMPData mp;
        xEntMotionMechData mech;
        xEntMotionPenData pen;
    };
};

enum
{
    k_XENTMOTIONTYPE_ER,
    k_XENTMOTIONTYPE_ORBIT,
    k_XENTMOTIONTYPE_SPLINE,
    k_XENTMOTIONTYPE_MP,
    k_XENTMOTIONTYPE_MECH,
    k_XENTMOTIONTYPE_PEND,
    k_XENTMOTIONTYPE_NONE
};

enum
{
    k_XENTMOTION_0x1 = (1<<0),
    k_XENTMOTION_STOPPED = (1<<2)
};
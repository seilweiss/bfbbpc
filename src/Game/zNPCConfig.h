#pragma once

#include "xListItem.h"
#include "xMath3.h"
#include "xModel.h"

struct NPCSndTrax;

struct NPCConfig : xListItem<NPCConfig>
{
    U32 modelID;
    S32 flg_config;
    F32 spd_turnMax;
    F32 spd_moveMax;
    F32 fac_accelMax;
    F32 fac_driftMax;
    F32 fac_gravKnock;
    F32 fac_elastic;
    S32 pts_damage;
    S32 useBoxBound;
    xVec3 off_bound;
    xVec3 dim_bound;
    F32 npcMass;
    F32 npcMassInv;
    F32 rad_detect;
    F32 hyt_detect;
    F32 off_detect;
    F32 rad_attack;
    F32 fov_attack;
    xVec3 scl_model;
    F32 tym_attack;
    F32 tym_fidget;
    F32 tym_stun;
    F32 tym_alert;
    F32 dst_castShadow;
    F32 rad_shadowCache;
    F32 rad_shadowRaster;
    F32 rad_dmgSize;
    S32 flg_vert;
    xModelTag tag_vert[20];
    xVec3 animFrameRange[9];
    S32 cnt_esteem[5];
    F32 rad_sound;
    NPCSndTrax* snd_trax;
    NPCSndTrax* snd_traxShare;
    S32 test_count;
    U8 talk_filter[4];
    U8 talk_filter_size;
};
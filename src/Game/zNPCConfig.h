#pragma once

#include "xListItem.h"
#include "xMath3.h"
#include "xModel.h"

struct NPCSndTrax;

enum en_mdlvert
{
    NPC_MDLVERT_ATTACKBASE,
    NPC_MDLVERT_ATTACK,
    NPC_MDLVERT_ATTACK1,
    NPC_MDLVERT_ATTACK2,
    NPC_MDLVERT_ATTACK3,
    NPC_MDLVERT_ATTACK4,
    NPC_MDLVERT_LOSEYEBALL,
    NPC_MDLVERT_DMGSMOKE_A,
    NPC_MDLVERT_DMGSMOKE_B,
    NPC_MDLVERT_DMGSMOKE_C,
    NPC_MDLVERT_DMGFLAME_A,
    NPC_MDLVERT_DMGFLAME_B,
    NPC_MDLVERT_DMGFLAME_C,
    NPC_MDLVERT_PROPEL,
    NPC_MDLVERT_EXHAUST,
    NPC_MDLVERT_GEN01,
    NPC_MDLVERT_GEN02,
    NPC_MDLVERT_GEN03,
    NPC_MDLVERT_GEN04,
    NPC_MDLVERT_GEN05,
    NPC_MDLVERT_NOMORE,
    NPC_MDLVERT_FORCEINT = FORCEENUMSIZEINT
};

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
    xModelTag tag_vert[NPC_MDLVERT_NOMORE];
    xVec3 animFrameRange[9];
    S32 cnt_esteem[5];
    F32 rad_sound;
    NPCSndTrax* snd_trax;
    NPCSndTrax* snd_traxShare;
    S32 test_count;
    U8 talk_filter[4];
    U8 talk_filter_size;
};
#pragma once

#include "zEnt.h"
#include "zLasso.h"

struct zJumpParam
{
    F32 PeakHeight;
    F32 TimeGravChange;
    F32 TimeHold;
    F32 ImpulseVel;
};

struct zLedgeGrabParams
{
    F32 animGrab;
    F32 zdist;
    xVec3 tranTable[60];
    S32 tranCount;
    xEnt* optr;
    xMat4x3 omat;
    F32 y0det;
    F32 dydet;
    F32 r0det;
    F32 drdet;
    F32 thdet;
    F32 rtime;
    F32 ttime;
    F32 tmr;
    xVec3 spos;
    xVec3 epos;
    xVec3 tpos;
    S32 nrays;
    S32 rrand;
    F32 startrot;
    F32 endrot;
};

enum _zPlayerType
{
    ePlayer_SB,
    ePlayer_Patrick,
    ePlayer_Sandy,
    ePlayer_MAXTYPES
};
typedef enum _zPlayerType zPlayerType;

struct zPlayerSettings
{
    zPlayerType pcType;
    F32 MoveSpeed[6];
    F32 AnimSneak[3];
    F32 AnimWalk[3];
    F32 AnimRun[3];
    F32 JumpGravity;
    F32 GravSmooth;
    F32 FloatSpeed;
    F32 ButtsmashSpeed;
    zJumpParam Jump;
    zJumpParam Bounce;
    zJumpParam Spring;
    zJumpParam Wall;
    zJumpParam Double;
    zJumpParam SlideDouble;
    zJumpParam SlideJump;
    F32 WallJumpVelocity;
    zLedgeGrabParams ledge;
    F32 spin_damp_xz;
    F32 spin_damp_y;
    U8 talk_anims;
    U8 talk_filter_size;
    U8 talk_filter[4];
};

struct zPlayerCarryInfo
{
    xEnt* grabbed;
    U32 grabbedModelID;
    xMat4x3 spin;
    xEnt* throwTarget;
    xEnt* flyingToTarget;
    F32 minDist;
    F32 maxDist;
    F32 minHeight;
    F32 maxHeight;
    F32 maxCosAngle;
    F32 throwMinDist;
    F32 throwMaxDist;
    F32 throwMinHeight;
    F32 throwMaxHeight;
    F32 throwMaxStack;
    F32 throwMaxCosAngle;
    F32 throwTargetRotRate;
    F32 targetRot;
    U32 grabTarget;
    xVec3 grabOffset;
    F32 grabLerpMin;
    F32 grabLerpMax;
    F32 grabLerpLast;
    U32 grabYclear;
    F32 throwGravity;
    F32 throwHeight;
    F32 throwDistance;
    F32 fruitFloorDecayMin;
    F32 fruitFloorDecayMax;
    F32 fruitFloorBounce;
    F32 fruitFloorFriction;
    F32 fruitCeilingBounce;
    F32 fruitWallBounce;
    F32 fruitLifetime;
    xEnt* patLauncher;
};

struct zPlayerLassoInfo
{
    xEnt* target;
    F32 dist;
    bool destroy;
    bool targetGuide;
    F32 lassoRot;
    xEnt* swingTarget;
    xEnt* releasedSwing;
    F32 copterTime;
    S32 canCopter;
    zLasso lasso;
    xAnimState* zeroAnim;
};

enum _zPlayerWallJumpState
{
    k_WALLJUMP_NOT,
    k_WALLJUMP_LAUNCH,
    k_WALLJUMP_FLIGHT,
    k_WALLJUMP_LAND
};
typedef enum _zPlayerWallJumpState zPlayerWallJumpState;

enum zControlOwner
{
    CONTROL_OWNER_GLOBAL        = (1<<0),
    CONTROL_OWNER_EVENT         = (1<<1),
    CONTROL_OWNER_OOB           = (1<<2),
    CONTROL_OWNER_BOSS          = (1<<3),
    CONTROL_OWNER_TALK_BOX      = (1<<4),
    CONTROL_OWNER_TAXI          = (1<<5),
    CONTROL_OWNER_BUS_STOP      = (1<<6),
    CONTROL_OWNER_TELEPORT_BOX  = (1<<7),
    CONTROL_OWNER_CRUISE_BUBBLE = (1<<8),
    CONTROL_OWNER_FLY_CAM       = (1<<9),
    CONTROL_OWNER_FROZEN        = (1<<10),
    CONTROL_OWNER_TURRET        = (1<<11),
    CONTROL_OWNER_REWARDANIM    = (1<<12),
    CONTROL_OWNER_BUNGEE        = (1<<13),
    CONTROL_OWNER_SPRINGBOARD   = (1<<14),
    CONTROL_OWNER_CUTSCENE      = (1<<15)
};

void zEntPlayerControlOn(zControlOwner owner);
void zEntPlayerControlOff(zControlOwner owner);
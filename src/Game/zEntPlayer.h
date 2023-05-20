#pragma once

#include "zEnt.h"
#include "zLasso.h"

struct xEntBoulder;

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

typedef enum _zPlayerType
{
    ePlayer_SB,
    ePlayer_Patrick,
    ePlayer_Sandy,
    ePlayer_MAXTYPES
} zPlayerType;

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

typedef enum _zPlayerWallJumpState
{
    k_WALLJUMP_NOT,
    k_WALLJUMP_LAUNCH,
    k_WALLJUMP_FLIGHT,
    k_WALLJUMP_LAND
} zPlayerWallJumpState;

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

typedef enum _tagePlayerStreamSnd
{
    ePlayerStreamSnd_Invalid,
    ePlayerStreamSnd_PickupSock1,
    ePlayerStreamSnd_PickupSock2,
    ePlayerStreamSnd_PickupSock3,
    ePlayerStreamSnd_UnderwearComment1,
    ePlayerStreamSnd_UnderwearComment2,
    ePlayerStreamSnd_UnderwearComment3,
    ePlayerStreamSnd_EnterScene1,
    ePlayerStreamSnd_EnterScene2,
    ePlayerStreamSnd_EnterScene3,
    ePlayerStreamSnd_EnterScene4,
    ePlayerStreamSnd_EnterScene5,
    ePlayerStreamSnd_EnterScene6,
    ePlayerStreamSnd_EnterScene7,
    ePlayerStreamSnd_SpatulaComment1,
    ePlayerStreamSnd_ShinyComment1,
    ePlayerStreamSnd_ShinyComment2,
    ePlayerStreamSnd_ShinyComment3,
    ePlayerStreamSnd_ShinyComment4,
    ePlayerStreamSnd_ShinyComment5,
    ePlayerStreamSnd_SpongeBallComment1,
    ePlayerStreamSnd_SpongeBallComment2,
    ePlayerStreamSnd_SpongeBallComment3,
    ePlayerStreamSnd_CruiseComment1,
    ePlayerStreamSnd_CruiseComment2,
    ePlayerStreamSnd_CruiseComment3,
    ePlayerStreamSnd_BowlComment1,
    ePlayerStreamSnd_BowlComment2,
    ePlayerStreamSnd_BowlComment3,
    ePlayerStreamSnd_BowlComment4,
    ePlayerStreamSnd_BowlComment5,
    ePlayerStreamSnd_PushButton1,
    ePlayerStreamSnd_PushButton2,
    ePlayerStreamSnd_PushButton3,
    ePlayerStreamSnd_BellySmashComment1,
    ePlayerStreamSnd_BellySmashComment2,
    ePlayerStreamSnd_BellySmashComment3,
    ePlayerStreamSnd_ChopComment1,
    ePlayerStreamSnd_ChopComment2,
    ePlayerStreamSnd_ChopComment3,
    ePlayerStreamSnd_KickComment1,
    ePlayerStreamSnd_KickComment2,
    ePlayerStreamSnd_KickComment3,
    ePlayerStreamSnd_RopingComment1,
    ePlayerStreamSnd_RopingComment2,
    ePlayerStreamSnd_RopingComment3,
    ePlayerStreamSnd_HeliComment1,
    ePlayerStreamSnd_HeliComment2,
    ePlayerStreamSnd_HeliComment3,
    ePlayerStreamSnd_DestroyTiki1,
    ePlayerStreamSnd_DestroyTiki2,
    ePlayerStreamSnd_DestroyTiki3,
    ePlayerStreamSnd_DestroyRobot1,
    ePlayerStreamSnd_DestroyRobot2,
    ePlayerStreamSnd_DestroyRobot3,
    ePlayerStreamSnd_SeeWoodTiki,
    ePlayerStreamSnd_SeeLoveyTiki,
    ePlayerStreamSnd_SeeShhhTiki,
    ePlayerStreamSnd_SeeThunderTiki,
    ePlayerStreamSnd_SeeStoneTiki,
    ePlayerStreamSnd_SeeFodder,
    ePlayerStreamSnd_SeeHammer,
    ePlayerStreamSnd_SeeTarTar,
    ePlayerStreamSnd_SeeGLove,
    ePlayerStreamSnd_SeeMonsoon,
    ePlayerStreamSnd_SeeSleepyTime,
    ePlayerStreamSnd_SeeArf,
    ePlayerStreamSnd_SeeTubelets,
    ePlayerStreamSnd_SeeSlick,
    ePlayerStreamSnd_SeeKingJellyfish,
    ePlayerStreamSnd_SeePrawn,
    ePlayerStreamSnd_SeeDutchman,
    ePlayerStreamSnd_SeeSandyBoss,
    ePlayerStreamSnd_SeePatrickBoss1,
    ePlayerStreamSnd_SeePatrickBoss2,
    ePlayerStreamSnd_SeeSpongeBobBoss,
    ePlayerStreamSnd_SeeRobotPlankton,
    ePlayerStreamSnd_PickupSpecialGeneric1,
    ePlayerStreamSnd_PickupSpecialGeneric2,
    ePlayerStreamSnd_GoldenUnderwear4,
    ePlayerStreamSnd_GoldenUnderwear5,
    ePlayerStreamSnd_GoldenUnderwear6,
    ePlayerStreamSnd_Combo1,
    ePlayerStreamSnd_Combo2,
    ePlayerStreamSnd_Combo3,
    ePlayerStreamSnd_Combo4,
    ePlayerStreamSnd_Combo5,
    ePlayerStreamSnd_BigCombo1,
    ePlayerStreamSnd_BigCombo2,
    ePlayerStreamSnd_BigCombo3,
    ePlayerStreamSnd_BigCombo4,
    ePlayerStreamSnd_BigCombo5,
    ePlayerStreamSnd_Lift1,
    ePlayerStreamSnd_Exclaim1,
    ePlayerStreamSnd_Exclaim2,
    ePlayerStreamSnd_Exclaim3,
    ePlayerStreamSnd_Exclaim4,
    ePlayerStreamSnd_BeginBungee,
    ePlayerStreamSnd_BungeeAttachComment = ePlayerStreamSnd_BeginBungee,
    ePlayerStreamSnd_BungeeBeginDive,
    ePlayerStreamSnd_BungeeDive1 = ePlayerStreamSnd_BungeeBeginDive,
    ePlayerStreamSnd_BungeeDive2,
    ePlayerStreamSnd_BungeeEndDive = ePlayerStreamSnd_BungeeDive2,
    ePlayerStreamSnd_BungeeBeginDeath,
    ePlayerStreamSnd_BungeeDeath1 = ePlayerStreamSnd_BungeeBeginDeath,
    ePlayerStreamSnd_BungeeDeath2,
    ePlayerStreamSnd_BungeeDeath3,
    ePlayerStreamSnd_BungeeDeath4,
    ePlayerStreamSnd_BungeeDeath5,
    ePlayerStreamSnd_BungeeDeath6,
    ePlayerStreamSnd_BungeeDeath7,
    ePlayerStreamSnd_BungeeDeath8,
    ePlayerStreamSnd_BungeeDeath9,
    ePlayerStreamSnd_BungeeDeath10,
    ePlayerStreamSnd_BungeeDeath11,
    ePlayerStreamSnd_BungeeDeath12,
    ePlayerStreamSnd_BungeeDeath13,
    ePlayerStreamSnd_BungeeDeath14,
    ePlayerStreamSnd_BungeeDeath15,
    ePlayerStreamSnd_BungeeDeath16,
    ePlayerStreamSnd_BungeeDeath17,
    ePlayerStreamSnd_BungeeDeath18,
    ePlayerStreamSnd_BungeeEndDeath = ePlayerStreamSnd_BungeeDeath18,
    ePlayerStreamSnd_EndBungee = ePlayerStreamSnd_BungeeEndDeath,
    ePlayerStreamSnd_Total
} ePlayerStreamSnd;

typedef enum _CurrentPlayer
{
    eCurrentPlayerSpongeBob,
    eCurrentPlayerPatrick,
    eCurrentPlayerSandy,
    eCurrentPlayerCount
} zCurrentPlayer;

extern xEntBoulder* boulderVehicle;
extern zCurrentPlayer gCurrentPlayer;

void zEntPlayerControlOn(zControlOwner owner);
void zEntPlayerControlOff(zControlOwner owner);
S32 zEntPlayer_IsSneaking();
S32 zEntPlayer_DamageNPCKnockBack(xBase* src, U32 damage, xVec3* npcPos);
S32 zEntPlayer_Damage(xBase* src, U32 damage);
U32 zEntPlayer_MoveInfo();
void zEntPlayer_StoreCheckPoint(xVec3* pos, F32 rot, U32 initCamID);
void zEntPlayer_SNDPlayStreamRandom(U32 lower, U32 upper, ePlayerStreamSnd player_snd_start, ePlayerStreamSnd player_snd_end, F32 delay);
void zEntPlayer_SNDPlayStreamRandom(ePlayerStreamSnd player_snd_start, ePlayerStreamSnd player_snd_end, F32 delay);

xAnimTable* zSandy_AnimTable();
xAnimTable* zPatrick_AnimTable();
xAnimTable* zEntPlayer_AnimTable();
xAnimTable* zSpongeBobTongue_AnimTable();
xAnimTable* zEntPlayer_BoulderVehicleAnimTable();
xAnimTable* zEntPlayer_TreeDomeSBAnimTable();
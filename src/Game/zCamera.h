#pragma once

#include "xCamera.h"

#define ZCAM_DEFAULT_FOV 75.0f

enum camera_owner_enum
{
    CO_BOULDER       = (1<<0),
    CO_CRUISE_BUBBLE = (1<<1),
    CO_BUNGEE        = (1<<2),
    CO_BOSS          = (1<<3),
    CO_OOB           = (1<<4),
    CO_ZIPLINE       = (1<<5),
    CO_TURRET        = (1<<6),
    CO_REWARDANIM    = (1<<7)
};

struct xCamAsset;

extern F32 zcam_pad_pyaw_scale;
extern F32 zcam_pad_pitch_scale;
extern F32 zcam_near_d;
extern F32 zcam_near_h;
extern F32 zcam_near_pitch;
extern F32 zcam_far_d;
extern F32 zcam_far_h;
extern F32 zcam_far_pitch;
extern F32 zcam_wall_d;
extern F32 zcam_wall_h;
extern F32 zcam_wall_pitch;
extern F32 zcam_above_d;
extern F32 zcam_above_h;
extern F32 zcam_above_pitch;
extern F32 zcam_below_d;
extern F32 zcam_below_h;
extern F32 zcam_below_pitch;
extern F32 zcam_highbounce_d;
extern F32 zcam_highbounce_h;
extern F32 zcam_highbounce_pitch;
extern F32 zcam_overrot_min;
extern F32 zcam_overrot_mid;
extern F32 zcam_overrot_max;
extern F32 zcam_overrot_rate;
extern F32 zcam_overrot_tstart;
extern F32 zcam_overrot_tend;
extern F32 zcam_overrot_velmin;
extern F32 zcam_overrot_velmax;
extern F32 zcam_overrot_tmanual;
extern F32 zcam_overrot_tmr;
extern xCamera zcam_backupcam;
extern xCamera zcam_backupconvers;
extern S32 zcam_near;
extern S32 zcam_mode;
extern S32 zcam_bbounce;
extern S32 zcam_lbbounce;
extern S32 zcam_convers;
extern S32 zcam_lconvers;
extern S32 zcam_longbounce;
extern S32 zcam_highbounce;
extern S32 zcam_cutscene;
extern S32 zcam_reward;
extern xVec3* zcam_playervel;
extern F32 zcam_mintgtheight;
extern S32 zcam_fly;
extern S32 zcam_flypaused;
extern void* zcam_flydata;
extern U32 zcam_flysize;
extern F32 zcam_flytime;
extern U32 zcam_flyasset_current;
extern xCamAsset* zcam_dest;
extern xQuat zcam_quat;
extern F32 zcam_tmr;
extern F32 zcam_ttm;
extern F32 zcam_fovcurr;
extern F32 zcam_fovdest;

void zCameraReset(xCamera* cam);
void zCameraFlyStart(U32 assetID);
void zCameraUpdate(xCamera* cam, F32 dt);
void zCameraSetBbounce(S32 bbouncing);
void zCameraSetLongbounce(S32 lbounce);
void zCameraSetHighbounce(S32 hbounce);
void zCameraSetPlayerVel(xVec3* vel);
void zCameraDisableTracking(camera_owner_enum owner);
void zCameraEnableTracking(camera_owner_enum owner);
U32 zCameraIsTrackingDisabled();
void zCameraDisableInput();
void zCameraEnableInput();
void zCameraDisableLassoCam();
void zCameraEnableLassoCam();
void zCameraSetLassoCamFactor(F32 factor);
F32 zCameraGetLassoCamFactor();
S32 zCameraGetConvers();
void zCameraSetConvers(S32 on);
void zCameraDoTrans(xCamAsset* asset, F32 ttime);
void zCameraTranslate(xCamera* cam, F32 dposx, F32 dposy, F32 dposz);
void zCameraEnableWallJump(xCamera* cam, const xVec3& collNormal);
void zCameraDisableWallJump(xCamera* cam);
void zCameraSetReward(S32 on);
void zCameraMinTargetHeightSet(F32 height);
void zCameraMinTargetHeightClear();
U32 zCamera_FlyOnly();
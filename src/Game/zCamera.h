#pragma once

#include "xCamera.h"

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
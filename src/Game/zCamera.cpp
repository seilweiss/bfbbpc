#include "zCamera.h"

F32 zcam_pad_pyaw_scale = 0.18124573f;
F32 zcam_pad_pitch_scale = 0.01923077f;
F32 zcam_near_d = 3.0f;
F32 zcam_near_h = 1.8f;
F32 zcam_near_pitch = xdeg2rad(10.0f);
F32 zcam_far_d = 5.0f;
F32 zcam_far_h = 3.0f;
F32 zcam_far_pitch = xdeg2rad(15.0f);
F32 zcam_wall_d = 7.5f;
F32 zcam_wall_h = 2.0f;
F32 zcam_wall_pitch = xdeg2rad(18.0f);
F32 zcam_above_d = 0.2f;
F32 zcam_above_h = 2.2f;
F32 zcam_above_pitch = xdeg2rad(70.0f);
F32 zcam_below_d = 0.6f;
F32 zcam_below_h = 0.2f;
F32 zcam_below_pitch = xdeg2rad(-70.0f);
F32 zcam_highbounce_d = 0.2f;
F32 zcam_highbounce_h = 5.0f;
F32 zcam_highbounce_pitch = xdeg2rad(89.0f);
F32 zcam_overrot_min = xdeg2rad(25.0f);
F32 zcam_overrot_mid = xdeg2rad(90.0f);
F32 zcam_overrot_max = xdeg2rad(170.0f);
F32 zcam_overrot_rate = 0.1f;
F32 zcam_overrot_tstart = 1.5f;
F32 zcam_overrot_tend = 2.5f;
F32 zcam_overrot_velmin = 3.0f;
F32 zcam_overrot_velmax = 5.0f;
F32 zcam_overrot_tmanual = 1.5f;
F32 zcam_overrot_tmr = 0.0f;
xCamera zcam_backupcam;
xCamera zcam_backupconvers;
S32 zcam_near;
S32 zcam_mode;
S32 zcam_bbounce;
S32 zcam_lbbounce;
S32 zcam_convers;
S32 zcam_lconvers;
S32 zcam_longbounce;
S32 zcam_highbounce;
S32 zcam_cutscene;
S32 zcam_reward;
xVec3* zcam_playervel;
F32 zcam_mintgtheight = -HUGE;
S32 zcam_fly;
S32 zcam_flypaused;
void* zcam_flydata;
U32 zcam_flysize;
F32 zcam_flytime;
U32 zcam_flyasset_current;
xCamAsset* zcam_dest;
xQuat zcam_quat;
F32 zcam_tmr;
F32 zcam_ttm;
F32 zcam_fovcurr;
F32 zcam_fovdest;

void zCameraReset(xCamera* cam) WIP
{
}
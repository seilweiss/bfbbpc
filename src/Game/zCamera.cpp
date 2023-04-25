#include "zCamera.h"

#include "xutil.h"
#include "xstransvc.h"
#include "xScrFx.h"
#include "xCamAsset.h"
#include "zCameraTweak.h"
#include "zCameraFly.h"
#include "zMusic.h"
#include "zGlobals.h"
#include "zCutsceneMgr.h"

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

namespace {
enum WallJumpViewState
{
    WJVS_DISABLED,
    WJVS_DISABLING,
    WJVS_ENABLED,
    WJVS_ENABLING
};

U32 stop_track;
bool input_enabled = true;
bool lassocam_enabled;
F32 lassocam_factor;
WallJumpViewState wall_jump_enabled;
xVec3 wall_jump_view;
F32 dMultiplier;
F32 dOffset;
F32 hMultiplier;
F32 hOffset;

inline F32 _GetCurrentD()
{
    if (zcam_highbounce) return zcam_highbounce_d;
    if (wall_jump_enabled == WJVS_ENABLED) return zcam_wall_d;
    return zCameraTweakGlobal_GetD();
}

inline F32 GetCurrentD()
{
    return dOffset + dMultiplier * _GetCurrentD();
}

inline F32 _GetCurrentH()
{
    if (zcam_highbounce) return zcam_highbounce_h;
    if (wall_jump_enabled == WJVS_ENABLED) return zcam_wall_h;
    return zCameraTweakGlobal_GetH();
}

inline F32 GetCurrentH()
{
    return dOffset + dMultiplier * _GetCurrentH();
}

inline F32 GetCurrentPitch()
{
    if (zcam_highbounce) return zcam_highbounce_pitch;
    return zCameraTweakGlobal_GetPitch();
}
}

void zCameraReset(xCamera* cam) NONMATCH("https://decomp.me/scratch/ufOt7")
{
    zcam_mode = 0;
    zcam_bbounce = 0;
    zcam_lbbounce = 0;
    zcam_lconvers = 0;
    zcam_longbounce = 0;
    zcam_highbounce = 0;
    zcam_convers = 0;
    zcam_fly = 0;
    zcam_flypaused = 0;
    zcam_cutscene = 0;
    zcam_reward = 0;
    zcam_fovcurr = ZCAM_DEFAULT_FOV;
    zcam_overrot_tmr = 0.0f;
    wall_jump_enabled = WJVS_DISABLED;
    lassocam_enabled = false;
    stop_track = 0;
    zcam_mintgtheight = -HUGE;

    xCameraSetFOV(cam, ZCAM_DEFAULT_FOV);
    zCameraTweakGlobal_Update(0.0f);
    xCameraReset(cam, GetCurrentD(), GetCurrentH(), GetCurrentPitch());

    input_enabled = true;
    dMultiplier = 1.0f;
    dOffset = 0.0f;
    hMultiplier = 1.0f;
    hOffset = 0.0f;
}

static F32 EaseInOut(F32 x)
{
    return x * (0.5f + x * (2.0f - 1.5f * x));
}

static void zCameraConversUpdate(xCamera* cam, F32 dt)
{
    if (!zcam_dest) return;

    if (zcam_tmr <= 0.0f) {
        zcam_tmr = 0.0f;
        return;
    }

    if (dt / zcam_tmr > 1.0f) {
        cam->mat.right = zcam_dest->right;
        cam->mat.up = zcam_dest->up;
        cam->mat.at = zcam_dest->at;
        cam->mat.pos = zcam_dest->pos;
        zcam_fovcurr = zcam_fovdest;
    } else {
        F32 s = EaseInOut(1.0f - zcam_tmr / zcam_ttm);
        s = (EaseInOut(1.0f - (zcam_tmr - dt) / zcam_ttm) - s) / (1.0f - s);

        xQuat a;
        xQuatFromMat(&a, &cam->mat);

        xQuat c;
        xQuatSlerp(&c, &a, &zcam_quat, s);
        xQuatToMat(&c, &cam->mat);

        xVec3Lerp(&cam->mat.pos, &cam->mat.pos, &zcam_dest->pos, s);

        zcam_fovcurr = zcam_fovcurr * (1.0f - s) + zcam_fovdest * s;
    }

    zcam_tmr -= dt;
}

static F32 TranSpeed(zFlyKey* keys)
{
    return ZFLY_FPS * xVec3Dist((xVec3*)&keys[0].matrix[9], (xVec3*)&keys[1].matrix[9]);
}

static F32 MatrixSpeed(zFlyKey* keys)
{
    F32 dot;
    dot = xabs(xVec3Dot((xVec3*)&keys[0].matrix[0], (xVec3*)&keys[1].matrix[0]));
    dot = xmax(xabs(xVec3Dot((xVec3*)&keys[0].matrix[3], (xVec3*)&keys[1].matrix[3])), dot);
    dot = xmax(xabs(xVec3Dot((xVec3*)&keys[0].matrix[6], (xVec3*)&keys[1].matrix[6])), dot);
    dot = xmin(1.0f, dot);
    return xrad2deg(2.0f) * xacos(dot) * ZFLY_FPS;
}

static S32 zCameraFlyUpdate(xCamera* cam, F32 dt) NONMATCH("https://decomp.me/scratch/c1VOt")
{
    S32 i, flyIdx, numKeys;
    F32 flyLerp, flyFrame;
    zFlyKey keys[4];
    
    if (globals.pad0->pressed & (k_XPAD_A | k_XPAD_C)) {
        if (zcam_flytime > gSkipTimeFlythrough) {
            zcam_flytime = (1/ZFLY_FPS) * zcam_flysize;
        }
    }

    flyFrame = ZFLY_FPS * zcam_flytime;
    flyIdx = (S32)std::floorf(flyFrame);
    flyLerp = flyFrame - std::floorf(flyFrame);
    numKeys = zcam_flysize / sizeof(zFlyKey);

    if (flyIdx >= numKeys - 1) return 0;

    keys[0] = ((zFlyKey*)zcam_flydata)[flyIdx - 1 >= 0 ? flyIdx - 1 : flyIdx];
    keys[1] = ((zFlyKey*)zcam_flydata)[flyIdx];
    keys[2] = ((zFlyKey*)zcam_flydata)[flyIdx+1];
    keys[3] = ((zFlyKey*)zcam_flydata)[flyIdx + 2 < numKeys ? flyIdx + 2 : flyIdx + 1];

#ifndef LITTLE_ENDIAN
    for (S32 i = 0; i < sizeof(keys); i += 4) {
        U8 c0 = ((U8*)keys)[i+0];
        U8 c1 = ((U8*)keys)[i+1];
        U8 c2 = ((U8*)keys)[i+2];
        U8 c3 = ((U8*)keys)[i+3];
        ((U8*)keys)[i+0] = c3;
        ((U8*)keys)[i+1] = c2;
        ((U8*)keys)[i+2] = c1;
        ((U8*)keys)[i+3] = c0;
    }
#endif

    if (flyIdx > 0) {
        F32 matdiff1, matdiff2, matdiff3;
        matdiff1 = TranSpeed(&keys[0]);
        matdiff2 = TranSpeed(&keys[1]);
        matdiff3 = TranSpeed(&keys[2]);
        if (matdiff2 > 10.0f && matdiff2 > 5.0f * matdiff1 && matdiff2 > 5.0f * matdiff3) {
            flyLerp = 0.0f;
        } else {
            matdiff1 = MatrixSpeed(&keys[0]);
            matdiff2 = MatrixSpeed(&keys[1]);
            matdiff3 = MatrixSpeed(&keys[2]);
            if (matdiff2 > 45.0f && matdiff2 > 5.0f * matdiff1 && matdiff2 > 5.0f * matdiff3) {
                flyLerp = 0.0f;
            }
        }
    }

    xMat3x3 tmpMat;
    xQuat quats[2];
    for (i = 0; i < 2; i++) {
        
        tmpMat.right.x = -keys[i+1].matrix[0];
        tmpMat.right.y = -keys[i+1].matrix[1];
        tmpMat.right.z = -keys[i+1].matrix[2];
        tmpMat.up.x = keys[i+1].matrix[3];
        tmpMat.up.y = keys[i+1].matrix[4];
        tmpMat.up.z = keys[i+1].matrix[5];
        tmpMat.at.x = -keys[i+1].matrix[6];
        tmpMat.at.y = -keys[i+1].matrix[7];
        tmpMat.at.z = -keys[i+1].matrix[8];
        xQuatFromMat(&quats[i], &tmpMat);
    }

    xQuat qresult;
    xQuatSlerp(&qresult, &quats[0], &quats[1], flyLerp);
    xQuatToMat(&qresult, &cam->mat);
    xVec3Lerp(&cam->mat.pos, (xVec3*)&keys[1].matrix[9], (xVec3*)&keys[2].matrix[9], flyLerp);

    zcam_flytime += dt;
    
    return 1;
}

void zCameraFlyStart(U32 assetID) NONMATCH("https://decomp.me/scratch/ukvlS")
{
    st_PKR_ASSET_TOCINFO ainfo;
    if (xSTGetAssetInfo(assetID, &ainfo)) {
        zcam_fly = 1;
        zcam_flypaused = 0;
        zcam_flydata = ainfo.mempos;
        zcam_flysize = ainfo.size;
        zcam_flytime = 1/ZFLY_FPS;
        zcam_flyasset_current = assetID;

        zEntPlayerControlOff(CONTROL_OWNER_FLY_CAM);
        xScrFxLetterbox(1);

        zcam_backupcam = globals.camera;

        if (!zCamera_FlyOnly()) {
            zMusicSetVolume(0.5f, 0.1f);
        }
    }
}

static void zCameraFlyRestoreBackup(xCamera* backup)
{
    globals.camera.mat = backup->mat;
    globals.camera.omat = backup->omat;
    globals.camera.mbasis = backup->mbasis;
    globals.camera.bound = backup->bound;
    globals.camera.focus = backup->focus;
    globals.camera.flags = backup->flags;
    globals.camera.tmr = backup->tmr;
    globals.camera.tm_acc = backup->tm_acc;
    globals.camera.tm_dec = backup->tm_dec;
    globals.camera.ltmr = backup->ltmr;
    globals.camera.ltm_acc = backup->ltm_acc;
    globals.camera.ltm_dec = backup->ltm_dec;
    globals.camera.dmin = backup->dmin;
    globals.camera.dmax = backup->dmax;
    globals.camera.dcur = backup->dcur;
    globals.camera.dgoal = backup->dgoal;
    globals.camera.hmin = backup->hmin;
    globals.camera.hmax = backup->hmax;
    globals.camera.hcur = backup->hcur;
    globals.camera.hgoal = backup->hgoal;
    globals.camera.pmin = backup->pmin;
    globals.camera.pmax = backup->pmax;
    globals.camera.pcur = backup->pcur;
    globals.camera.pgoal = backup->pgoal;
    globals.camera.depv = backup->depv;
    globals.camera.hepv = backup->hepv;
    globals.camera.pepv = backup->pepv;
    globals.camera.orn_epv = backup->orn_epv;
    globals.camera.yaw_epv = backup->yaw_epv;
    globals.camera.pitch_epv = backup->pitch_epv;
    globals.camera.roll_epv = backup->roll_epv;
    globals.camera.orn_cur = backup->orn_cur;
    globals.camera.orn_goal = backup->orn_goal;
    globals.camera.orn_diff = backup->orn_diff;
    globals.camera.yaw_cur = backup->yaw_cur;
    globals.camera.yaw_goal = backup->yaw_goal;
    globals.camera.pitch_cur = backup->pitch_cur;
    globals.camera.pitch_goal = backup->pitch_goal;
    globals.camera.roll_cur = backup->roll_cur;
    globals.camera.roll_goal = backup->roll_goal;
    globals.camera.dct = backup->dct;
    globals.camera.dcd = backup->dcd;
    globals.camera.dccv = backup->dccv;
    globals.camera.dcsv = backup->dcsv;
    globals.camera.hct = backup->hct;
    globals.camera.hcd = backup->hcd;
    globals.camera.hccv = backup->hccv;
    globals.camera.hcsv = backup->hcsv;
    globals.camera.pct = backup->pct;
    globals.camera.pcd = backup->pcd;
    globals.camera.pccv = backup->pccv;
    globals.camera.pcsv = backup->pcsv;
    globals.camera.orn_ct = backup->orn_ct;
    globals.camera.orn_cd = backup->orn_cd;
    globals.camera.orn_ccv = backup->orn_ccv;
    globals.camera.orn_csv = backup->orn_csv;
    globals.camera.yaw_ct = backup->yaw_ct;
    globals.camera.yaw_cd = backup->yaw_cd;
    globals.camera.yaw_ccv = backup->yaw_ccv;
    globals.camera.yaw_csv = backup->yaw_csv;
    globals.camera.pitch_ct = backup->pitch_ct;
    globals.camera.pitch_cd = backup->pitch_cd;
    globals.camera.pitch_ccv = backup->pitch_ccv;
    globals.camera.pitch_csv = backup->pitch_csv;
    globals.camera.roll_ct = backup->roll_ct;
    globals.camera.roll_cd = backup->roll_cd;
    globals.camera.roll_ccv = backup->roll_ccv;
    globals.camera.roll_csv = backup->roll_csv;
}

static F32 rewardMove;
static F32 rewardMoveSpeed;
static F32 rewardZoomSpeed;
static F32 rewardZoomAmount;
static F32 rewardTiltTime;
static F32 rewardTiltAmount;

static S32 zCameraRewardUpdate(xCamera* cam, F32 dt) NONMATCH("https://decomp.me/scratch/6DdQf")
{
    xCameraUpdate(cam, dt);
    
    xVec3 v = {};
    xVec3Copy(&v, (xVec3*)&globals.player.ent.model->Mat->pos);
    v.y -= 0.7f;

    if (zcam_near) {
        if (globals.player.s->pcType == ePlayer_SB) {
            rewardMove = 1.3f;
            rewardMoveSpeed = 0.68f;
            rewardZoomSpeed = 7.1f;
            rewardZoomAmount = 108.0f;
            rewardTiltTime = 1.5f;
            rewardTiltAmount = -0.22f;
        } else if (globals.player.s->pcType == ePlayer_Patrick) {
            rewardMove = 1.6f;
            rewardMoveSpeed = 0.68f;
            rewardZoomSpeed = 7.1f;
            rewardZoomAmount = 108.0f;
            rewardTiltTime = 1.0f;
            rewardTiltAmount = -0.25f;
        } else if (globals.player.s->pcType == ePlayer_Sandy) {
            // Same as SB
            rewardMove = 1.3f;
            rewardMoveSpeed = 0.68f;
            rewardZoomSpeed = 7.1f;
            rewardZoomAmount = 108.0f;
            rewardTiltTime = 1.5f;
            rewardTiltAmount = -0.22f;
        }
    } else {
        // Same for all 3
        if (globals.player.s->pcType == ePlayer_SB) {
            rewardMove = 1.5f;
            rewardMoveSpeed = 1.1f;
            rewardZoomSpeed = 5.9f;
            rewardZoomAmount = 100.0f;
            rewardTiltTime = 1.5f;
            rewardTiltAmount = -0.2f;
        } else if (globals.player.s->pcType == ePlayer_Patrick) {
            rewardMove = 1.5f;
            rewardMoveSpeed = 1.1f;
            rewardZoomSpeed = 5.9f;
            rewardZoomAmount = 100.0f;
            rewardTiltTime = 1.5f;
            rewardTiltAmount = -0.2f;
        } else if (globals.player.s->pcType == ePlayer_Sandy) {
            rewardMove = 1.5f;
            rewardMoveSpeed = 1.1f;
            rewardZoomSpeed = 5.9f;
            rewardZoomAmount = 100.0f;
            rewardTiltTime = 1.5f;
            rewardTiltAmount = -0.2f;
        }
    }

    if (xVec3Dist2((xVec3*)&globals.player.ent.model->Mat->pos, &globals.camera.mat.pos) > xsqr(rewardMove)) {
        xCameraMove(cam, v, rewardMoveSpeed * dt);
        xCameraFOV(cam, rewardZoomAmount, rewardZoomSpeed, dt);
    }

    xCameraLookYPR(cam, 0, globals.camera.yaw_cur, rewardTiltAmount, globals.camera.roll_cur, rewardTiltTime, 0.1f, 0.1f);

    return 1;
}

static void zCameraFreeLookSetGoals(xCamera* cam, F32 pitch_s, F32& dgoal, F32& hgoal, F32& pitch_goal, F32& lktm, F32 dt) NONMATCH("https://decomp.me/scratch/Dy6xy")
{
    if (zcam_bbounce) {
        if (zcam_highbounce) {
            dgoal = GetCurrentD();
            hgoal = GetCurrentH();
            pitch_goal = GetCurrentPitch();
        } else {
            dgoal = zcam_near ? 3.5f : GetCurrentD();
            hgoal = zcam_near ? 2.4f : GetCurrentH();
            if (zcam_longbounce) {
                F32 f1 = xsqrt(xsqr(zcam_playervel->x) + xsqr(zcam_playervel->y) + xsqr(zcam_playervel->z));
                F32 s = (zcam_playervel && f1)
                    ? -xmin(0.0f,
                        (cam->mat.at.x * zcam_playervel->x +
                        cam->mat.at.y * zcam_playervel->y +
                        cam->mat.at.z * zcam_playervel->z) / f1)
                    : 0.0f;
                pitch_goal = zcam_near ? xdeg2rad(20.0f + 20.0f * s) : xdeg2rad(30.0f);
            } else {
                pitch_goal = zcam_near ? xdeg2rad(40.0f) : xdeg2rad(30.0f);
            }
        }
    } else {
        F32 d = GetCurrentD();
        F32 h = GetCurrentH();
        F32 pitch = GetCurrentPitch();
        if (lassocam_enabled && !stop_track) {
            dgoal = zcam_near_d + (d - zcam_near_d) * lassocam_factor;
            hgoal = zcam_near_h + (h - zcam_near_h) * lassocam_factor;
            pitch_goal = zcam_near_pitch + lassocam_factor * (pitch - zcam_near_pitch);
        } else {
            if (pitch_s > 0.0f) {
                dgoal = d + (zcam_below_d - d) * pitch_s;
                hgoal = h + (zcam_below_h - h) * pitch_s;
                pitch_goal = pitch + (zcam_below_pitch - pitch) * pitch_s * pitch_s * pitch_s;
            } else {
                dgoal = d + (zcam_above_d - d) * -pitch_s;
                hgoal = h + (zcam_above_h - h) * -pitch_s;
                pitch_goal = pitch + (zcam_above_pitch - pitch) * -pitch_s;
            }
            if (lktm > 0.1f) {
                lktm -= dt;
                if (lktm < 0.1f) lktm = 0.1f;
            } else lktm = 0.1f;
        }
    }
}

void zCameraUpdate(xCamera* cam, F32 dt) NONMATCH("https://decomp.me/scratch/c4Qf9")
{
    if (globals.sceneCur->sceneID == 'HB02' ||
        globals.sceneCur->sceneID == 'HB03' ||
        globals.sceneCur->sceneID == 'HB04' ||
        globals.sceneCur->sceneID == 'HB06' ||
        globals.sceneCur->sceneID == 'HB07' ||
        globals.sceneCur->sceneID == 'HB08' ||
        globals.sceneCur->sceneID == 'HB09' ||
        globals.sceneCur->sceneID == 'HB10') {
        zcam_near |= 2;
    } else {
        zcam_near &= 1;
    }

    zCameraTweakGlobal_Update(dt);

    xVec3 tran_accum = cam->tran_accum;
    cam->tran_accum.x = 0.0f;
    cam->tran_accum.y = 0.0f;
    cam->tran_accum.z = 0.0f;

    if (globals.cmgr && globals.cmgr->csn->Ready) {
        if (zcam_fly && zcam_flypaused) {
            zcam_fly = 0;
            zcam_flypaused = 0;
            zEntPlayerControlOn(CONTROL_OWNER_FLY_CAM);
            xScrFxLetterbox(0);
            zCameraFlyRestoreBackup(&zcam_backupcam);
            zcam_flyasset_current = 0;
            xCameraSetFOV(cam, ZCAM_DEFAULT_FOV);
        }
        if (!zcam_cutscene) {
            zcam_backupcam = globals.camera;
            zcam_cutscene = 1;
        }
        xCutscene_SetCamera(globals.cmgr->csn, cam);
        iCameraUpdatePos(cam->lo_cam, &cam->mat);
        return;
    }

    if (zcam_cutscene) {
        zCameraFlyRestoreBackup(&zcam_backupcam);
        xCameraSetFOV(cam, ZCAM_DEFAULT_FOV);
        zcam_cutscene = 0;
    }

    if (zcam_convers) {
        zCameraConversUpdate(cam, dt);
        iCameraUpdatePos(cam->lo_cam, &cam->mat);
        xCameraSetFOV(cam, zcam_fovcurr);
        zcam_lconvers = zcam_convers;
        return;
    }

    if (zcam_reward) {
        F32 minDist = zcam_near ? 12.14f : 33.9f;
        F32 dist = xsqr(cam->dcur) + xsqr(cam->hcur);
        if (stop_track & CO_REWARDANIM) {
            zCameraRewardUpdate(cam, dt);
            return;
        }
        if (dist > minDist) {
            zCameraDisableTracking(CO_REWARDANIM);
        }
    }

    if (zcam_fly) {
        if (zCameraFlyUpdate(cam, dt)) {
            iCameraUpdatePos(cam->lo_cam, &cam->mat);
            return;
        }
        if (!zcam_flypaused) {
            zCameraFlyProcessStopEvent();
        }
        if (globals.cmgr) {
            zcam_flypaused = 1;
            return;
        }
        zcam_fly = 0;
        zEntPlayerControlOn(CONTROL_OWNER_FLY_CAM);
        xScrFxLetterbox(0);
        zCameraFlyRestoreBackup(&zcam_backupcam);
        zcam_flyasset_current = 0;
        xCameraSetFOV(cam, ZCAM_DEFAULT_FOV);
    }

    if (stop_track) {
        if (stop_track & CO_OOB) {
            xCameraUpdate(cam, dt);
        } else {
            iCameraUpdatePos(cam->lo_cam, &cam->mat);
        }
        return;
    }

    F32 tgtHeight = cam->tgt_mat->pos.y;
    F32 oldTgtHeight = cam->tgt_omat->pos.y;
    if (tgtHeight < zcam_mintgtheight) {
        cam->tgt_omat->pos.y = oldTgtHeight + (zcam_mintgtheight - tgtHeight);
        cam->tgt_mat->pos.y += zcam_mintgtheight - cam->tgt_mat->pos.y;
    }

    F32 plerp = xrad2deg(0.1f) * (xabs(cam->pitch_cur) - xdeg2rad(70.0f));
    plerp = xclamp(plerp, 0.0f, 1.0f);

    F32 dlerp = 1.6666666f * (1.2f -
        xsqrt(xsqr(cam->mat.pos.x - cam->tgt_mat->pos.x) +
              xsqr(cam->mat.pos.z - cam->tgt_mat->pos.z)));
    dlerp = xclamp(dlerp, 0.0f, 1.0f);

    F32 vertical_lerp = xmax(dlerp, plerp);
    if (vertical_lerp) {
        xVec3 delta;
        delta.x = vertical_lerp * (cam->tgt_mat->pos.x - cam->tgt_omat->pos.x - tran_accum.x);
        delta.y = vertical_lerp * (cam->tgt_mat->pos.y - cam->tgt_omat->pos.y - tran_accum.y);
        delta.z = vertical_lerp * (cam->tgt_mat->pos.z - cam->tgt_omat->pos.z - tran_accum.z);
        cam->mat.pos.x += delta.x;
        cam->mat.pos.y += delta.y;
        cam->mat.pos.z += delta.z;
        cam->omat.pos = cam->mat.pos;
    }

    static F32 mvtm = 0.1f;
    static F32 lktm = 0.1f;
    static F32 pitch_s = 0.0f;

    if (wall_jump_enabled == WJVS_ENABLING ||
        wall_jump_enabled == WJVS_DISABLING) {
        mvtm = 0.3f;
        lktm = 0.3f;
        if (wall_jump_enabled == WJVS_ENABLING) {
            wall_jump_enabled = WJVS_ENABLED;
        }
    }

    U32 button = k_XPAD_E;
    if (globals.pad0->pressed & button) {
        if (input_enabled && !(zcam_near & 2)) {
            zcam_near ^= 1;
            mvtm = 0.3f;
            lktm = 0.3f;
            pitch_s = 0.0f;
        }
    } else if (zcam_bbounce != zcam_lbbounce) {
        mvtm = 0.3f;
        lktm = 0.3f;
        pitch_s = 0.0f;
    } else if (mvtm > 0.1f) {
        mvtm -= dt;
        if (mvtm < 0.1f) mvtm = 0.1f;
    }

    F32 dgoal = GetCurrentD();
    F32 hgoal = GetCurrentH();
    F32 pgoal = cam->pcur;
    F32 pitch_goal = zcam_near ? zcam_near_pitch : zcam_far_pitch;

    if (lassocam_enabled && !stop_track) {
        dgoal = zcam_near_d + lassocam_factor * (dgoal - zcam_near_d);
        hgoal = zcam_near_h + lassocam_factor * (hgoal - zcam_near_h);
    }

    if (input_enabled && wall_jump_enabled == WJVS_DISABLED) {
        if (globals.pad0->analog2.x > 32) {
            S32 x = xclamp(globals.pad0->analog2.x, 32, 110) - 32;
            F32 dp = (F32)x * zcam_pad_pyaw_scale * 0.01666666f;
            if (lassocam_enabled && !stop_track) {
                dp *= 0.2f;
            }
            pgoal += dp;
            cam->pcur += dp;
            cam->pgoal += dp;
            if (lktm > 0.025f) {
                lktm -= dt;
                if (lktm < 0.025f) lktm = 0.025f;
            }
            zcam_overrot_tmr = -zcam_overrot_tmanual;
        } else if (globals.pad0->analog2.x < -32) {
            S32 x = xclamp(globals.pad0->analog2.x, -110, -32) + 32;
            F32 dp = (F32)x * zcam_pad_pyaw_scale * 0.01666666f;
            if (lassocam_enabled && !stop_track) {
                dp *= 0.2f;
            }
            pgoal += dp;
            cam->pcur += dp;
            cam->pgoal += dp;
            if (lktm > 0.025f) {
                lktm -= dt;
                if (lktm < 0.025f) lktm = 0.025f;
            }
            zcam_overrot_tmr = -zcam_overrot_tmanual;
        }
    }

    pitch_s = 0.0f;
    if (input_enabled && wall_jump_enabled == WJVS_DISABLED && !zcam_highbounce) {
        if (globals.pad0->analog2.y > 32) {
            S32 y = xclamp(globals.pad0->analog2.y, 32, 110) - 32;
            pitch_s = (F32)y * zcam_pad_pitch_scale * 0.01666666f;
            if (pitch_s > 1.0f) pitch_s = 1.0f;
            zcam_overrot_tmr = -zcam_overrot_tmanual;
        } else if (globals.pad0->analog2.y < -32) {
            S32 y = xclamp(globals.pad0->analog2.y, -110, -32) + 32;
            pitch_s = (F32)y * zcam_pad_pitch_scale * 0.01666666f;
            if (pitch_s < -1.0f) pitch_s = -1.0f;
            zcam_overrot_tmr = -zcam_overrot_tmanual;
        }
    }

    if (dt > 0.00001f &&
        cam->tgt_mat &&
        cam->tgt_omat &&
        vertical_lerp < 0.9999f) {
        F32 velx = (cam->tgt_mat->pos.x - cam->tgt_omat->pos.x - tran_accum.x) / dt;
        F32 velz = (cam->tgt_mat->pos.z - cam->tgt_omat->pos.z - tran_accum.z) / dt;
        F32 camx = cam->tgt_mat->pos.x - cam->mat.pos.x;
        F32 camz = cam->tgt_mat->pos.z - cam->mat.pos.z;
        F32 cammag = xsqrt(xsqr(camx) + xsqr(camz));
        F32 velmag = xsqrt(xsqr(velx) + xsqr(velz));
        if (velmag > zcam_overrot_velmin || zcam_overrot_tmr < 0.0f) {
            zcam_overrot_tmr += dt;
        } else if (zcam_overrot_tmr > 0.0f) {
            if (zcam_overrot_tmr > zcam_overrot_tend) zcam_overrot_tmr = zcam_overrot_tend;
            zcam_overrot_tmr -= dt;
            if (zcam_overrot_tmr < 0.0f) zcam_overrot_tmr = 0.0f;
        }
        if (zcam_overrot_tmr > zcam_overrot_tstart &&
            cammag > 1.2f &&
            velmag > zcam_overrot_velmin) {
            camz /= cammag;
            velz /= velmag;
            camx /= cammag;
            velx /= velmag;
            F32 f1 = xclamp(camx * velx + camz * velz, -1.0f, 1.0f);
            F32 f22 = camz * velx - camx * velz;
            F32 velang = xacos(f1);
            if (velang > zcam_overrot_min && velang < zcam_overrot_max) {
                F32 angle_factor;
                if (velang <= zcam_overrot_mid) {
                    angle_factor = (velang - zcam_overrot_min) / (zcam_overrot_mid - zcam_overrot_min);
                } else {
                    angle_factor = (zcam_overrot_max - velang) / (zcam_overrot_max - zcam_overrot_mid);
                }
                F32 vel_factor = xclamp((velmag - zcam_overrot_velmin) / (zcam_overrot_velmax - zcam_overrot_velmin), 0.0f, 1.0f);
                F32 time_factor = xclamp((zcam_overrot_tmr - zcam_overrot_tstart) / (zcam_overrot_tend - zcam_overrot_tstart), 0.0f, 1.0f);
                F32 dp = vel_factor * angle_factor * zcam_overrot_rate * time_factor;
                if (lassocam_enabled && !stop_track) {
                    dp *= 0.2f;
                }
                if (f22 > 0.0f) {
                    pgoal += dp;
                    cam->pcur += dp;
                    cam->pgoal += dp;
                } else {
                    pgoal -= dp;
                    cam->pcur -= dp;
                    cam->pgoal -= dp;
                }
            }
        }
    }

    zCameraFreeLookSetGoals(cam, pitch_s, dgoal, hgoal, pitch_goal, lktm, dt);

    F32 dist, dirx, diry, dirz;
    xVec3NormalizeDistMacro2(&dirx, &diry, &dirz, &cam->mat.pos, &cam->tgt_mat->pos, &dist);

    F32 f22 = xatan2(dirx, dirz);

    if (lassocam_enabled && !stop_track) {
        xCameraMove(cam, 0x20, dgoal, hgoal, pgoal, mvtm, 0.0f, 0.0f);
    } else {
        xCameraMove(cam, 0x28, dgoal, hgoal, pgoal, mvtm, 0.0f, 0.0f);
    }

    xCameraLookYPR(cam, 0, f22, pitch_goal, 0.0f, lktm, 0.0f, 0.0f);

    if (wall_jump_enabled == WJVS_ENABLED) {
        xCameraRotate(cam, wall_jump_view, 0.0f, 0.5f, 0.1f, 0.1f);
        cam->dcur = cam->dgoal;
        
        xVec3 destPosition;
        xVec3SMul(&destPosition, &wall_jump_view, -cam->dcur);
        xVec3Add(&destPosition, &destPosition, (xVec3*)&globals.player.ent.model->Mat->pos);
        destPosition.y += hgoal * 0.5f;

        xCameraMove(cam, destPosition, 25.0f * dt);

        cam->flags |= 0x20;
    } else if (wall_jump_enabled == WJVS_DISABLING) {
        cam->dcur = cam->dgoal;
        cam->pcur = cam->pgoal;
        cam->hcur = cam->hgoal;
        wall_jump_enabled = WJVS_DISABLED;
    }

    xCameraUpdate(cam, dt);

    if (lassocam_enabled && !stop_track) {
        cam->pcur = pgoal;
    }

    zcam_lbbounce = zcam_bbounce;
    
    cam->tgt_mat->pos.y = tgtHeight;
    cam->tgt_omat->pos.y = oldTgtHeight;
}

void zCameraSetBbounce(S32 bbouncing)
{
    zcam_bbounce = bbouncing;
}

void zCameraSetLongbounce(S32 lbounce) NONMATCH("https://decomp.me/scratch/sSJai")
{
    if (zcam_highbounce || zcam_longbounce != lbounce) {
        zcam_lbbounce = 0;
    }
    zcam_longbounce = lbounce;
    zcam_highbounce = 0;
}

void zCameraSetHighbounce(S32 hbounce) NONMATCH("https://decomp.me/scratch/eZUYp")
{
    if (zcam_longbounce || zcam_highbounce != hbounce) {
        zcam_lbbounce = 0;
    }
    zcam_highbounce = hbounce;
    zcam_longbounce = 0;
}

void zCameraSetPlayerVel(xVec3* vel)
{
    zcam_playervel = vel;
}

void zCameraDisableTracking(camera_owner_enum owner)
{
    stop_track |= owner;
}

void zCameraEnableTracking(camera_owner_enum owner)
{
    stop_track &= ~owner;
}

U32 zCameraIsTrackingDisabled()
{
    return stop_track;
}

void zCameraDisableInput()
{
    input_enabled = false;
}

void zCameraEnableInput()
{
    input_enabled = true;
}

void zCameraDisableLassoCam()
{
    lassocam_enabled = false;
}

void zCameraEnableLassoCam()
{
    lassocam_enabled = true;
}

void zCameraSetLassoCamFactor(F32 factor)
{
    lassocam_factor = factor;
}

F32 zCameraGetLassoCamFactor()
{
    return lassocam_factor;
}

S32 zCameraGetConvers()
{
    return zcam_convers;
}

void zCameraSetConvers(S32 on) NONMATCH("https://decomp.me/scratch/gHwV3")
{
    zcam_convers = on;
    
    xCamera& cam = globals.camera;
    static bool saved = false;

    if (on) {
        zcam_backupconvers = cam;
        saved = true;
        zcam_dest = NULL;
        zcam_tmr = 0.0f;
    } else {
        xCameraSetFOV(&cam, ZCAM_DEFAULT_FOV);
        zcam_fovcurr = ZCAM_DEFAULT_FOV;
        if (saved) {
            zCameraFlyRestoreBackup(&zcam_backupconvers);
            xCameraMove(&cam, 0x2E, cam.dcur, cam.hcur, cam.pcur, 0.0f, 0.0f, 0.0f);
            saved = false;
        }
    }
}

void zCameraDoTrans(xCamAsset* asset, F32 ttime) NONMATCH("https://decomp.me/scratch/Te0WQ")
{
    zcam_dest = asset;
    
    ttime = (ttime > 0.0f) ? ttime : asset->trans_time;
    
    zcam_tmr = ttime;
    zcam_ttm = ttime;
    
    if (ttime <= 0.0f) {
        globals.camera.mat.right = asset->right;
        globals.camera.mat.up = asset->up;
        globals.camera.mat.at = asset->at;
        globals.camera.mat.pos = asset->pos;
        zcam_fovcurr = asset->fov;
        zcam_fovdest = asset->fov;
    } else {
        xMat3x3 m;
        m.right = asset->right;
        m.up = asset->up;
        m.at = asset->at;
        xQuatFromMat(&zcam_quat, &m);
        zcam_fovdest = asset->fov;
    }
}

void zCameraTranslate(xCamera* cam, F32 dposx, F32 dposy, F32 dposz)
{
    cam->mat.pos.x += dposx;
    cam->mat.pos.y += dposy;
    cam->mat.pos.z += dposz;
    cam->tran_accum.x += dposx;
    cam->tran_accum.y += dposy;
    cam->tran_accum.z += dposz;
}

void zCameraEnableWallJump(xCamera* cam, const xVec3& collNormal)
{
    if (wall_jump_enabled != WJVS_ENABLED) {
        wall_jump_enabled = WJVS_ENABLING;
    }
    
    xVec3 up = { 0.0f, 1.0f, 0.0f };
    xVec3Cross(&wall_jump_view, &collNormal, &up);
    xVec3Normalize(&wall_jump_view, &wall_jump_view);
    if (xVec3Dot(&wall_jump_view, &globals.camera.mat.at) < 0.0f) {
        xVec3Sub(&wall_jump_view, &g_O3, &wall_jump_view);
    }
}

void zCameraDisableWallJump(xCamera* cam)
{
    if (wall_jump_enabled != WJVS_DISABLED) {
        wall_jump_enabled = WJVS_DISABLING;
    }
}

void zCameraSetReward(S32 on)
{
    if (zCameraIsTrackingDisabled()) {
        zcam_reward = 0;
        return;
    }

    zcam_reward = on;
}

void zCameraMinTargetHeightSet(F32 height)
{
    zcam_mintgtheight = height;
}

void zCameraMinTargetHeightClear()
{
    zcam_mintgtheight = -HUGE;
}

U32 zCamera_FlyOnly()
{
    switch (globals.sceneCur->sceneID) {
    case 'DB02':
    case 'KF05':
    case 'PG12':
    case 'SM02':
    case 'SM03':
    case 'SM04':
        return 1;
    }

    return 0;
}
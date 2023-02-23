#include "xCamera.h"

#include "xstransvc.h"
#include "xScrFx.h"
#include "xCollideFast.h"

#include "zGlobals.h"
#include "zGrid.h"

static S32 sCamCollis = 0;

S32 xcam_collis_owner_disable = 0;
S32 xcam_do_collis = 1;
F32 xcam_collis_radius = 0.4f;
F32 xcam_collis_stiffness = 0.3f;

static RpAtomic* sInvisWallHack = NULL;

F32 gCameraLastFov = 0.0f;

static xMat4x3 sCameraFXMatOld;

cameraFXTableEntry sCameraFXTable[eCameraFXCount] =
{
    { eCameraFXNone, NULL, NULL },
    { eCameraFXZoom, xCameraFXZoomUpdate, NULL },
    { eCameraFXShake, xCameraFXShakeUpdate, NULL },
};

cameraFX sCameraFX[CAMERAFX_MAX];

static void xCameraFXInit();

void xCameraInit(xCamera* cam, U32 width, U32 height) NONMATCH("https://decomp.me/scratch/8VXbZ")
{
    xCameraFXInit();

    cam->lo_cam = iCameraCreate(width, height, 1);
    xCameraSetFOV(cam, 75.0f);
    cam->bound.sph.center.x = 0.0f;
    cam->bound.sph.center.y = 0.0f;
    cam->bound.sph.center.z = 0.0f;
    cam->bound.sph.r = 0.5f;
    cam->tgt_mat = NULL;
    cam->tgt_omat = NULL;
    cam->tgt_bound = NULL;
    cam->sc = NULL;
    cam->tran_accum.x = 0.0f;
    cam->tran_accum.y = 0.0f;
    cam->tran_accum.z = 0.0f;

    add_camera_tweaks();
}

void xCameraExit(xCamera* cam)
{
    if (cam->lo_cam) {
        iCameraDestroy(cam->lo_cam);
        cam->lo_cam = NULL;
    }
}

void xCameraReset(xCamera* cam, F32 d, F32 h, F32 pitch) NONMATCH("https://decomp.me/scratch/wE3tq")
{
    sInvisWallHack = (RpAtomic*)xSTFindAsset(0xB8895D14, NULL);

    xMat4x3Identity(&cam->mat);
    cam->omat = cam->mat;
    cam->focus.x = 0.0f;
    cam->focus.y = 0.0f;
    cam->focus.z = 10.0f;
    cam->tran_accum.x = 0.0f;
    cam->tran_accum.y = 0.0f;
    cam->tran_accum.z = 0.0f;
    cam->flags = 0;
    
    F32 goal_p = PI;
    if (cam->tgt_mat) goal_p += xatan2(cam->tgt_mat->at.x, cam->tgt_mat->at.z);

    xCameraMove(cam, 0x2E, d, h, goal_p, 0.0f, 2.0f/3.0f, 2.0f/3.0f);

    cam->pitch_goal = pitch;
    cam->pitch_cur = pitch;
    cam->roll_cur = 0.0f;
    xMat3x3Euler(&cam->mat, cam->yaw_cur, cam->pitch_cur, cam->roll_cur);
    cam->omat = cam->mat;
    cam->yaw_ct = 1.0f;
    cam->yaw_cd = 1.0f;
    cam->yaw_ccv = 0.65f;
    cam->yaw_csv = 1.0f;
    cam->pitch_ct = 1.0f;
    cam->pitch_cd = 1.0f;
    cam->pitch_ccv = 0.7f;
    cam->pitch_csv = 1.0f;
    cam->roll_ct = 1.0f;
    cam->roll_cd = 1.0f;
    cam->roll_ccv = 0.7f;
    cam->roll_csv = 1.0f;
    cam->flags |= 0x80;

    xcam_do_collis = 1;
    xcam_collis_owner_disable = 0;
}

static void xCam_buildbasis(xCamera* cam) NONMATCH("https://decomp.me/scratch/jZUnp")
{
    if (!cam->tgt_mat) return;

    F32 d2d;
    xVec3NormalizeDistXZMacro(&cam->mbasis.at, &cam->tgt_mat->pos, &cam->mat.pos, &d2d);

    if (d2d < EPSILON) {
        cam->mbasis.at.x = cam->mat.at.x;
        cam->mbasis.at.z = cam->mat.at.z;

        F32 f1 = xsqrt(xsqr(cam->mbasis.at.x) + xsqr(cam->mbasis.at.z));
        if (f1 > 0.001f) {
            F32 f1_inv = 1.0f / f1;
            cam->mbasis.at.x *= f1_inv;
            cam->mbasis.at.z *= f1_inv;
        } else {
            cam->mbasis.at.x = isin(cam->pcur);
            cam->mbasis.at.z = icos(cam->pcur);
        }
    }

    cam->mbasis.at.y = 0.0f;
    cam->mbasis.up.x = 0.0f;
    cam->mbasis.up.y = 1.0f;
    cam->mbasis.up.z = 0.0f;
    cam->mbasis.right.x = cam->mbasis.at.z;
    cam->mbasis.right.y = 0.0f;
    cam->mbasis.right.z = -cam->mbasis.at.x;
}

static void xCam_cyltoworld(xVec3* v, const xMat4x3* tgt_mat, F32 d, F32 h, F32 p, U32 flags)
{
    if (flags & 0x10) {
        v->y = h;
    } else {
        v->y = h + tgt_mat->pos.y;
    }

    if (flags & 0x20) {
        v->x = tgt_mat->pos.x + d * isin(p);
        v->z = tgt_mat->pos.z + d * icos(p);
    } else {
        p += xatan2(tgt_mat->at.x, tgt_mat->at.z);
        v->x = tgt_mat->pos.x + d * isin(p);
        v->z = tgt_mat->pos.z + d * icos(p);
    }
}

static void xCam_worldtocyl(F32& d, F32& h, F32& p, const xMat4x3* tgt_mat, const xVec3* v, U32 flags)
{
    xVec3 f1_f2;
    xVec3NormalizeDistXZMacro(&f1_f2, &tgt_mat->pos, v, &d);

    if (flags & 0x10) {
        h = v->y;
    } else {
        h = v->y - tgt_mat->pos.y;
    }

    p = xatan2(f1_f2.x, f1_f2.z);
    if (!(flags & 0x20)) {
        p = xDangleClamp(p - xatan2(tgt_mat->at.x, tgt_mat->at.z));
    }
}

static void xCam_CorrectD(xCamera* cam, F32 f1, F32 f2, F32 f3)
{
    F32 f5 = (2.0f * f1 - f2 * f3) * (10.0f/7.0f);
    F32 f2_0 = f5 - f2;
    f2_0 *= f3;
    F32 f0_0 = cam->mbasis.at.x * f2_0;
    F32 f0_1 = cam->mbasis.at.z * f2_0;
    cam->mat.pos.x += f0_0;
    cam->mat.pos.z += f0_1;
}

static void xCam_CorrectH(xCamera* cam, F32 f1, F32 f2, F32 f3)
{
    F32 f1_0 = (f1 - f2 * 0.15f * f3) * (10.0f / 7.0f);
    F32 f1_1 = f1_0 - f2 * 0.15f;
    f1_1 *= f3;
    cam->mat.pos.y += f1_1;
}

static void xCam_CorrectP(xCamera* cam, F32 f1, F32 f2, F32 f3)
{
    F32 f1_0 = (2.0f * f1 - f2 * 0.15f * f3) * 2.5f;
    F32 f1_1 = f1_0 - f2 * 0.15f;
    f1_1 *= f3;
    F32 f0 = cam->mbasis.right.x * f1_1;
    F32 f1_2 = cam->mbasis.right.z * f1_1;
    cam->mat.pos.x += f0;
    cam->mat.pos.z += f1_2;
}

static void xCam_DampP(xCamera* cam, F32 f1, F32 f2)
{
    F32 f2_0 = -6.0f * f1 * f2 * f2;
    F32 f0_0 = cam->mbasis.right.x * f2_0;
    F32 f0_1 = cam->mbasis.right.z * f2_0;
    cam->mat.pos.x += f0_0;
    cam->mat.pos.z += f0_1;
}

static void xCam_CorrectYaw(xCamera* cam, F32 f1, F32 f2, F32 f3)
{
    F32 f1_0 = (2.0f * cam->yaw_cd * f1 - f2 * f3) * (1.0f / cam->yaw_ct);
    F32 f1_1 = f1_0 - f2;
    f1_1 *= cam->yaw_csv * f3;
    cam->yaw_cur += f1_1;
}

static void xCam_CorrectPitch(xCamera* cam, F32 f1, F32 f2, F32 f3)
{
    F32 f1_0 = (2.0f * cam->pitch_cd * f1 - f2 * f3) * (1.0f / cam->pitch_ct);
    F32 f1_1 = f1_0 - f2;
    f1_1 *= cam->pitch_csv * f3;
    cam->pitch_cur += f1_1;
}

static void xCam_CorrectRoll(xCamera* cam, F32 f1, F32 f2, F32 f3)
{
    F32 f1_0 = (2.0f * cam->roll_cd * f1 - f2 * f3) * (1.0f / cam->roll_ct);
    F32 f1_1 = f1_0 - f2;
    f1_1 *= cam->roll_csv * f3;
    cam->roll_cur += f1_1;
}

void SweptSphereHitsCameraEnt(xScene* sc, xRay3* ray, xQCData* qcd, xEnt* ent, void* data)
{
    if (!ent->camcollModel) return;
    if (!(ent->chkby & 0x10)) return;
    if (!xQuickCullIsects(qcd, &ent->bound.qcd)) return;

    xSweptSphere* sws = (xSweptSphere*)data;

    if (!xEntIsVisible(ent)) {
        if (ent->model->Data != sInvisWallHack) return;
        if (ent->collLev != 5) {
            if (ent->bound.type == 2) {
                xSweptSphereToBox(sws, &ent->bound.box.box, NULL);
                return;
            } else if (ent->bound.type == 4) {
                xSweptSphereToBox(sws, &ent->bound.box.box, ent->bound.mat);
                return;
            } else {
                return;
            }
        }
    }

    U32 result = 0;
    switch (ent->bound.type) {
    case 1:
    {
        F32 oldrad = ent->bound.sph.r;
        ent->bound.sph.r += sws->radius;
        result = xRayHitsSphereFast(ray, &ent->bound.sph);
        ent->bound.sph.r = oldrad;
        break;
    }
    case 2:
    {
        xBox tmpbox;
        tmpbox.upper.x = ent->bound.box.box.upper.x + sws->radius;
        tmpbox.upper.y = ent->bound.box.box.upper.y + sws->radius;
        tmpbox.upper.z = ent->bound.box.box.upper.z + sws->radius;
        tmpbox.lower.x = ent->bound.box.box.lower.x - sws->radius;
        tmpbox.lower.y = ent->bound.box.box.lower.y - sws->radius;
        tmpbox.lower.z = ent->bound.box.box.lower.z - sws->radius;
        result = xRayHitsBoxFast(ray, &tmpbox);
        break;
    }
    case 4:
    {
        F32 f31 = xVec3Length(&ent->bound.mat->right);
        xBox tmpbox;
        xRay3 lr;
        xMat3x3 mn;
        xMat3x3Normalize(&mn, ent->bound.mat);
        xMat4x3Tolocal(&lr.origin, ent->bound.mat, &ray->origin);
        xMat3x3Tolocal(&lr.dir, &mn, &ray->dir);
        lr.max_t = ray->max_t / f31;
        lr.min_t = ray->min_t / f31;
        lr.flags = ray->flags;
        tmpbox.upper.x = ent->bound.box.box.upper.x + sws->radius / f31;
        tmpbox.upper.y = ent->bound.box.box.upper.y + sws->radius / f31;
        tmpbox.upper.z = ent->bound.box.box.upper.z + sws->radius / f31;
        tmpbox.lower.x = ent->bound.box.box.lower.x - sws->radius / f31;
        tmpbox.lower.y = ent->bound.box.box.lower.y - sws->radius / f31;
        tmpbox.lower.z = ent->bound.box.box.lower.z - sws->radius / f31;
        result = xRayHitsBoxFast(&lr, &tmpbox);
        break;
    }
    }

    if (result) {
        xModelInstance* collmod = ent->camcollModel;
        xSweptSphereToModel(sws, collmod->Data, collmod->Mat);
    }
}

static void _xCameraUpdate(xCamera* cam, F32 dt) NONMATCH("https://decomp.me/scratch/2q6zO")
{
    if (!cam->tgt_mat) return;

    static F32 last_dt = 1.0f/VBLANKS_PER_SEC;

    xCam_worldtocyl(cam->dcur, cam->hcur, cam->pcur, cam->tgt_mat, &cam->mat.pos, cam->flags);

    F32 wcvx = cam->mat.pos.x - cam->omat.pos.x;
    F32 wcvy = cam->mat.pos.y - cam->omat.pos.y;
    F32 wcvz = cam->mat.pos.z - cam->omat.pos.z;
    F32 m = 1.0f / last_dt;
    wcvx *= m;
    wcvy *= m;
    wcvz *= m;

    cam->omat.pos = cam->mat.pos;

    xCam_buildbasis(cam);

    F32 dcv = wcvx * cam->mbasis.at.x + wcvz * cam->mbasis.at.z;
    F32 hcv = wcvy;
    F32 pcv = wcvx * cam->mbasis.right.x + wcvz * cam->mbasis.right.z;
    wcvx *= dt;
    wcvy *= dt;
    wcvz *= dt;

    cam->mat.pos.x += wcvx;
    cam->mat.pos.y += wcvy;
    cam->mat.pos.z += wcvz;

    if (cam->flags & 0x1) {
        F32 tnext = cam->tmr - dt;
        if (tnext <= 0.0f) {
            cam->flags &= ~0x1;
            cam->tmr = 0.0f;
            cam->omat.pos = cam->mat.pos;
        } else {
            F32 dtg = cam->dgoal - cam->dcur;
            F32 htg = cam->hgoal - cam->hcur;
            F32 ptg = (cam->dgoal + cam->dcur) * xDangleClamp(cam->pgoal - cam->pcur) * 0.5f;
            F32 dsv, hsv, psv;
            if (tnext <= cam->tm_dec) {
                F32 T_inv = 1.0f / cam->tmr;
                dsv = (2.0f * dtg - dcv * dt) * T_inv;
                hsv = (2.0f * htg - hcv * dt) * T_inv;
                psv = (2.0f * ptg - pcv * dt) * T_inv;
            } else if (tnext <= cam->tm_acc) {
                F32 T_inv = 1.0f / (2.0f * cam->tmr - dt - cam->tm_dec);
                dsv = (2.0f * dtg - dcv * dt) * T_inv;
                hsv = (2.0f * htg - hcv * dt) * T_inv;
                psv = (2.0f * ptg - pcv * dt) * T_inv;
            } else {
                F32 it = cam->tm_acc + (cam->tmr - dt) - cam->tm_dec;
                F32 ot = 2.0f / (cam->tmr + cam->tm_acc - cam->tm_dec);
                F32 T_inv = 1.0f / (cam->tmr - cam->tm_acc);
                dsv = (2.0f * dtg - (dtg * ot + cam->depv) * 0.5f * it - dcv * dt) * T_inv;
                hsv = (2.0f * htg - (htg * ot + cam->hepv) * 0.5f * it - hcv * dt) * T_inv;
                psv = (2.0f * ptg - (ptg * ot + cam->pepv) * 0.5f * it - pcv * dt) * T_inv;
            }
            F32 dpv = dsv - dcv;
            F32 hpv = hsv - hcv;
            F32 ppv = psv - pcv;
            F32 vax = cam->mbasis.right.x * ppv + cam->mbasis.at.x * dpv;
            F32 vay = cam->mbasis.right.y * ppv + hpv;
            F32 vaz = cam->mbasis.right.z * ppv + cam->mbasis.at.z * dpv;
            vax *= dt;
            vay *= dt;
            vaz *= dt;
            cam->mat.pos.x += vax;
            cam->mat.pos.y += vay;
            cam->mat.pos.z += vaz;
            cam->tmr = tnext;
        }
    } else {
        if (cam->flags & 0x2) {
            if (xeq(cam->dcur / cam->dgoal, 1.0f, EPSILON)) {
            } else {
                F32 dtg = cam->dgoal - cam->dcur;
                xCam_CorrectD(cam, dtg, dcv, dt);
            }
        } else if (cam->dmax > cam->dmin) {
            if (cam->dcur < cam->dmin) {
                F32 dtg = cam->dmin - cam->dcur;
                xCam_CorrectD(cam, dtg, dcv, dt);
            } else if (cam->dcur > cam->dmax) {
                F32 dtg = cam->dmax - cam->dcur;
                xCam_CorrectD(cam, dtg, dcv, dt);
            }
        }

        if (cam->flags & 0x4) {
            if (xeq(cam->hcur / cam->hgoal, 1.0f, EPSILON)) {
            } else {
                F32 htg = cam->hgoal - cam->hcur;
                xCam_CorrectH(cam, htg, hcv, dt);
            }
        } else if (cam->hmax > cam->hmin) {
            if (cam->hcur < cam->hmin) {
                F32 htg = cam->hmin - cam->hcur;
                xCam_CorrectH(cam, htg, hcv, dt);
            } else if (cam->hcur > cam->hmax) {
                F32 htg = cam->hmax - cam->hcur;
                xCam_CorrectH(cam, htg, hcv, dt);
            }
        }

        if (cam->flags & 0x8) {
            if (xeq(cam->pcur / cam->pgoal, 1.0f, EPSILON)) {
            } else {
                F32 ptg = cam->dcur * xDangleClamp(cam->pgoal - cam->pcur);
                xCam_CorrectP(cam, ptg, pcv, dt);
            }
        } else if (cam->pmax > cam->pmin) {
            F32 dphi = xDangleClamp(cam->pmax - cam->pcur);
            F32 dplo = xDangleClamp(cam->pmin - cam->pcur);
            if (dplo > 0.0f && (dphi > 0.0f || xabs(dplo) <= xabs(dphi))) {
                F32 ptg = (EPSILON + dplo) * cam->dcur;
                xCam_CorrectP(cam, ptg, pcv, dt);
            } else if (dphi < 0.0f) {
                F32 ptg = (dphi - EPSILON) * cam->dcur;
                xCam_CorrectP(cam, ptg, pcv, dt);
            } else {
                xCam_DampP(cam, pcv, dt);
            }
        } else {
            xCam_DampP(cam, pcv, dt);
        }
    }

    if (cam->flags & 0x80) {
        xVec3 oeu, eu;
        xMat3x3GetEuler(&cam->mat, &eu);
        xMat3x3GetEuler(&cam->omat, &oeu);

        F32 m = 1.0f / last_dt;
        F32 ycv = m * xDangleClamp(eu.x - oeu.x);
        F32 pcv = m * xDangleClamp(eu.y - oeu.y);
        F32 rcv = m * xDangleClamp(eu.z - oeu.z);
        ycv *= cam->yaw_ccv;
        pcv *= cam->pitch_ccv;
        rcv *= cam->roll_ccv;

        cam->omat = cam->mat;
        cam->yaw_cur += ycv * dt;
        cam->pitch_cur += pcv * dt;
        cam->roll_cur += rcv * dt;

        if (cam->flags & 0x40) {
            F32 tnext = cam->ltmr - dt;
            if (tnext <= 0.0f) {
                cam->flags &= ~0x40;
                cam->ltmr = 0.0f;
            } else {
                F32 ytg = xDangleClamp(cam->yaw_goal - cam->yaw_cur);
                F32 ptg = xDangleClamp(cam->pitch_goal - cam->pitch_cur);
                F32 rtg = xDangleClamp(cam->roll_goal - cam->roll_cur);
                F32 ysv, psv, rsv;
                if (tnext <= cam->ltm_dec) {
                    F32 T_inv = 1.0f / cam->ltmr;
                    ysv = (2.0f * ytg - ycv * dt) * T_inv;
                    psv = (2.0f * ptg - pcv * dt) * T_inv;
                    rsv = (2.0f * rtg - rcv * dt) * T_inv;
                } else if (tnext <= cam->ltm_acc) {
                    F32 T_inv = 1.0f / (2.0f * cam->ltmr - dt - cam->ltm_dec);
                    ysv = (2.0f * ytg - ycv * dt) * T_inv;
                    psv = (2.0f * ptg - pcv * dt) * T_inv;
                    rsv = (2.0f * rtg - rcv * dt) * T_inv;
                } else {
                    F32 it = cam->ltm_acc + (cam->ltmr - dt) - cam->ltm_dec;
                    F32 ot = 2.0f / (cam->ltmr + cam->ltm_acc - cam->ltm_dec);
                    F32 T_inv = 1.0f / (cam->ltmr - cam->ltm_acc);
                    ysv = ((2.0f * ytg - (ytg * ot + cam->yaw_epv) * 0.5f * it) - ycv * dt) * T_inv;
                    psv = ((2.0f * ptg - (ptg * ot + cam->pitch_epv) * 0.5f * it) - pcv * dt) * T_inv;
                    rsv = ((2.0f * rtg - (rtg * ot + cam->roll_epv) * 0.5f * it) - rcv * dt) * T_inv;
                }
                F32 ypv = ysv - ycv;
                F32 ppv = psv - pcv;
                F32 rpv = rsv - rcv;
                cam->yaw_cur += ypv * dt;
                cam->pitch_cur += ppv * dt;
                cam->roll_cur += rpv * dt;
                xMat3x3Euler(&cam->mat, cam->yaw_cur, cam->pitch_cur, cam->roll_cur);
                cam->ltmr = tnext;
            }
        } else {
            if (xeq(cam->yaw_cur, cam->yaw_goal,  EPSILON)) {
            } else {
                F32 ytg = xDangleClamp(cam->yaw_goal - cam->yaw_cur);
                xCam_CorrectYaw(cam, ytg, ycv, dt);
            }

            if (xeq(cam->pitch_cur, cam->pitch_goal, EPSILON)) {
            } else {
                F32 ptg = xDangleClamp(cam->pitch_goal - cam->pitch_cur);
                xCam_CorrectPitch(cam, ptg, pcv, dt);
            }

            if (xeq(cam->roll_cur, cam->roll_goal, EPSILON)) {
            } else {
                F32 rtg = xDangleClamp(cam->roll_goal - cam->roll_cur);
                xCam_CorrectRoll(cam, rtg, rcv, dt);
            }

            xMat3x3Euler(&cam->mat, cam->yaw_cur, cam->pitch_cur, cam->roll_cur);
        }
    } else {
        xQuatFromMat(&cam->orn_cur, &cam->mat);

        xQuat oq;
        xQuatFromMat(&oq, &cam->omat);
        
        xQuat qdiff_o_c;
        xQuatDiff(&qdiff_o_c, &oq, &cam->orn_cur);
        
        xRot rot_cv;
        xQuatToAxisAngle(&qdiff_o_c, &rot_cv.axis, &rot_cv.angle);
        rot_cv.angle *= m;
        rot_cv.angle = 0.0f; // lol

        cam->omat = cam->mat;

        xVec3 f;
        xMat3x3RMulVec(&f, cam->tgt_mat, &cam->focus);
        xVec3AddTo(&f, &cam->tgt_mat->pos);

        xVec3 v;
        F32 dist;
        xVec3NormalizeDistXZMacro(&v, &cam->mat.pos, &cam->tgt_mat->pos, &dist);
        v.y = 0.0f;
        
        if (cam->tgt_mat->at.x * v.x +
            cam->tgt_mat->at.y * v.y +
            cam->tgt_mat->at.z * v.z < 0.0f) {
            F32 mpx = f.x - cam->tgt_mat->pos.x;
            F32 mpy = f.y - cam->tgt_mat->pos.y;
            F32 mpz = f.z - cam->tgt_mat->pos.z;
            F32 s = (mpx * v.x + mpy * v.y + mpz * v.z) * -2.0f;
            mpx = v.x * s;
            mpy = v.y * s;
            mpz = v.z * s;
            f.x += mpx;
            f.y += mpy;
            f.z += mpz;
        }

        xMat3x3 des_mat;
        xMat3x3LookAt(&des_mat, &f, &cam->mat.pos);
        
        xMat3x3 latgt;
        xMat3x3LookAt(&latgt, &cam->tgt_mat->pos, &cam->mat.pos);

        F32 ang_dist = xacos(latgt.at.x * des_mat.at.x +
                             latgt.at.y * des_mat.at.y +
                             latgt.at.z * des_mat.at.z);

        if (ang_dist > xdeg2rad(30.0f)) {
            xQuat a;
            xQuatFromMat(&a, &latgt);

            xQuat b;
            xQuatFromMat(&b, &des_mat);

            xQuat o;
            F32 s = PI - ang_dist;
            if (s < xdeg2rad(90.0f)) {
                if (s > xdeg2rad(5.0f)) {
                    xQuatSlerp(&o, &a, &b, s / ang_dist);
                } else {
                    o = a;
                }
            } else {
                xQuatSlerp(&o, &a, &b, xdeg2rad(30.0f) / ang_dist);
            }

            xQuatToMat(&o, &des_mat);
        }

        xQuat desq;
        xQuatFromMat(&desq, &des_mat);

        xCameraLook(cam, 0, &desq, 0.25f, 0.0f, 0.0f);
        
        xQuat difq;
        xQuatConj(&difq, &cam->orn_cur);
        xQuatMul(&difq, &difq, &desq);
        
        xQuat newq;
        xQuatSlerp(&newq, &cam->orn_cur, &desq, 25.5f * dt);
        xQuatToMat(&newq, &cam->mat);
    }

    while (xcam_do_collis && sCamCollis) {
        xSweptSphere sws;
        
        xVec3 tgtpos;
        tgtpos.x = cam->tgt_mat->pos.x;
        tgtpos.y = 0.7f + cam->tgt_mat->pos.y;
        tgtpos.z = cam->tgt_mat->pos.z;

        xSweptSpherePrepare(&sws, &tgtpos, &cam->mat.pos, 0.07f);
        xSweptSphereToEnv(&sws, globals.sceneCur->env);
        
        xRay3 ray;
        xVec3Copy(&ray.origin, &sws.start);
        xVec3Sub(&ray.dir, &sws.end, &sws.start);

        ray.max_t = xVec3Length(&ray.dir);
        
        F32 one_len = 1.0f / xmax(ray.max_t, EPSILON);
        xVec3SMul(&ray.dir, &ray.dir, one_len);

        ray.flags = 0x800;
        if (!(ray.flags & 0x400)) {
            ray.flags |= 0x400;
            ray.min_t = 0.0f;
        }

        xRayHitsGrid(&colls_grid, globals.sceneCur, &ray, SweptSphereHitsCameraEnt, &sws.qcd, &sws);
        xRayHitsGrid(&colls_oso_grid, globals.sceneCur, &ray, SweptSphereHitsCameraEnt, &sws.qcd, &sws);

        if (sws.curdist != sws.dist) {
            F32 stopdist = xmax(sws.curdist, 0.6f);
            cam->mat.pos.x = ray.origin.x + stopdist * ray.dir.x;
            cam->mat.pos.y = ray.origin.y + stopdist * ray.dir.y;
            cam->mat.pos.z = ray.origin.z + stopdist * ray.dir.z;
        }

        break;
    }

    last_dt = dt;

    iCameraUpdatePos(cam->lo_cam, &cam->mat);
}

void xCameraUpdate(xCamera* cam, F32 dt)
{
    S32 i;
    S32 num_updates = (S32)std::ceilf(144.0f * dt);
    F32 sdt = dt / num_updates;
    for (i = 0; i < num_updates; i++) {
        sCamCollis = (i == num_updates - 1);
        _xCameraUpdate(cam, sdt);
    }
}

void xCameraBegin(xCamera* cam, S32 clear)
{
    iCameraBegin(cam->lo_cam, clear);
    iCameraFrustumPlanes(cam->lo_cam, cam->frustplane);
    iCameraUpdateFog(cam->lo_cam, 0);
}

void xCameraFXBegin(xCamera* cam)
{
    xMat4x3Identity(&sCameraFXMatOld);
    xMat4x3Copy(&sCameraFXMatOld, &cam->mat);
}

static void xCameraFXInit()
{
    memset(&sCameraFX, 0, sizeof(sCameraFX));
    for (S32 i = 0; i < CAMERAFX_MAX; i++) {
        sCameraFX[i].flags = 0;
    }
}

cameraFX* xCameraFXAlloc() NONMATCH("https://decomp.me/scratch/uO8I8")
{
    for (S32 i = 0; i < CAMERAFX_MAX; i++) {
        cameraFX* f = &sCameraFX[i];
        if (f->flags == 0) {
            f->flags = CAMERAFX_ACTIVE;
            f->elapsedTime = 0.0f;
            f->maxTime = 0.0f;
            return f;
        }
    }
    return NULL;
}

void xCameraFXZoomUpdate(cameraFX* f, F32 dt, const xMat4x3*, xMat4x3* m) NONMATCH("https://decomp.me/scratch/xR6lL")
{
    switch (f->zoom.mode) {
    case eCameraFXZoomOut:
        f->zoom.velCur += f->zoom.accel * dt;
        f->zoom.distanceCur += f->zoom.velCur * dt;
        if (f->zoom.distanceCur >= f->zoom.distance) {
            f->zoom.distanceCur = f->zoom.distance;
            f->zoom.mode = eCameraFXZoomHold;
            f->zoom.holdTimeCur = 0.0f;
        }
        xMat4x3MoveLocalAt(m, f->zoom.distanceCur);
        break;
    case eCameraFXZoomHold:
        f->zoom.holdTimeCur += dt;
        if (f->zoom.holdTimeCur > f->zoom.holdTime) {
            f->zoom.mode = eCameraFXZoomIn;
            f->zoom.distanceCur = f->zoom.distance;
            f->zoom.velCur = f->zoom.vel;
        }
        xMat4x3MoveLocalAt(m, f->zoom.distance);
        break;
    case eCameraFXZoomIn:
        f->zoom.velCur += f->zoom.accel * dt;
        f->zoom.distanceCur -= f->zoom.velCur * dt;
        if (f->zoom.distanceCur <= 0.0f) {
            f->zoom.distanceCur = 0.0f;
            f->zoom.mode = eCameraFXZoomDone;
            f->flags |= CAMERAFX_DONE;
        }
        xMat4x3MoveLocalAt(m, f->zoom.distanceCur);
        break;
    case eCameraFXZoomDone:
        break;
    }
}

void xCameraFXShake(F32 maxTime, F32 magnitude, F32 cycleMax, F32 rotate_magnitude, F32 radius,
                    const xVec3* epicenter, const xVec3* player) NONMATCH("https://decomp.me/scratch/y5dIM")
{
    cameraFX* f = xCameraFXAlloc();
    if (!f) return;

    f->type = eCameraFXShake;
    f->maxTime = maxTime;
    f->shake.magnitude = magnitude;
    f->shake.dir.x = 1.0f;
    f->shake.dir.y = 1.0f;
    f->shake.cycleMax = cycleMax;
    f->shake.cycleTime = 0.0f;
    f->shake.dampen = 0.0f;
    f->shake.dampenRate = 1.0f / maxTime;
    f->shake.rotate_magnitude = rotate_magnitude;
    f->shake.radius = radius;
    f->shake.epicenterP = epicenter;
    if (f->shake.epicenterP) f->shake.epicenter = *f->shake.epicenterP;
    f->shake.player = player;
}

void xCameraFXShakeUpdate(cameraFX* f, F32 dt, const xMat4x3*, xMat4x3* m)
{
    f->shake.cycleTime += dt;
    while (f->shake.cycleTime > f->shake.cycleMax) {
        f->shake.dir.x = -f->shake.dir.x;
        f->shake.dir.y = -f->shake.dir.y;
        f->shake.cycleTime -= f->shake.cycleMax;
    }
    
    F32 x, y;
    F32 scale = f->shake.dampenRate * (f->maxTime - f->elapsedTime);
    F32 noise = 0.1f * (xurand() - 0.5f);

    if (f->shake.radius > 0.0f && f->shake.player) {
        xVec3 d;
        xVec3Sub(&d, f->shake.player, &f->shake.epicenter);
        
        F32 len = d.length();
        if (len > f->shake.radius) {
            scale = 0.0f;
        } else {
            scale *= icos(PI * (len / f->shake.radius) * 0.5f);
        }
    }

    x = f->shake.dir.x * (f->shake.magnitude + noise) * scale / f->shake.cycleMax * f->shake.cycleTime * isin(PI * (f->shake.cycleTime / f->shake.cycleMax));
    noise = 0.1f * (xurand() - 0.5f);
    y = f->shake.dir.y * (f->shake.magnitude + noise) * scale / f->shake.cycleMax * f->shake.cycleTime * isin(PI * (f->shake.cycleTime / f->shake.cycleMax));

    xMat4x3MoveLocalRight(m, x);
    xMat4x3MoveLocalUp(m, y);
    
    xVec3 e;
    xMat3x3GetEuler(m, &e);
    e.z += f->shake.cycleTime / f->shake.cycleMax * (2.0f/PI) * 0.1f * scale * f->shake.rotate_magnitude;
    xMat3x3Euler(m, &e);
}

void xCameraFXUpdate(xCamera* cam, F32 dt) NONMATCH("https://decomp.me/scratch/YIOgp")
{
    for (S32 i = 0; i < CAMERAFX_MAX; i++) {
        cameraFX* f = &sCameraFX[i];
        if (f->flags & CAMERAFX_ACTIVE) {
            f->elapsedTime += dt;
            if ((f->maxTime > 0.0f && f->elapsedTime > f->maxTime) || (f->flags & CAMERAFX_DONE)) {
                f->flags = 0;
                if (sCameraFXTable[f->type].funcKill) {
                    sCameraFXTable[f->type].funcKill(f);
                }
            } else {
                if (sCameraFXTable[f->type].func) {
                    sCameraFXTable[f->type].func(f, dt, &sCameraFXMatOld, &cam->mat);
                }
            }
        }
    }

    iCameraUpdatePos(cam->lo_cam, &cam->mat);
}

void xCameraFXEnd(xCamera* cam)
{
    xMat4x3Copy(&cam->mat, &sCameraFXMatOld);
    iCameraUpdatePos(cam->lo_cam, &sCameraFXMatOld);
}

void xCameraEnd(xCamera* cam, F32 seconds, S32 update_scrn_fx)
{
    if (update_scrn_fx) {
        xScrFxUpdate(cam->lo_cam, seconds);
    }
    iCameraEnd(cam->lo_cam);
}

void xCameraShowRaster(xCamera* cam)
{
    iCameraShowRaster(cam->lo_cam);
}

void xCameraSetScene(xCamera* cam, xScene* sc)
{
    cam->sc = sc;
    iCameraAssignEnv(cam->lo_cam, sc->env->geom);
}

void xCameraSetTargetMatrix(xCamera* cam, xMat4x3* mat)
{
    cam->tgt_mat = mat;
}

void xCameraSetTargetOMatrix(xCamera* cam, xMat4x3* mat)
{
    cam->tgt_omat = mat;
}

void xCameraDoCollisions(S32 do_collis, S32 owner) NONMATCH("https://decomp.me/scratch/ArCQC")
{
    xcam_collis_owner_disable &= ~(1 << owner);
    xcam_collis_owner_disable |= !do_collis << owner;
    xcam_do_collis = !xcam_collis_owner_disable;
}

void xCameraMove(xCamera* cam, U32 flags, F32 dgoal, F32 hgoal, F32 pgoal,
                 F32 tm, F32 tm_acc, F32 tm_dec) NONMATCH("https://decomp.me/scratch/dKFmj")
{
    cam->flags = (cam->flags & ~0x3E) | (flags & 0x3E);
    cam->dgoal = dgoal;
    cam->hgoal = hgoal;
    cam->pgoal = pgoal;

    if (tm <= 0.0f) {
        if (!cam->tgt_mat) return;

        cam->dcur = dgoal;
        cam->hcur = hgoal;
        cam->pcur = pgoal;

        xCam_cyltoworld(&cam->mat.pos, cam->tgt_mat, dgoal, hgoal, pgoal, cam->flags);
        cam->omat.pos = cam->mat.pos;
        cam->yaw_cur = cam->yaw_goal = cam->pcur + (cam->pcur >= PI ? -PI : PI);
    } else {
        cam->flags |= 0x1;
        cam->tm_acc = tm - tm_acc;
        cam->tm_dec = tm_dec;
        cam->tmr = tm;

        F32 s = 1.0f / (tm - 0.5f * (tm_acc - tm_dec));
        cam->depv = s * (dgoal - cam->dcur);
        cam->hepv = s * (hgoal - cam->hcur);
        cam->pepv = s * xDangleClamp(pgoal - cam->pcur) * 0.5f * (dgoal + cam->dcur);
    }
}

void xCameraMove(xCamera* cam, const xVec3& loc) NONMATCH("https://decomp.me/scratch/qdRWT")
{
    cam->omat.pos = cam->mat.pos = loc;
    cam->flags &= ~0x3E;
    cam->tm_acc = cam->tm_dec = cam->tmr = 0.0f;
}

void xCameraMove(xCamera* cam, const xVec3& loc, F32 maxSpeed) NONMATCH("https://decomp.me/scratch/w7Rfh")
{
    xVec3 var_28;
    xVec3Sub(&var_28, &loc, &cam->mat.pos);

    F32 f1 = xVec3Length(&var_28);
    if (f1 > maxSpeed) {
        F32 f1_0 = maxSpeed / f1;
        xVec3SMul(&var_28, &var_28, f1_0);
        xVec3Add(&cam->mat.pos, &cam->mat.pos, &var_28);
    } else {
        cam->mat.pos = loc;
    }

    cam->omat.pos = cam->mat.pos;
    cam->flags &= ~0x3E;
    cam->tm_acc = cam->tm_dec = cam->tmr = 0.0f;
}

void xCameraFOV(xCamera* cam, F32 fov, F32 maxSpeed, F32 dt) NONMATCH("https://decomp.me/scratch/uUYzD")
{
    F32 speed = maxSpeed * dt;
    F32 currentFOV = xCameraGetFOV(cam);
    if (currentFOV == fov) return;

    if (speed != 0.0f) {
        F32 len = fov - currentFOV;
        if (xabs(len) > speed) {
            F32 f1 = speed / len * len;
            xCameraSetFOV(cam, currentFOV + f1);
        } else {
            xCameraSetFOV(cam, fov);
        }
    } else {
        xCameraSetFOV(cam, fov);
    }
}

void xCameraLook(xCamera* cam, U32 flags, const xQuat* orn_goal, F32 tm, F32 tm_acc, F32 tm_dec)
{
    cam->flags = (cam->flags & ~0xF80) | (flags & 0xF80);
    cam->orn_goal = *orn_goal;

    if (tm <= 0.0f) {
        if (!cam->tgt_mat) return;

        xQuatToMat(orn_goal, &cam->mat);
        cam->omat = cam->mat;
    } else {
        cam->flags |= 0x40;
        cam->ltm_acc = tm - tm_acc;
        cam->ltm_dec = tm_dec;
        cam->ltmr = tm;

        xQuatDiff(&cam->orn_diff, &cam->orn_cur, orn_goal);
        
        F32 s = xQuatGetAngle(&cam->orn_diff);
        F32 f0 = 1.0f / (tm - 0.5f * (tm_acc - tm_dec));
        cam->orn_epv = f0 * s;
    }
}

void xCameraLookYPR(xCamera* cam, U32 flags, F32 yaw, F32 pitch, F32 roll,
                    F32 tm, F32 tm_acc, F32 tm_dec) NONMATCH("https://decomp.me/scratch/M3zSQ")
{
    cam->flags = (cam->flags & ~0xF80) | (flags & 0xF80) | 0x80;
    cam->yaw_goal = yaw;
    cam->pitch_goal = pitch;
    cam->roll_goal = roll;

    if (tm <= 0.0f) {
        if (!cam->tgt_mat) return;

        cam->yaw_cur = yaw;
        cam->pitch_cur = pitch;
        cam->roll_cur = roll;

        xMat3x3Euler(&cam->mat, yaw, pitch, roll);
        cam->omat = cam->mat;
    } else {
        cam->flags |= 0x40;
        cam->ltm_acc = tm - tm_acc;
        cam->ltm_dec = tm_dec;
        cam->ltmr = tm;

        F32 s = 1.0f / (tm - 0.5f * (tm_acc - tm_dec));
        cam->yaw_epv = s * xDangleClamp(yaw - cam->yaw_cur);
        cam->pitch_epv = s * xDangleClamp(pitch - cam->pitch_cur);
        cam->roll_epv = s * xDangleClamp(roll - cam->roll_cur);
    }
}

void xCameraRotate(xCamera* cam, const xMat3x3& m, F32 time, F32 accel, F32 decl) NONMATCH("https://decomp.me/scratch/hNyVk")
{
    cam->flags = (cam->flags & ~0xF80) | 0x80;

    xVec3 eu;
    xMat3x3GetEuler(&m, &eu);

    cam->yaw_goal = eu.x;
    cam->pitch_goal = eu.y;
    cam->roll_goal = eu.z;

    if (time == 0.0f) {
        cam->yaw_cur = eu.x;
        cam->pitch_cur = eu.y;
        cam->roll_cur = eu.z;
    }

    (xMat3x3)cam->mat = m;
    if (time == 0.0f) (xMat3x3)cam->omat = m;

    if (time == 0.0f) {
        cam->ltm_acc = cam->ltm_dec = cam->ltmr = 0.0f;
    } else {
        cam->ltm_acc = accel;
        cam->ltm_dec = decl;
        cam->ltmr = time;
    }

    cam->yaw_epv = cam->pitch_epv = cam->roll_epv = 0.0f;
}

void xCameraRotate(xCamera* cam, const xVec3& v, F32 roll, F32 time, F32 accel, F32 decl) NONMATCH("https://decomp.me/scratch/c0uZ0")
{
    cam->yaw_goal = xatan2(v.x, v.z);
    cam->pitch_goal = -xasin(v.y);
    cam->roll_goal = roll;

    if (time == 0.0f) {
        cam->yaw_cur = cam->yaw_goal;
        cam->pitch_cur = cam->pitch_goal;
        cam->roll_cur = cam->roll_goal;
    }

    cam->flags = (cam->flags & ~0xF80) | 0x80;

    xMat3x3Euler(&cam->mat, cam->yaw_goal, cam->pitch_goal, cam->roll_goal);
    if (time == 0.0f) cam->omat = cam->mat;

    if (time == 0.0f) {
        cam->ltm_acc = cam->ltm_dec = cam->ltmr = 0.0f;
    } else {
        cam->ltm_acc = accel;
        cam->ltm_dec = decl;
        cam->ltmr = time;
    }

    cam->yaw_epv = cam->pitch_epv = cam->roll_epv = 0.0f;
}

static void bound_sphere_xz(xVec3& r28, xVec3& r29, const xVec3& r30, F32 f30,
                            const xVec3& r31, F32 f2)
{
    F32 f31 = f30 / f2;
    F32 f3 = f31 * xsqrt(xsqr(f2) - xsqr(f30));
    F32 f5 = f30 * f31;
    F32 f6 = f3 * r31.x;
    F32 f7 = f3 * r31.z;
    F32 f8 = f5 * r31.x;
    F32 f5_0 = f5 * r31.z;
    r28.x = r30.x + f7 + f8;
    r28.y = r30.y;
    r28.z = r30.z - f6 + f5_0;
    r29.x = r30.x - f7 + f8;
    r29.y = r30.y;
    r29.z = r30.z + f6 + f5_0;
}

void xBinaryCamera::init()
{
    camera = NULL;
    s1 = s2 = NULL;
}

void xBinaryCamera::start(xCamera& camera)
{
    this->camera = &camera;
    xQuatFromMat(&cam_dir, &camera.mat);
    stick_offset = 0.0f;
}

void xBinaryCamera::stop()
{
    camera = NULL;
}

void xBinaryCamera::update(F32 dt) NONMATCH("https://decomp.me/scratch/c9iST")
{
    xVec3& A = camera->mat.pos;
    const xVec3& B = *s1;
    const xVec3& C = *s2;
    
    xVec3 CA = { 0.0f, 0.0f, 0.0f };
    CA.x = A.x - C.x;
    CA.z = A.z - C.z;
    
    F32 dCA = CA.length();
    if (dCA < 0.01f) {
        CA.assign(A.x - B.x, 0.0f, A.z - B.z).right_normalize();
        dCA = 0.01f;
    } else {
        CA /= dCA;
    }

    F32 yaw_start = xatan2(B.x - A.x, B.z - A.z);
    F32 yaw_end;
    if (dCA > s2_radius) {
        xVec3 Q1, Q2;
        bound_sphere_xz(Q1, Q2, C, s2_radius, CA, dCA);
        F32 yaw_Q1 = xatan2(Q1.x - A.x, Q1.z - A.z);
        F32 yaw_Q2 = xatan2(Q2.x - A.x, Q2.z - A.z);
        F32 dyaw1 = xrmod(yaw_Q1 - yaw_start + PI) - PI;
        F32 dyaw2 = xrmod(yaw_Q2 - yaw_start + PI) - PI;
        
        F32 fov = xdeg2rad(1.0f) * xCameraGetFOV(camera) * 0.5f + cfg.margin_angle;
        fov = range_limit(fov, 0.0f, PI);
        F32 max_dyaw = 0.5f * xabs(dyaw2 - dyaw1);
        if (max_dyaw > fov) {
            yaw_end = xatan2(C.x - B.x, C.z - B.z);
        } else if (dyaw1 >= dyaw2) {
            yaw_end = xatan2(C.x - B.x, C.z - B.z);
        } else if (dyaw1 >= -fov) {
            if (dyaw2 <= fov) {
                yaw_end = yaw_start;
            } else {
                yaw_end = yaw_start + (dyaw2 - fov);
            }
        } else {
            yaw_end = yaw_start + (dyaw1 + fov);
        }
    } else {
        yaw_end = xatan2(C.x - B.x, C.z - B.z);
    }

    F32 sstick = 1.0f - xexp(-cfg.stick_speed * dt);
    xPad::analog_data& stick = globals.pad0->analog[1];
    
    stick_offset.x += (cfg.stick_yaw_vel * stick.offset.x * dt - stick_offset.x) * sstick;
    yaw_end += stick_offset.x;
    
    F32 yaw_diff = xrmod(yaw_end - yaw_start + PI) - PI;
    F32 max_yaw_diff = cfg.max_yaw_vel * dt;
    if (xabs(yaw_diff) > max_yaw_diff) {
        if (yaw_diff < 0.0f) {
            if (max_yaw_diff > 0.0f) {
                max_yaw_diff = -max_yaw_diff;
            }
        } else {
            if (max_yaw_diff < 0.0f) {
                max_yaw_diff = -max_yaw_diff;
            }
        }
        yaw_end = yaw_start + max_yaw_diff;
    }

    stick_offset.y += (stick.offset.y - stick_offset.y) * sstick;

    F32 d, h, hf;
    if (stick_offset.y > 0.0f) {
        F32 s = stick_offset.y;
        d = xlerp(cfg.zone_rest.distance, cfg.zone_below.distance, s);
        h = xlerp(cfg.zone_rest.height, cfg.zone_below.height, s);
        hf = xlerp(cfg.zone_rest.height_focus, cfg.zone_below.height_focus, s);
    } else {
        F32 s = -stick_offset.y;
        d = xlerp(cfg.zone_rest.distance, cfg.zone_above.distance, s);
        h = xlerp(cfg.zone_rest.height, cfg.zone_above.height, s);
        hf = xlerp(cfg.zone_rest.height_focus, cfg.zone_above.height_focus, s);
    }

    xVec3 end_loc = { 0.0f, 0.0f, 0.0f };
    end_loc.x = B.x - d * isin(yaw_end);
    end_loc.y = B.y + h;
    end_loc.z = B.z - d * icos(yaw_end);

    F32 sloc = 1.0f - xexp(-cfg.move_speed * dt);
    
    xVec3 cam_loc = { 0.0f, 0.0f, 0.0f };
    cam_loc.x = xlerp(A.x, end_loc.x, sloc);
    cam_loc.y = xlerp(A.y, end_loc.y, sloc);
    cam_loc.z = xlerp(A.z, end_loc.z, sloc);
    
    xVec3 heading = { 0.0f, 0.0f, 0.0f };
    heading.x = B.x - end_loc.x;
    heading.y = B.y - end_loc.y + hf;
    heading.z = B.z - end_loc.z;

    F32 heading_dist2 = heading.length2();
    if (heading_dist2 >= 0.001f) {
        heading /= xsqrt(heading_dist2);
        
        xQuat end_dir;
        xMat3x3 mat;
        xMat3x3LookVec(&mat, &heading.invert());
        xQuatFromMat(&end_dir, &mat);

        F32 sdir = 1.0f - xexp(-cfg.turn_speed * dt);
        xQuatSlerp(&cam_dir, &cam_dir, &end_dir, sdir);
    }

    xMat3x3 mat;
    xQuatToMat(&cam_dir, &mat);
    
    xCameraMove(camera, cam_loc);
    xCameraRotate(camera, mat, 0.0f, 0.0f, 0.0f);
    
    render_debug();
}
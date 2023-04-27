#include "xEntMotion.h"

#include "xEntMotionAsset.h"
#include "xScene.h"
#include "xMovePoint.h"
#include "xMovePointAsset.h"
#include "xEntAsset.h"
#include "xDebug.h"
#include "xDraw.h"
#include "xPad.h"

static xEntMotion** dbg_xems = NULL;
static U16 dbg_num = 0;
static U16 dbg_num_allocd = 0;
static S16 dbg_idx = -1;

static void xEntERMove(xEntMotion* motion, xScene*, F32 dt, xEntFrame* frame);
static void xEntOrbitMove(xEntMotion* motion, xScene*, F32 dt, xEntFrame* frame);
static void xEntMPGetNext(xEntMotion* motion, xMovePoint* prev, xScene*);
static void xEntMPMove(xEntMotion* motion, xScene* sc, F32 dt, xEntFrame* frame);
static void xEntPenMove(xEntMotion* motion, xScene*, F32 dt, xEntFrame* frame);
static void xEntMechMove(xEntMotion* motion, xScene* sc, F32 dt, xEntFrame* frame);
static U32 xEntSldMove(xEntMotion* motion, xScene*, F32 dt, xEntFrame* frame);
static U32 xEntRotMove(xEntMotion* motion, xScene*, F32 dt, xEntFrame* frame);
static void xEntMotionDebugAdd(xEntMotion* xem);
static void xEntMotionDebugCB();
static void xEntMotionDebugWrite(const xEntMotion* xem);
static void xEntMotionDebugDraw(const xEntMotion* xem);
static void xEntMotionDebugIPad(xEntMotion* xem);

void xEntMotionInit(xEntMotion* motion, xEnt* owner, xEntMotionAsset* asset) NONMATCH("https://decomp.me/scratch/NSn2m")
{
    motion->asset = asset;
    motion->type = asset->type;
    motion->flags = asset->flags;

    if (motion->type == k_XENTMOTIONTYPE_ER) {
        xVec3Copy(&motion->er.a, &asset->er.ret_pos);
        xVec3Add(&motion->er.b, &asset->er.ret_pos, &asset->er.ext_dpos);
        motion->er.et = asset->er.ext_tm;
        motion->er.wet = asset->er.ext_wait_tm;
        motion->er.rt = asset->er.ret_tm;
        motion->er.wrt = asset->er.ret_wait_tm;
        if (motion->er.p <= 0.0f) {
            motion->er.p = 1.0f;
        }
        motion->er.brt = motion->er.et + motion->er.wet;
        motion->er.ert = motion->er.brt + motion->er.rt;
        motion->er.p = motion->er.ert + motion->er.wrt;
    } else if (motion->type == k_XENTMOTIONTYPE_ORBIT) {
        xVec3Copy(&motion->orb.c, &asset->orb.center);
        motion->orb.a = asset->orb.w;
        motion->orb.b = asset->orb.h;
        if (asset->orb.period <= 0.00001f) {
            asset->orb.period = 1.0f;
        }
        motion->orb.p = asset->orb.period;
        motion->orb.w = 2*PI / asset->orb.period;
    } else if (motion->type == k_XENTMOTIONTYPE_MP) {
    } else if (motion->type == k_XENTMOTIONTYPE_PEND) {
        if (asset->pen.period <= 0.00001f) {
            asset->pen.period = 1.0f;
        }
        motion->pen.w = 2*PI / asset->pen.period;
    } else if (motion->type == k_XENTMOTIONTYPE_MECH) {
        xEntMotionMechData* mkasst = &asset->mech;
        if (mkasst->sld_tm < 0.00001f) {
            mkasst->sld_tm = 1.0f;
        }
        if (mkasst->sld_acc_tm + mkasst->sld_dec_tm > mkasst->sld_tm) {
            mkasst->sld_dec_tm = mkasst->sld_acc_tm = 0.5f * mkasst->sld_tm;
        }
        if (mkasst->rot_tm < 0.00001f) {
            mkasst->rot_tm = 1.0f;
        }
        if (mkasst->type == k_XENTMOTIONMECH_SLIDE_ROT &&
            mkasst->rot_tm != mkasst->sld_tm) {
            mkasst->rot_tm = mkasst->sld_tm;
        }
        if (mkasst->rot_acc_tm + mkasst->rot_dec_tm > mkasst->rot_tm) {
            mkasst->rot_dec_tm = mkasst->rot_acc_tm = 0.5f * mkasst->rot_tm;
        }
    }
    
    motion->owner = owner;
    motion->target = NULL;

    xEntMotionDebugAdd(motion);
}

void xEntMotionReset(xEntMotion* motion, xScene* sc) NONMATCH("https://decomp.me/scratch/ztWmC")
{
    motion->flags = motion->asset->flags;
    motion->t = 0.0f;

    if (motion->type == k_XENTMOTIONTYPE_ER) {
        motion->er.state = 0;
    } else if (motion->type == k_XENTMOTIONTYPE_MP) {
        motion->mp.src = (xMovePoint*)xSceneResolvID(sc, motion->asset->mp.mp_id);
        xEntMPSetSpeed(motion, motion->asset->mp.speed);
        xEntMPGetNext(motion, NULL, sc);
        xVec3Copy((xVec3*)&motion->owner->model->Mat->pos, xMovePointGetPos(motion->mp.src));
    } else if (motion->type == k_XENTMOTIONTYPE_ORBIT) {
        xVec3Copy(&motion->orb.orig, (xVec3*)&motion->owner->model->Mat->pos);
    } else if (motion->type == k_XENTMOTIONTYPE_PEND) {
        xEntMotionPenData* aspen = &motion->asset->pen;
        F32 dangle;
        xMat3x3 pshrot;
        xVec3 totop, pshtotop, pshdelta;
        xMat3x3* modlrot = (xMat3x3*)motion->owner->model->Mat;
        xVec3* modlpos = (xVec3*)&motion->owner->model->Mat->pos;
        
        dangle = aspen->range * isin(aspen->phase);
        xMat3x3Rot(&pshrot, &modlrot->at, dangle);

        totop.x = 0.0f;
        totop.y = aspen->len;
        totop.z = 0.0f;
        xMat3x3RMulVec(&pshtotop, &pshrot, &totop);

        xVec3Sub(&pshdelta, &totop, &pshtotop);
        
        xMat3x3Mul(modlrot, modlrot, &pshrot);
        xVec3AddTo(modlpos, &pshdelta);
        xVec3Add(&motion->pen.top, modlpos, &pshtotop);
        xMat4x3Copy(&motion->pen.omat, (xMat4x3*)modlrot);
    } else if (motion->type == k_XENTMOTIONTYPE_MP) { // BUG: unreachable, MP case already handled above
        if (motion->asset->flags & k_XENTMOTION_0x1) {
            xQuatFromMat(&motion->mp.aquat, (xMat3x3*)motion->owner->model->Mat);
            xQuatCopy(&motion->mp.bquat, &motion->mp.aquat);
        }
        motion->tmr = 0.0f;
    } else if (motion->type == k_XENTMOTIONTYPE_MECH) {
        xEnt* ownr = motion->owner;
        xEntMotionMechData* mkasst = &motion->asset->mech;
        xEntMechData* mech = &motion->mech;
        
        if (ownr && ownr->frame && ownr->model) {
            xVec3Copy(&mech->apos, (xVec3*)&ownr->model->Mat->pos);
            if (mkasst->sld_axis == 0) {
                xVec3Copy(&mech->dir, (xVec3*)&ownr->model->Mat->right);
            } else if (mkasst->sld_axis == 1) {
                xVec3Copy(&mech->dir, (xVec3*)&ownr->model->Mat->up);
            } else {
                xVec3Copy(&mech->dir, (xVec3*)&ownr->model->Mat->at);
            }
            xVec3SMul(&mech->bpos, &mech->dir, mkasst->sld_dist);
            xVec3AddTo(&mech->bpos, &mech->apos);
            mech->ss = mkasst->sld_dist / (mkasst->sld_tm - 0.5f * (mkasst->sld_acc_tm + mkasst->sld_dec_tm));
            mech->tsfd = mkasst->sld_tm - mkasst->sld_dec_tm;
            mech->tsbd = mkasst->sld_tm - mkasst->sld_acc_tm;

            if (mkasst->rot_axis == 0) {
                mech->rotptr = &ownr->frame->rot.axis.y;
            } else if (mkasst->rot_axis == 1) {
                mech->rotptr = &ownr->frame->rot.axis.x;
            } else {
                mech->rotptr = &ownr->frame->rot.axis.z;
            }
            mech->arot = *mech->rotptr;
            F32 drot = xdeg2rad(mkasst->rot_dist);
            mech->brot = xAngleClamp(mech->arot + drot);
            mech->sr = drot / (mkasst->rot_tm - 0.5f * (mkasst->rot_acc_tm + mkasst->rot_dec_tm));
            mech->trfd = mkasst->rot_tm - mkasst->rot_dec_tm;
            mech->trbd = mkasst->rot_tm - mkasst->rot_acc_tm;
            if (mkasst->type == k_XENTMOTIONMECH_SLIDE ||
                mkasst->type == k_XENTMOTIONMECH_SLIDE_ROT ||
                mkasst->type == k_XENTMOTIONMECH_SLIDE_THEN_ROT) {
                mech->state = 0;
            } else {
                mech->state = 1;
            }
            
            motion->tmr = 0.0f;
        }
    }
}

void xEntMotionMove(xEntMotion* motion, xScene* sc, F32 dt, xEntFrame* frame)
{
    if (motion->flags & k_XENTMOTION_STOPPED) return;

    if (motion->type == k_XENTMOTIONTYPE_ER) {
        xEntERMove(motion, sc, dt, frame);
    } else if (motion->type == k_XENTMOTIONTYPE_ORBIT) {
        xEntOrbitMove(motion, sc, dt, frame);
    } else if (motion->type == k_XENTMOTIONTYPE_MP) {
        xEntMPMove(motion, sc, dt, frame);
    } else if (motion->type == k_XENTMOTIONTYPE_PEND) {
        xEntPenMove(motion, sc, dt, frame);
    } else if (motion->type == k_XENTMOTIONTYPE_MECH) {
        xEntMechMove(motion, sc, dt, frame);
    } else {
        frame->mode = 0;
    }

    motion->t += dt;
}

static void xEntERMove(xEntMotion* motion, xScene*, F32 dt, xEntFrame* frame) NONMATCH("https://decomp.me/scratch/7A2wR")
{
    F32 newt = motion->t - motion->er.p;
    if (newt > 0.0f) motion->t = newt;

    if (xEntERIsExtending(motion)) {
        F32 rem = motion->er.et - motion->t;
        F32 scale = xmin(rem, dt) / motion->er.et;
        if (rem < dt) {
            xVec3Sub(&frame->dpos, &motion->er.b, &frame->mat.pos);
        } else {
            xVec3Sub(&frame->dpos, &motion->er.b, &motion->er.a);
            xVec3SMulBy(&frame->dpos, scale);
        }
        frame->mode = 2;
        motion->er.state = 0;
    } else if (xEntERIsExtended(motion)) {
        motion->er.state = 1;
    } else if (xEntERIsRetracting(motion)) {
        F32 rem = motion->er.ert - motion->t;
        F32 scale = xmin(rem, dt) / motion->er.rt;
        if (rem < dt) {
            xVec3Sub(&frame->dpos, &motion->er.a, &frame->mat.pos);
        } else {
            xVec3Sub(&frame->dpos, &motion->er.a, &motion->er.b);
            xVec3SMulBy(&frame->dpos, scale);
        }
        frame->mode = 2;
        motion->er.state = 2;
    } else if (xEntERIsRetracted(motion)) {
        motion->er.state = 3;
    }
}

static void xEntOrbitMove(xEntMotion* motion, xScene*, F32 dt, xEntFrame* frame) NONMATCH("https://decomp.me/scratch/PfdLw")
{
    F32 th = 0.5f * dt;
    F32 tf = motion->t + th;
    F32 u = motion->orb.w * tf;
    F32 s = isin(motion->orb.w * th);
    
    F32 newt = motion->t - motion->orb.p;
    if (newt > 0.0f) motion->t = newt;
    
    F32 rem = -newt;
    if (rem < dt && rem > 0.0f) {
        xVec3Sub(&frame->dpos, &motion->orb.orig, &frame->mat.pos);
    } else {
        frame->dpos.x = 2.0f * s * motion->orb.a * isin(u);
        frame->dpos.y = 0.0f;
        frame->dpos.z = -2.0f * s * motion->orb.b * icos(u);
    }

    frame->mode = 2;
}

static void xEntMPGetNext(xEntMotion* motion, xMovePoint* prev, xScene*) NONMATCH("https://decomp.me/scratch/AXMn3")
{
    xEntMPData* mp = &motion->mp;
    xVec3 tempdir;
    xMat3x3 mat;

    mp->spl = NULL;
    mp->curdist = 0.0f;
    mp->dist = xMovePointGetNext(mp->src, prev, &mp->dest, &tempdir);

    if (mp->dest == mp->src) {
        xEntMotionStop(motion);
        return;
    }

    if (!mp->dest) {
        xEntMotionStop(motion);
        return;
    }

    if (motion->asset->flags & k_XENTMOTION_0x1) {
        xQuatFromMat(&mp->aquat, (xMat3x3*)motion->owner->model->Mat);
        xVec3Inv(&tempdir, &tempdir);
        xMat3x3LookVec(&mat, &tempdir);
        xQuatFromMat(&mp->bquat, &mat);
    }

    if (mp->dest->spl) {
        mp->spl = mp->dest->spl;
        while (mp->dest->asset->bezIndex != 0) {
            mp->dest = mp->dest->nodes[0];
        }
        mp->dist = xSpline3_ArcTotal(mp->spl);
    }
}

static void xEntMPMove(xEntMotion* motion, xScene* sc, F32 dt, xEntFrame* frame)
{
    xEntMPData* mp = &motion->mp;
    
    if (mp->dest == mp->src || !mp->dest) return;

    if (motion->tmr > 0.0f) {
        motion->tmr -= dt;
        return;
    }

    frame->mode = 2;
    
    F32 newdist = mp->curdist + mp->speed * dt;
    if (newdist >= mp->dist) {
        frame->dpos.x = mp->dest->pos->x - frame->mat.pos.x;
        frame->dpos.y = mp->dest->pos->y - frame->mat.pos.y;
        frame->dpos.z = mp->dest->pos->z - frame->mat.pos.z;

        if (motion->asset->mp.flags & 0x1) {
            xEntMotionStop(motion);
        }

        xMovePoint* prev = mp->src;
        mp->src = mp->dest;
        xEntMPGetNext(motion, prev, sc);
        
        motion->tmr = mp->src->asset->delay;
        return;
    }

    mp->curdist = newdist;

    F32 qdot;
    xVec3 tgt, dir, bank;
    xQuat quat, qold;
    xMat3x3 tmpmat;
    if (mp->spl) {
        F32 u = xSpline3_EvalArcApprox(mp->spl, newdist, 0, &tgt);

        if (motion->asset->flags & k_XENTMOTION_0x1) {
            xSpline3_EvalSeg(mp->spl, u, 1, &dir);
            if (motion->asset->use_banking == 1) {
                xSpline3_EvalSeg(mp->spl, u, 2, &bank);
            }
            
            if (xVec3Length2(&dir) < 0.000001f) {
                if (u < 0.1f) u += 0.01f;
                else u -= 0.01f;
                xSpline3_EvalSeg(mp->spl, u, 1, &dir);
                if (motion->asset->use_banking == 1) {
                    xSpline3_EvalSeg(mp->spl, u, 2, &bank);
                }
            }

            if (motion->asset->use_banking != 1) {
                xVec3Inv(&dir, &dir);
                xMat3x3LookVec(&tmpmat, &dir);
            } else {
                xVec3 gravity = { 0.0f, -sc->gravity, 0.0f };
                
                bank = bank * mp->speed * 0.01f + gravity;
                dir.normalize();
                bank -= dir * bank.dot(dir);
                bank.normalize();
                
                tmpmat.at = dir;
                tmpmat.right = bank.cross(dir);
                tmpmat.up = bank;
            }
    
            xQuatFromMat(&quat, &tmpmat);
            xQuatFromMat(&qold, &frame->mat);
            
            qdot = xQuatDot(&quat, &qold);
            if (qdot < 0.0f) {
                xQuatFlip(&quat, &quat);
                qdot = -qdot;
            }
            if (qdot > 1.0f) {
                qdot = 1.0f;
            }
            qdot = 2.0f * xacos(qdot);
    
            if (qdot <= PI*dt) {
                frame->mode |= 0x10;
                (xMat3x3)frame->mat = tmpmat;
            } else {
                qdot = PI*dt / qdot;
                xQuatSlerp(&quat, &qold, &quat, qdot);
                xQuatNormalize(&quat, &quat);
                xQuatToMat(&quat, &frame->mat);
            }
        }
    } else {
        xVec3Lerp(&tgt, mp->src->pos, mp->dest->pos, mp->curdist / mp->dist);

        if ((motion->asset->flags & k_XENTMOTION_0x1) && !xQuatEquals(&mp->aquat, &mp->bquat)) {
            xQuatSlerp(&quat, &mp->aquat, &mp->bquat, xmin(1.0f, 2.0f * (mp->curdist / mp->dist)));
            xQuatToMat(&quat, &frame->mat);
            frame->mode |= 0x10;
        }
    }
    
    frame->dpos.x = tgt.x - frame->mat.pos.x;
    frame->dpos.y = tgt.y - frame->mat.pos.y;
    frame->dpos.z = tgt.z - frame->mat.pos.z;
}

static void xEntPenMove(xEntMotion* motion, xScene*, F32 dt, xEntFrame* frame)
{
    xEntPenData* pen = &motion->pen;
    xEntMotionPenData* aspen = &motion->asset->pen;
    F32 th = 0.5f * dt;
    F32 tf = motion->t + th;
    F32 u = tf * pen->w + aspen->phase;
    F32 v = th * pen->w;
    F32 dangle = 2.0f * aspen->range * isin(v) * icos(u);
    
    F32 newt = motion->t - aspen->period;
    if (newt > 0.0f) motion->t = newt;
    
    F32 rem = -newt;
    if (rem < dt && rem > 0.0f) {
        xMat4x3Copy(&frame->mat, &motion->pen.omat);
        frame->mode = 0x11;
    } else {
        xMat4x3 delta_mat;
        xMat4x3Rot(&delta_mat, &frame->mat.at, dangle, &pen->top);
        xMat4x3Mul(&frame->mat, &frame->mat, &delta_mat);
        frame->mode = 0x41;
    }
}

static void xEntMechMove(xEntMotion* motion, xScene* sc, F32 dt, xEntFrame* frame)
{
    xEntMechData* mech = &motion->mech;
    xEntMotionMechData* mkasst = &motion->asset->mech;
    U32 last = 0;

    frame->mode = 0;

    if (mech->state == 0) {
        last = xEntSldMove(motion, sc, dt, frame);
        if (mkasst->type == k_XENTMOTIONMECH_SLIDE_ROT) {
            xEntRotMove(motion, sc, dt, frame);
        }
        if (last) {
            if (mkasst->type == k_XENTMOTIONMECH_SLIDE_THEN_ROT) {
                mech->state = 1;
            } else if (mkasst->flags & k_XENTMOTIONMECH_RETURNS) {
                mech->state = 2;
                mech->ss = -mech->ss;
                mech->sr = -mech->sr;
            } else if (mkasst->flags & k_XENTMOTIONMECH_ONCE) {
                mech->state = 6;
            } else {
                mech->state = 5;
            }
        }
    } else if (mech->state == 1) {
        last = xEntRotMove(motion, sc, dt, frame);
        if (last) {
            if (mkasst->type == k_XENTMOTIONMECH_ROT_THEN_SLIDE) {
                mech->state = 0;
            } else if (mkasst->flags & k_XENTMOTIONMECH_RETURNS) {
                mech->state = 2;
                mech->ss = -mech->ss;
                mech->sr = -mech->sr;
            } else if (mkasst->flags & k_XENTMOTIONMECH_ONCE) {
                mech->state = 6;
            } else if (mkasst->post_ret_delay != 0.0f) {
                mech->state = 5;
            }
        }
    } else if (mech->state == 2) {
        if (motion->tmr >= mkasst->ret_delay) {
            if (mkasst->type == k_XENTMOTIONMECH_SLIDE ||
                mkasst->type == k_XENTMOTIONMECH_SLIDE_ROT ||
                mkasst->type == k_XENTMOTIONMECH_ROT_THEN_SLIDE) {
                mech->state = 3;
            } else {
                mech->state = 4;
            }
            last = 1;
        }
    } else if (mech->state == 3) {
        last = xEntSldMove(motion, sc, dt, frame);
        if (mkasst->type == k_XENTMOTIONMECH_SLIDE_ROT) {
            xEntRotMove(motion, sc, dt, frame);
        }
        if (last) {
            if (mkasst->type == k_XENTMOTIONMECH_ROT_THEN_SLIDE) {
                mech->state = 4;
            } else if (mkasst->flags & k_XENTMOTIONMECH_ONCE) {
                mech->state = 7;
            } else {
                mech->state = 5;
            }
        }
    } else if (mech->state == 4) {
        last = xEntRotMove(motion, sc, dt, frame);
        if (last) {
            if (mkasst->type == k_XENTMOTIONMECH_SLIDE_THEN_ROT) {
                mech->state = 3;
            } else if (mkasst->flags & k_XENTMOTIONMECH_ONCE) {
                mech->state = 7;
            } else {
                mech->state = 5;
            }
        }
    } else if (mech->state == 5) {
        if (motion->tmr >= mkasst->post_ret_delay) {
            xEntMotionReset(motion, sc);
            xEntMotionRun(motion);
            last = 1;
        }
    }

    if (last) {
        motion->tmr = 0.0f;
    } else {
        motion->tmr += dt;
    }
}

static U32 xEntSldMove(xEntMotion* motion, xScene*, F32 dt, xEntFrame* frame) NONMATCH("https://decomp.me/scratch/U5Lyx")
{
    xEntMechData* mech = &motion->mech;
    xEntMotionMechData* mkasst = &motion->asset->mech;
    U32 last;
    F32 tmradj = motion->tmr + 0.5f * dt;
    F32 rem = mkasst->sld_tm - tmradj;

    if (motion->tmr + dt >= mkasst->sld_tm) {
        if (mech->state == 0) {
            xVec3Sub(&frame->dpos, &mech->bpos, &frame->mat.pos);
        } else {
            xVec3Sub(&frame->dpos, &mech->apos, &frame->mat.pos);
        }
        last = 1;
    } else {
        F32 speed;
        if (mech->state == 0) {
            if (tmradj < mkasst->sld_acc_tm && mkasst->sld_acc_tm > 0.00001f) {
                speed = mech->ss * tmradj / mkasst->sld_acc_tm;
            } else if (tmradj > mech->tsfd && mkasst->sld_dec_tm > 0.00001f) {
                speed = mech->ss * rem / mkasst->sld_dec_tm;
            } else {
                speed = mech->ss;
            }
        } else {
            if (tmradj < mkasst->sld_dec_tm && mkasst->sld_dec_tm > 0.00001f) {
                speed = mech->ss * tmradj / mkasst->sld_dec_tm;
            } else if (tmradj > mech->tsbd && mkasst->sld_acc_tm > 0.00001f) {
                speed = mech->ss * rem / mkasst->sld_acc_tm;
            } else {
                speed = mech->ss;
            }
        }
        xVec3SMul(&frame->dpos, &mech->dir, speed * dt);
        last = 0;
    }
    
    frame->mode |= 0x2;
    return last;
}

static U32 xEntRotMove(xEntMotion* motion, xScene*, F32 dt, xEntFrame* frame) NONMATCH("https://decomp.me/scratch/SPQg7")
{
    xEntMechData* mech = &motion->mech;
    xEntMotionMechData* mkasst = &motion->asset->mech;
    U32 last;
    F32 tmradj = motion->tmr + 0.5f * dt;
    F32 rem = mkasst->rot_tm - tmradj;

    if (motion->tmr + dt >= mkasst->rot_tm) {
        if (mech->state == 1 ||
            (mkasst->type == k_XENTMOTIONMECH_SLIDE_ROT && mech->state == 0)) {
            *mech->rotptr = mech->brot;
        } else {
            *mech->rotptr = mech->arot;
        }
        last = 1;
    } else {
        F32 speed;
        if (mech->state == 1 ||
            (mkasst->type == k_XENTMOTIONMECH_SLIDE_ROT && mech->state == 0)) {
            if (tmradj < mkasst->rot_acc_tm && mkasst->rot_acc_tm > 0.00001f) {
                speed = mech->sr * tmradj / mkasst->rot_acc_tm;
            } else if (tmradj > mech->trfd && mkasst->rot_dec_tm > 0.00001f) {
                speed = mech->sr * rem / mkasst->rot_dec_tm;
            } else {
                speed = mech->sr;
            }
        } else {
            if (tmradj < mkasst->rot_dec_tm && mkasst->rot_dec_tm > 0.00001f) {
                speed = mech->sr * tmradj / mkasst->rot_dec_tm;
            } else if (tmradj > mech->trbd && mkasst->rot_acc_tm > 0.00001f) {
                speed = mech->sr * rem / mkasst->rot_acc_tm;
            } else {
                speed = mech->sr;
            }
        }
        *mech->rotptr = xAngleClamp(*mech->rotptr + speed * dt);
        last = 0;
    }

    xMat3x3Euler(&frame->mat, frame->rot.axis.x, frame->rot.axis.y, frame->rot.axis.z);
    
    xEnt* ownr = motion->owner;
    if (ownr) {
        xEntAsset* easst = ownr->asset;
        xVec3SMulBy(&frame->mat.right, easst->scale.x);
        xVec3SMulBy(&frame->mat.up, easst->scale.y);
        xVec3SMulBy(&frame->mat.at, easst->scale.z);
    }

    frame->mode |= 0x10;
    return last;
}

void xEntMechForward(xEntMotion* motion) NONMATCH("https://decomp.me/scratch/q1HR8")
{
    xEntMechData* mech = &motion->mech;
    xEntMotionMechData* mkasst = &motion->asset->mech;

    xEntMotionRun(motion);

    if (mech->state == 0) {
    } else if (mech->state == 1) {
    } else if (mech->state == 2) {
    } else if (mech->state == 3) {
        mech->ss = -mech->ss;
        mech->sr = -mech->sr;
        motion->tmr = mkasst->sld_tm - motion->tmr;
        mech->state = 0;
    } else if (mech->state == 4) {
        mech->ss = -mech->ss;
        mech->sr = -mech->sr;
        motion->tmr = mkasst->rot_tm - motion->tmr;
        mech->state = 1;
    } else if (mech->state == 5) {
    } else if (mech->state == 6) {
    } else if (mech->state == 7) {
        mech->ss = -mech->ss;
        mech->sr = -mech->sr;
        motion->tmr = 0.0f;
        if (mkasst->type == k_XENTMOTIONMECH_SLIDE ||
            mkasst->type == k_XENTMOTIONMECH_SLIDE_ROT ||
            mkasst->type == k_XENTMOTIONMECH_ROT_THEN_SLIDE) {
            mech->state = 0;
        } else {
            mech->state = 1;
        }
    }
}

void xEntMechReverse(xEntMotion* motion) NONMATCH("https://decomp.me/scratch/KMDlB")
{
    xEntMechData* mech = &motion->mech;
    xEntMotionMechData* mkasst = &motion->asset->mech;

    xEntMotionRun(motion);

    if (mech->state == 0) {
        mech->ss = -mech->ss;
        mech->sr = -mech->sr;
        motion->tmr = mkasst->sld_tm - motion->tmr;
        mech->state = 3;
    } else if (mech->state == 1) {
        mech->ss = -mech->ss;
        mech->sr = -mech->sr;
        motion->tmr = mkasst->rot_tm - motion->tmr;
        mech->state = 4;
    } else if (mech->state == 2) {
    } else if (mech->state == 3) {
    } else if (mech->state == 4) {
    } else if (mech->state == 5) {
    } else if (mech->state == 6) {
        mech->ss = -mech->ss;
        mech->sr = -mech->sr;
        motion->tmr = 0.0f;
        if (mkasst->type == k_XENTMOTIONMECH_SLIDE ||
            mkasst->type == k_XENTMOTIONMECH_SLIDE_ROT ||
            mkasst->type == k_XENTMOTIONMECH_ROT_THEN_SLIDE) {
            mech->state = 3;
        } else {
            mech->state = 4;
        }
    } else if (mech->state == 7) {
    }
}

void xEntMotionTranslate(xEntMotion* motion, const xVec3* dpos, xMat4x3* dmat)
{
    if (dmat) {
        if (motion->type == k_XENTMOTIONTYPE_ER) {
            xMat4x3Toworld(&motion->er.a, dmat, &motion->er.a);
            xMat4x3Toworld(&motion->er.b, dmat, &motion->er.b);
        } else if (motion->type == k_XENTMOTIONTYPE_ORBIT) {
            xMat4x3Toworld(&motion->orb.orig, dmat, &motion->orb.orig);
            xMat4x3Toworld(&motion->orb.c, dmat, &motion->orb.c);
        } else if (motion->type == k_XENTMOTIONTYPE_MP) {
        } else if (motion->type == k_XENTMOTIONTYPE_PEND) {
            xMat4x3Toworld(&motion->pen.top, dmat, &motion->pen.top);
            xMat4x3Mul(&motion->pen.omat, &motion->pen.omat, dmat);
        } else if (motion->type == k_XENTMOTIONTYPE_MECH) {
            xMat4x3Toworld(&motion->mech.apos, dmat, &motion->mech.apos);
            xMat4x3Toworld(&motion->mech.bpos, dmat, &motion->mech.bpos);
            xMat3x3RMulVec(&motion->mech.dir, dmat, &motion->mech.dir);
        }
    } else {
        if (motion->type == k_XENTMOTIONTYPE_ER) {
            xVec3AddTo(&motion->er.a, dpos);
            xVec3AddTo(&motion->er.b, dpos);
        } else if (motion->type == k_XENTMOTIONTYPE_ORBIT) {
            xVec3AddTo(&motion->orb.orig, dpos);
            xVec3AddTo(&motion->orb.c, dpos);
        } else if (motion->type == k_XENTMOTIONTYPE_MP) {
        } else if (motion->type == k_XENTMOTIONTYPE_PEND) {
            xVec3AddTo(&motion->pen.top, dpos);
            xVec3AddTo(&motion->pen.omat.pos, dpos);
        } else if (motion->type == k_XENTMOTIONTYPE_MECH) {
            xVec3AddTo(&motion->mech.apos, dpos);
            xVec3AddTo(&motion->mech.bpos, dpos);
        }
    }
}

void xEntMotionDebugInit(U16 num_xems) NONMATCH("https://decomp.me/scratch/5aXKG")
{
    if (num_xems) {
        xDebugModeAdd("DBG_XENTMOTION", xEntMotionDebugCB);
        dbg_num = 0;
        dbg_xems = (xEntMotion**)xMALLOC(num_xems * sizeof(xEntMotion*));
        dbg_num_allocd = num_xems;
        dbg_idx = 0;
    }
}

void xEntMotionDebugExit()
{
    dbg_num = 0;
    dbg_xems = NULL;
    dbg_num_allocd = 0;
    dbg_idx = -1;
}

static void xEntMotionDebugAdd(xEntMotion* xem) NONMATCH("https://decomp.me/scratch/q1uXS")
{
    if (dbg_num >= dbg_num_allocd) return;
    dbg_xems[dbg_num++] = xem;
}

static void xEntMotionDebugCB()
{
    if (dbg_idx == -1) return;
    if (dbg_num == 0) return;

    xEntMotion* xem = dbg_xems[dbg_idx];
    xEntMotionDebugWrite(xem);
    xEntMotionDebugDraw(xem);
    xEntMotionDebugIPad(xem);
}

static void xEntMotionDebugWrite(const xEntMotion* xem)
{
    char* gps;

    switch (xem->type) {
    case k_XENTMOTIONTYPE_ER:     gps = "extend/retract"; break;
    case k_XENTMOTIONTYPE_ORBIT:  gps = "orbital";        break;
    case k_XENTMOTIONTYPE_SPLINE: gps = "spline";         break;
    case k_XENTMOTIONTYPE_MP:     gps = "movepoint";      break;
    case k_XENTMOTIONTYPE_MECH:   gps = "mechanism";      break;
    case k_XENTMOTIONTYPE_PEND:   gps = "pendulum";       break;
    default:                      gps = "????";           break;
    }
    xprintf("type:             %s\n",           gps);
    xprintf("stopped:          %s\n",           xbtoa(xem->flags & k_XENTMOTION_STOPPED));
    xprintf("owner:            %s\n",           xem->owner ? xSceneID2Name(g_xSceneCur, xem->owner->id) : "null");
    xprintf("t:  %.3f   d: %.3f   tmr: %.3f\n", xem->t, xem->d, xem->tmr);

    switch (xem->type) {
    case k_XENTMOTIONTYPE_ER:
        xprintf("a:       <%.3f %.3f %.3f>\n", xem->er.a.x, xem->er.a.y, xem->er.a.z);
        xprintf("b:       <%.3f %.3f %.3f>\n", xem->er.b.x, xem->er.b.y, xem->er.b.z);
        xprintf("b-a:     <%.3f %.3f %.3f>\n", xem->asset->er.ext_dpos.x, xem->asset->er.ext_dpos.y, xem->asset->er.ext_dpos.z);
        xprintf("ext_tm:  %.3f\n",             xem->er.et);
        xprintf("ret_tm:  %.3f\n",             xem->er.rt);
        xprintf("wait_et: %.3f\n",             xem->er.wet);
        xprintf("wait_rt: %.3f\n",             xem->er.wrt);
        xprintf("period:  %.3f\n",             xem->er.p);
        break;
    case k_XENTMOTIONTYPE_ORBIT:
        xprintf("c:   <%.3f %.3f %.3f>\n", xem->orb.c.x, xem->orb.c.y, xem->orb.c.z);
        xprintf("a:   %.3f\n",             xem->orb.a);
        xprintf("b:   %.3f\n",             xem->orb.b);
        xprintf("p:   %.3f\n",             xem->orb.p);
        xprintf("w:   %.3f\n",             xem->orb.w);
        break;
    case k_XENTMOTIONTYPE_MP:
        xprintf("src-mp:           %s\n", xem->mp.src  ? xSceneID2Name(g_xSceneCur, xem->mp.src->id)  : "");
        xprintf("dest-mp:          %s\n", xem->mp.dest ? xSceneID2Name(g_xSceneCur, xem->mp.dest->id) : "");
        xprintf("dist:  %.3f\n",          xem->mp.dist);
        xprintf("speed: %.3f\n",          xem->mp.speed);
        break;
    case k_XENTMOTIONTYPE_MECH:
        switch (xem->asset->mech.type) {
        case k_XENTMOTIONMECH_SLIDE:          gps = "slide";          break;
        case k_XENTMOTIONMECH_ROT:            gps = "rot";            break;
        case k_XENTMOTIONMECH_SLIDE_ROT:      gps = "slide_rot";      break;
        case k_XENTMOTIONMECH_SLIDE_THEN_ROT: gps = "slide_then_rot"; break;
        case k_XENTMOTIONMECH_ROT_THEN_SLIDE: gps = "rot_then_slide"; break;
        }
        xprintf("type:             %s\n", gps);
        
        xprintf("returns:          %s",   xbtoa(xem->asset->mech.flags & k_XENTMOTIONMECH_RETURNS));
        if (xem->asset->mech.flags & k_XENTMOTIONMECH_RETURNS) {
            xprintf("   ret_delay: %.3f", xem->asset->mech.ret_delay);
        }
        xprintf("\n");
        
        xprintf("continuous:       %s",   xbtoa(!(xem->asset->mech.flags & k_XENTMOTIONMECH_ONCE)));
        if (!(xem->asset->mech.flags & k_XENTMOTIONMECH_ONCE)) {
            xprintf("   end_delay: %.3f", xem->asset->mech.post_ret_delay);
        }
        xprintf("\n");
        
        switch (xem->mech.state) {
        case 0:
            if (xem->asset->mech.type == k_XENTMOTIONMECH_SLIDE_ROT) {
                gps = "sliding + rotating forth";
            } else {
                gps = "sliding forth";
            }
            break;
        case 1: gps = "rotating forth";    break;
        case 2: gps = "waiting to return"; break;
        case 3:
            if (xem->asset->mech.type == k_XENTMOTIONMECH_SLIDE_ROT) {
                gps = "sliding + rotating back";
            } else {
                gps = "sliding back";
            }
            break;
        case 4: gps = "rotating back";          break;
        case 5: gps = "waiting to begin again"; break;
        case 6: gps = "done";                   break;
        case 7: gps = "undone";                 break;
        }
        xprintf("state:            %s\n", gps);

        if (xem->asset->mech.type != k_XENTMOTIONMECH_ROT) {
            xprintf("slide_axis:   %s",               xem->asset->mech.sld_axis == 0 ? "X" :
                                                      xem->asset->mech.sld_axis == 1 ? "Y" :
                                                                                       "Z");
            xprintf("       slide_dist:   %.3f\n",    xem->asset->mech.sld_dist);
            xprintf("slide_tm:     %.3f",             xem->asset->mech.sld_tm);
            xprintf("   slide_speed:  %.3f\n",        xem->mech.ss);
            xprintf("slide_acc_tm: %.3f",             xem->asset->mech.sld_acc_tm);
            xprintf("   slide_dec_tm: %.3f\n",        xem->asset->mech.sld_dec_tm);
            xprintf("slide_bd_tm:  %.3f",             xem->mech.tsbd);
            xprintf("   slide_fd_tm:  %.3f\n",        xem->mech.tsfd);
            xprintf("start_pos:  <%.3f %.3f %.3f>\n", xem->mech.apos.x, xem->mech.apos.y, xem->mech.apos.z);
            xprintf("end_pos:    <%.3f %.3f %.3f>\n", xem->mech.bpos.x, xem->mech.bpos.y, xem->mech.bpos.z);
            xprintf("dir:        <%.3f %.3f %.3f>\n", xem->mech.dir.x, xem->mech.dir.y, xem->mech.dir.z);
        }

        if (xem->asset->mech.type != k_XENTMOTIONMECH_SLIDE) {
            xprintf("rot_axis:   %s",            xem->asset->mech.rot_axis == 0 ? "X" :
                                                 xem->asset->mech.rot_axis == 1 ? "Y" :
                                                                                  "Z");
            xprintf("       rot_dist:   %.3f\n", xem->asset->mech.rot_dist);
            xprintf("rot_tm:     %.3f",          xem->asset->mech.rot_tm);
            xprintf("   rot_speed:  %.3f\n",     xrad2deg(xem->mech.sr));
            xprintf("rot_acc_tm: %.3f",          xem->asset->mech.rot_acc_tm);
            xprintf("   rot_dec_tm: %.3f\n",     xem->asset->mech.rot_dec_tm);
            xprintf("rot_bd_tm:  %.3f",          xem->mech.trbd);
            xprintf("   rot_fd_tm:  %.3f\n",     xem->mech.trfd);
            xprintf("arot:       %.3f\n",        xrad2deg(xem->mech.arot));
            xprintf("brot:       %.3f\n",        xrad2deg(xem->mech.brot));
        }
        
        break;
    case k_XENTMOTIONTYPE_PEND:
        xprintf("top:    <%.3f %.3f %.3f>\n", xem->pen.top.x, xem->pen.top.y, xem->pen.top.z);
        xprintf("length: %.3f\n",             xem->asset->pen.len);
        xprintf("period: %.3f\n",             xem->asset->pen.period);
        xprintf("phase:  %.3f\n",             xrad2deg(xem->asset->pen.phase));
        xprintf("range:  %.3f\n",             xrad2deg(xem->asset->pen.range));
        xprintf("w:      %.3f\n",             xem->pen.w);
        break;
    }
}

static void xEntMotionDebugDraw(const xEntMotion* xem) NONMATCH("https://decomp.me/scratch/j2sCX")
{
    if (xem->owner && xem->target) {
        xDrawSetColor(g_NEON_GREEN);
        xDrawLine(xEntGetPos(xem->owner), xEntGetPos(xem->target));
    }

    switch (xem->type) {
    case k_XENTMOTIONTYPE_ER:
        xDrawSetColor(g_NEON_RED);
        xDrawLine(&xem->er.a, &xem->er.b);
        break;
    case k_XENTMOTIONTYPE_ORBIT:
        if (xem->owner) {
            xDrawSetColor(g_NEON_RED);
            xDrawLine(&xem->orb.c, xEntGetPos(xem->owner));
        }
        break;
    case k_XENTMOTIONTYPE_MP:
    {
        xDrawSetColor(g_PIMP_GOLD);
        xMovePoint* xmp = xem->mp.dest;
        if (xmp) {
            for (U16 idx = 0; idx < xMovePointGetNumPoints(xmp); idx++) {
                xMovePoint* omp = xMovePointGetPoint(xmp, idx);
                if (omp != xem->mp.src) {
                    xDrawLine(xMovePointGetPos(xmp), xMovePointGetPos(omp));
                }
                for (U16 jdx = 0; jdx < xMovePointGetNumPoints(omp); jdx++) {
                    xMovePoint* pmp = xMovePointGetPoint(omp, jdx);
                    xDrawLine(xMovePointGetPos(omp), xMovePointGetPos(pmp));
                }
            }
        }
        if (xem->mp.src && xem->mp.dest) {
            xDrawSetColor(g_NEON_RED);
            xDrawLine(xMovePointGetPos(xem->mp.src), xMovePointGetPos(xem->mp.dest));
        }
        break;
    }
    case k_XENTMOTIONTYPE_MECH:
        xDrawSetColor(g_NEON_RED);
        xDrawLine(&xem->mech.apos, &xem->mech.bpos);
        break;
    case k_XENTMOTIONTYPE_PEND:
        if (xem->owner) {
            xDrawSetColor(g_NEON_RED);
            xDrawLine(&xem->pen.top, xEntGetPos(xem->owner));
        }
        break;
    }
}

static void xEntMotionDebugIPad(xEntMotion* xem) NONMATCH("https://decomp.me/scratch/jRZHT")
{
    if (gDebugPad->pressed & k_XPAD_RIGHT) {
        dbg_idx++;
        if (dbg_idx >= dbg_num) dbg_idx = 0;
    }

    if (gDebugPad->pressed & k_XPAD_LEFT) {
        dbg_idx--;
        if (dbg_idx < 0) dbg_idx = dbg_num - 1;
    }

    if (gDebugPad->pressed & k_XPAD_A) {
        if (xem->owner) {
            xEntReset(xem->owner);
        }
        xEntMotionReset(xem, g_xSceneCur);
    }

    if (gDebugPad->pressed & k_XPAD_B) {
        if (xEntMotionIsStopped(xem)) {
            xEntMotionRun(xem);
        } else {
            xEntMotionStop(xem);
        }
    }
}
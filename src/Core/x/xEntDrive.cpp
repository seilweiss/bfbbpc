#include "xEntDrive.h"

void xEntDriveInit(xEntDrive* drv, xEnt* driven) NONMATCH("https://decomp.me/scratch/ggI1v")
{
    if (!drv) return;

    drv->flags = 0;
    drv->driven = driven;
    drv->driver = NULL;
    drv->s = 0.0f;
    drv->tm = 0.0f;
    drv->tmr = 0.0f;
    drv->odriver = NULL;
    drv->os = 0.0f;
    drv->otm = 0.0f;
    drv->otmr = 0.0f;
}

void xEntDriveMount(xEntDrive* drv, xEnt* driver, F32 mt, const xCollis* coll) NONMATCH("https://decomp.me/scratch/2UuqP")
{
    if (!drv->driven || !drv->driven->frame || !driver || !driver->frame) {
        xEntDriveInit(drv, drv->driven);
        return;
    }

    drv->dloc = 0.0f;

    if (driver == drv->odriver && drv->os) {
        drv->driver = driver;
        driver->driving_count++;
        if (mt < 0.0f) {
            drv->s = 1.0f;
            drv->tmr = 0.0f;
        } else {
            drv->s = drv->os;
            drv->tmr = mt * (1.0f - drv->s);
        }
        drv->tm = mt;
        drv->odriver = NULL;
        drv->os = 0.0f;
        drv->otm = 0.0f;
        drv->otmr = 0.0f;
    } else {
        drv->driver = driver;
        driver->driving_count++;
        if (mt < 0.0f) {
            drv->s = 1.0f;
            drv->tmr = 0.0f;
        } else {
            drv->s = 0.0f;
            drv->tmr = mt;
        }
        drv->tm = mt;
    }

    if (drv->flags & 0x1) {
        xVec3 euler;
        xMat3x3 a_descaled;
        F32 dummy;
        
        xVec3NormalizeMacro(&a_descaled.right, &drv->driver->frame->mat.right, &dummy);
        xVec3NormalizeMacro(&a_descaled.up, &drv->driver->frame->mat.up, &dummy);
        xVec3NormalizeMacro(&a_descaled.at, &drv->driver->frame->mat.at, &dummy);
        
        xMat3x3GetEuler(&a_descaled, &euler);
        drv->yaw = euler.x;
    }

    if (coll && (coll->flags & k_HIT_CALC_TRI)) {
        drv->flags |= 0x2;
        (xCollis::tri_data)drv->tri = coll->tri;
        drv->tri.loc = xCollisTriHit(drv->tri, *driver->model);

        xMat4x3Tolocal(&drv->tri.loc, &drv->driver->frame->mat, &drv->tri.loc);

        drv->tri.coll = coll;
    }

    xVec3Copy(&drv->q, &drv->driven->frame->mat.pos);
    xMat4x3Tolocal(&drv->p, &drv->driver->frame->mat, &drv->q);
}

void xEntDriveDismount(xEntDrive* drv, F32 dmt) NONMATCH("https://decomp.me/scratch/oYJZY")
{
    if (!drv) return;
    
    xEnt* pDriver = drv->driver;
    if (!pDriver) return;

    if (!drv->driven || !drv->driven->frame || !pDriver->frame) {
        xEntDriveInit(drv, drv->driven);
        return;
    }

    drv->odriver = pDriver;
    drv->os = drv->s;
    drv->otm = dmt;
    drv->otmr = dmt * drv->os;

    if (drv->driver) {
        drv->driver->driving_count--;
    }

    drv->driver = NULL;
    drv->s = 0.0f;
    drv->tm = 0.0f;
    drv->tmr = 0.0f;
    drv->flags &= ~0x2;

    xVec3Copy(&drv->q, &drv->driven->frame->mat.pos);
    xMat4x3Tolocal(&drv->op, &drv->odriver->frame->mat, &drv->q);
}

void xEntDriveUpdate(xEntDrive* drv, xScene*, F32 dt, const xCollis*) NONMATCH("https://decomp.me/scratch/7tnhc")
{
    if (!drv) return;
    if (!drv->odriver && !drv->driver) return;

    if (!drv->driven || !drv->driven->frame || (drv->odriver && !drv->odriver->frame)) {
        xEntDriveInit(drv, drv->odriver);
        return;
    }

    if (drv->otmr > 0.0f) {
        drv->otmr -= dt;
        if (drv->otmr <= 0.0f) {
            drv->os = 0.0f;
            drv->otmr = 0.0f;
        } else {
            drv->os = drv->otmr / drv->otm;
        }
    }

    if (drv->tmr > 0.0f) {
        drv->tmr -= dt;
        if (drv->tmr <= 0.0f) {
            drv->s = 1.0f;
            drv->tmr = 0.0f;
        } else {
            drv->s = 1.0f - drv->tmr / drv->tm;
        }
    }

    if (!drv->os && !drv->s) return;

    if (drv->s && (drv->flags & 0x1)) {
        if (!drv->driver || !drv->driver->frame) {
            xEntDriveInit(drv, drv->driven);
            return;
        }

        xVec3 euler;
        xMat3x3 rot;
        xMat3x3 a_descaled;
        F32 dummy;

        xVec3NormalizeMacro(&a_descaled.right, &drv->driver->frame->mat.right, &dummy);
        xVec3NormalizeMacro(&a_descaled.up, &drv->driver->frame->mat.up, &dummy);
        xVec3NormalizeMacro(&a_descaled.at, &drv->driver->frame->mat.at, &dummy);
        
        xMat3x3GetEuler(&a_descaled, &euler);
        xMat3x3RotY(&rot, drv->s * (euler.x - drv->yaw));
        xMat3x3Mul(&drv->driven->frame->mat, &drv->driven->frame->mat, &rot);

        drv->yaw = euler.x;
    }

    drv->dloc = 0.0f;

    xVec3 newq;

    if (drv->os) {
        if (!drv->odriver || !drv->odriver->frame) {
            xEntDriveInit(drv, drv->driven);
            return;
        }

        xMat4x3Toworld(&newq, &drv->odriver->frame->mat, &drv->op);
        xVec3Sub(&drv->driven->frame->dpos, &newq, &drv->q);
        xVec3SMulBy(&drv->driven->frame->dpos, drv->os);
        xVec3AddTo(&drv->driven->frame->mat.pos, &drv->driven->frame->dpos);
        drv->dloc += drv->driven->frame->dpos;
    }

    if (drv->s) {
        if (!drv->driver || !drv->driver->frame) {
            xEntDriveInit(drv, drv->driven);
            return;
        }

        if (drv->flags & 0x2) {
            xModelInstance& m = *drv->driver->model;
            if (xModelAnimCollDirty(m)) {
                xModelAnimCollRefresh(m);
            }

            xVec3 world_loc, new_loc;
            xMat4x3Toworld(&world_loc, &drv->driver->frame->mat, &drv->tri.loc);
            new_loc = xCollisTriHit(drv->tri, m);
            drv->driven->frame->dpos = new_loc - world_loc;

            if (drv->tri.index != drv->tri.coll->tri.index ||
                !xeq(drv->tri.r, drv->tri.coll->tri.r, 0.1f) ||
                !xeq(drv->tri.d, drv->tri.coll->tri.d, 0.1f)) {
                (xCollis::tri_data)drv->tri = drv->tri.coll->tri;
            }

            xMat4x3 oldmat = *(xMat4x3*)m.Mat;
            *(xMat4x3*)m.Mat = drv->driver->frame->mat;

            drv->tri.loc = xCollisTriHit(drv->tri, m);
            xMat4x3Tolocal(&drv->tri.loc, &drv->driver->frame->mat, &drv->tri.loc);

            *(xMat4x3*)m.Mat = oldmat;
        } else {
            xMat4x3Toworld(&newq, &drv->driver->frame->mat, &drv->p);
            xVec3Sub(&drv->driven->frame->dpos, &newq, &drv->q);
        }

        drv->driven->frame->dpos *= drv->s;
        drv->dloc += drv->driven->frame->dpos;
        drv->driven->frame->mat.pos += drv->driven->frame->dpos;

        if (drv->driven->model) {
            *(xVec3*)&drv->driven->model->Mat->pos = drv->driven->frame->mat.pos;
        }
    }

    xVec3Copy(&drv->q, &drv->driven->frame->mat.pos);

    if (drv->os) {
        if (!drv->odriver || !drv->odriver->frame) {
            xEntDriveInit(drv, drv->driven);
            return;
        }

        xMat4x3Tolocal(&drv->op, &drv->odriver->frame->mat, &drv->q);
    }

    if (drv->s) {
        if (!drv->driver || !drv->driver->frame) {
            xEntDriveInit(drv, drv->driven);
            return;
        }

        xMat4x3Tolocal(&drv->p, &drv->driver->frame->mat, &drv->q);
    }
}
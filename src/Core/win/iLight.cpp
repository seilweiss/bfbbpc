#include "iLight.h"

#include <rwframesync.h>

RpWorld* gLightWorld;

void iLightInit(RpWorld* world)
{
    gLightWorld = world;
}

iLight* iLightCreate(iLight* light, U32 type) NONMATCH("https://decomp.me/scratch/QzILQ")
{
    switch (type) {
    case 1:
        light->hw = RpLightCreate(rpLIGHTPOINT);
        break;
    case 2:
        light->hw = RpLightCreate(rpLIGHTSPOT);
        break;
    case 3:
        light->hw = RpLightCreate(rpLIGHTSPOTSOFT);
        break;
    default:
        return NULL;
    }

    if (!light->hw) {
        return NULL;
    }
    
    RwFrame* frame = RwFrameCreate();
    if (!frame) {
        RpLightDestroy(light->hw);
        return NULL;
    }

    RpLightSetFlags(light->hw, rpLIGHTLIGHTATOMICS);
    RpLightSetFrame(light->hw, frame);

    RwFrameUpdateObjects(frame);

    light->type = type;
    light->sph.center.x = 0.0f;
    light->sph.center.y = 0.0f;
    light->sph.center.z = 0.0f;
    light->sph.r = 0.0f;
    light->color.r = 1.0f;
    light->color.g = 1.0f;
    light->color.b = 1.0f;
    light->color.a = 1.0f;
    light->dir.x = 0.0f;
    light->dir.y = 0.0f;
    light->dir.z = 1.0f;
    light->coneangle = 0.0f;

    return light;
}

void iLightModify(iLight* light, U32 flags)
{
    if (flags & (0x1 | 0x4)) {
        RwFrame* frame = RpLightGetFrame(light->hw);

        RwMatrix temp;
        (xVec3&)temp.right = g_O3;
        (xVec3&)temp.up = g_O3;
        (xVec3&)temp.at = light->dir;
        (xVec3&)temp.pos = light->sph.center;

        RwFrameTransform(frame, &temp, rwCOMBINEREPLACE);
    }

    if (flags & 0x2) {
        RpLightSetRadius(light->hw, light->sph.r);
    }

    if (flags & 0x8) {
        RpLightSetColor(light->hw, (RwRGBAReal*)&light->color);
    }

    if (flags & 0x10) {
        if (light->type == 2 || light->type == 3) {
            RpLightSetConeAngle(light->hw, light->coneangle);
        }
    }
}

void iLightSetColor(iLight* light, xFColor* col)
{
    RpLightSetColor(light->hw, (RwRGBAReal*)col);
}

void iLightSetPos(iLight* light, xVec3* pos)
{
    RwFrame* f = RpLightGetFrame(light->hw);
    RwMatrix* m = &f->modelling;

    m->pos.x = pos->x;
    m->pos.y = pos->y;
    m->pos.z = pos->z;

    RwMatrixUpdate(m);
    RwFrameUpdateObjects(f);
}

void iLightDestroy(iLight* light)
{
    light->type = 0;

    _rwFrameSyncDirty();

    RwFrame* frame = RpLightGetFrame(light->hw);
    if (frame) {
        RwFrameDestroy(frame);
    }

    RpLightDestroy(light->hw);
}

void iLightEnv(iLight* light, S32 env)
{
    U32 flags = 0;

    switch (env) {
    case 0:
        flags = 0;
        break;
    case 1:
        flags = rpLIGHTLIGHTWORLD;
        break;
    case 2:
        flags = rpLIGHTLIGHTATOMICS;
        break;
    case 3:
        flags = rpLIGHTLIGHTWORLD | rpLIGHTLIGHTATOMICS;
        break;
    }

    RpLightSetFlags(light->hw, flags);
}
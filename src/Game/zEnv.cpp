#include "zEnv.h"

#include "xstransvc.h"
#include "xClimate.h"
#include "zGlobals.h"
#include "zSurface.h"
#include "zLight.h"
#include "zEvent.h"

void zEnvInit(void* env, void* easset)
{
    zEnvInit((zEnv*)env, (xEnvAsset*)easset);
}

void zEnvInit(zEnv* env, xEnvAsset* easset)
{
    xBaseInit(env, easset);

    env->easset = easset;
    env->eventFunc = zEnvEventCB;
    if (env->linkCount) {
        env->link = (xLinkAsset*)(env->easset + 1);
    }

    globals.sceneCur->zen = env;
    globals.sceneCur->zen = env; // why assign twice?

    xSTAssetCountByType('BSP '); // unused

    U32 bufsize;
    void* buf;

    if (buf = xSTFindAsset(easset->bspAssetID, &bufsize)) {
        xEnvLoadBsp(globals.sceneCur->env, buf, bufsize, 0);
    }

    zSurfaceRegisterMapper(easset->bspMapperID);

    if (easset->bspCollisionAssetID) {
        if (buf = xSTFindAsset(easset->bspCollisionAssetID, &bufsize)) {
            xEnvLoadBsp(globals.sceneCur->env, buf, bufsize, 1);
        }
    }

    if (easset->bspFXAssetID) {
        if (buf = xSTFindAsset(easset->bspFXAssetID, &bufsize)) {
            xEnvLoadBsp(globals.sceneCur->env, buf, bufsize, 2);
        }
    }

    if (easset->bspCameraAssetID) {
        if (buf = xSTFindAsset(easset->bspCameraAssetID, &bufsize)) {
            xEnvLoadBsp(globals.sceneCur->env, buf, bufsize, 3);
        }
    }

    if (globals.sceneCur->env && globals.sceneCur->env->geom) {
        zLightResetAll(globals.sceneCur->env);
        iLightInit(globals.sceneCur->env->geom->world);
    }
}

void zEnvSetup(zEnv* env)
{
    xBaseSetup(env);
    iEnvLightingBasics(globals.sceneCur->env->geom, env->easset);
}

void zEnvStartingCamera(zEnv* env)
{
    xCamera* cam = (xCamera*)zSceneFindObject(env->easset->startCameraAssetID);
    if (cam) {
        zEntPlayer_StoreCheckPoint(
            &globals.player.ent.frame->mat.pos,
            globals.player.ent.frame->rot.angle,
            globals.camera.id);
    }
}

void zEnvRender(xEnv* env)
{
    RpWorld* world = env->geom->world;
    S32 num = RpWorldGetNumMaterials(env->geom->world);

    for (S32 i = 0; i < num; i++) {
        const xSurface* sp = zSurfaceGetSurface(i);
        zSurfaceProps* pp = (zSurfaceProps*)sp->moprops;
        if (pp &&
            pp->asset &&
            (pp->texanim_flags & 0x1)) {
            S32 tdx = 0;
            RpMaterial* mp = RpWorldGetMaterial(world, i);
            if (mp) {
                xGroup* g = (xGroup*)pp->texanim[tdx].group_ptr;
                if (g) {
                    U32 texid = xGroupGetItem(g, pp->texanim[tdx].group_idx);
                    RwTexture* texptr = (RwTexture*)xSTFindAsset(texid, NULL);
                    if (texptr) {
                        RpMaterialSetTexture(mp, texptr);
                    }
                }
            }
        }
    }

    xEnvRender(env);
}

void zEnvSave(zEnv* ent, xSerial* s)
{
    xBaseSave(ent, s);
}

void zEnvLoad(zEnv* ent, xSerial* s)
{
    xBaseLoad(ent, s);
}

void zEnvReset(zEnv* env)
{
    xBaseReset(env, env->easset);
}

S32 zEnvEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget)
{
    switch (toEvent) {
    case eEventReset:
        zEnvReset((zEnv*)to);
        break;
    case eEventSetSnow:
        xClimateSetSnow(toParam[0]);
        break;
    case eEventSetRain:
        xClimateSetRain(toParam[0]);
        break;
    case eEventWaveSetLinear:
    case eEventWaveSetRipple:
        break;
    }
    return 1;
}
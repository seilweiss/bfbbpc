#pragma once

#include "xJSP.h"
#include "xMath3.h"

struct xEnvAsset;

struct iEnv
{
    RpWorld* world;
    RpWorld* collision;
    RpWorld* fx;
    RpWorld* camera;
    xJSPHeader* jsp;
    RpLight* light[2];
    RwFrame* light_frame[2];
    S32 memlvl;
};

void iEnvLoad(iEnv* env, const void* data, U32, S32 dataType);
void iEnvFree(iEnv* env);
void iEnvDefaultLighting(iEnv* env);
void iEnvLightingBasics(iEnv* env, xEnvAsset* asset);
void iEnvRender(iEnv* env);
void iEnvEndRenderFX(iEnv* env);

inline void iEnvStartup() {}

inline xBBox* iEnvGetBBox(iEnv* env)
{
    return (xBBox*)&env->world->boundingBox;
}
#pragma once

#include "iEnv.h"

#include "xLightKit.h"

struct xEnv
{
    iEnv* geom;
    iEnv ienv;
    xLightKit* lightKit;
};

extern xEnv* gCurXEnv;

void xEnvLoadBsp(xEnv* env, const void* data, U32 datasize, S32 dataType);
void xEnvSetup(xEnv* env);
void xEnvFree(xEnv* env);
void xEnvRender(xEnv* env);

inline xBBox* xEnvGetBBox(xEnv* env)
{
    return iEnvGetBBox(env->geom);
}
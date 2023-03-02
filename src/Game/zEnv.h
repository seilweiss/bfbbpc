#pragma once

#include "xBase.h"
#include "xEnv.h"

struct xEnvAsset;

typedef struct _zEnv zEnv;
struct _zEnv : xBase
{
    xEnvAsset* easset;
};

void zEnvInit(void* env, void* easset);
void zEnvInit(zEnv* env, xEnvAsset* easset);
void zEnvSetup(zEnv* env);
void zEnvStartingCamera(zEnv* env);
void zEnvRender(xEnv* env);
void zEnvSave(zEnv* ent, xSerial* s);
void zEnvLoad(zEnv* ent, xSerial* s);
void zEnvReset(zEnv* env);
S32 zEnvEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget);
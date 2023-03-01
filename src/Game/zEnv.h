#pragma once

#include "xBase.h"
#include "xEnv.h"

struct xEnvAsset : xBaseAsset
{
    U32 bspAssetID;
    U32 startCameraAssetID;
    U32 climateFlags;
    F32 climateStrengthMin;
    F32 climateStrengthMax;
    U32 bspLightKit;
    U32 objectLightKit;
    F32 padF1;
    U32 bspCollisionAssetID;
    U32 bspFXAssetID;
    U32 bspCameraAssetID;
    U32 bspMapperID;
    U32 bspMapperCollisionID;
    U32 bspMapperFXID;
    F32 loldHeight;
};

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
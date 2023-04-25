#pragma once

#include "xEnt.h"

#define XENT_AUTOANIM_COUNT 5

struct xModelAssetParam;
struct zScene;

struct zEnt : xEnt
{
    xAnimTable* atbl;
};

typedef struct _ShadowParams
{
    U32 type;
    F32 at;
    F32 rad;
} zShadowParams;

extern U32 g_hash_xentanim[XENT_AUTOANIM_COUNT];
extern char* g_strz_xentanim[XENT_AUTOANIM_COUNT];
extern zShadowParams gShadowParams[];

void zEntInit(zEnt* ent, xEntAsset* asset, U32 type);
void zEntSetup(zEnt* ent);
void zEntSave(zEnt* ent, xSerial* s);
void zEntLoad(zEnt* ent, xSerial* s);
void zEntReset(zEnt* ent);
void zEntUpdate(zEnt* ent, zScene* scene, F32 elapsedSec);
void zEntEventAll(xBase* from, U32 fromEvent, U32 toEvent, F32* toParam);
void zEntEventAllOfType(xBase* from, U32 fromEvent, U32 toEvent, F32* toParam, U32 type);
void zEntEventAllOfType(U32 toEvent, U32 type);
xModelInstance* zEntRecurseModelInfo(void* info, xEnt* ent);
void zEntParseModelInfo(xEnt* ent, U32 assetID);
void zEntAnimEvent(zEnt* ent, U32 animEvent, const F32* animParam);
xAnimTable* xEnt_AnimTable_AutoEventSmall();
void zEntAnimEvent_AutoAnim(zEnt* ent, U32 animEvent, const F32* animParam);
xModelAssetParam* zEntGetModelParams(U32 assetID, U32* size);
char* zParamGetString(xModelAssetParam* param, U32 size, char* tok, char* def);
S32 zParamGetInt(xModelAssetParam* param, U32 size, const char* tok, S32 def);
S32 zParamGetInt(xModelAssetParam* param, U32 size, char* tok, S32 def);
F32 zParamGetFloat(xModelAssetParam* param, U32 size, const char* tok, F32 def);
F32 zParamGetFloat(xModelAssetParam* param, U32 size, char* tok, F32 def);
S32 zParamGetFloatList(xModelAssetParam* param, U32 size, const char* tok, S32 count, F32* def, F32* result);
S32 zParamGetFloatList(xModelAssetParam* param, U32 size, char* tok, S32 count, F32* def, F32* result);
S32 zParamGetVector(xModelAssetParam* param, U32 size, const char* tok, xVec3 def, xVec3* result);
S32 zParamGetVector(xModelAssetParam* param, U32 size, char* tok, xVec3 def, xVec3* result);
void zEntGetShadowParams(xEnt* ent, xVec3* center, F32* radius, xEntShadow::radius_enum rtype);
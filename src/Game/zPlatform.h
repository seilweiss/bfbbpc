#pragma once

#include "zEnt.h"
#include "xEntMotion.h"
#include "xEntDrive.h"

struct xPlatformAsset;

struct zPlatFMRunTime
{
    U32 flags;
    F32 tmrs[12];
    F32 ttms[12];
    F32 atms[12];
    F32 dtms[12];
    F32 vms[12];
    F32 dss[12];
};

struct zPlatform : zEnt
{
    xPlatformAsset* passet;
    xEntMotion motion;
    U16 state;
    U16 plat_flags;
    F32 tmr;
    S32 ctr;
    xMovePoint* src;
    xModelInstance* am;
    xModelInstance* bm;
    S32 moving;
    xEntDrive drv;
    zPlatFMRunTime* fmrt;
    F32 pauseMult;
    F32 pauseDelta;
};

enum en_ZPLATFORMTYPE
{
    ePlatformTypeER,
    ePlatformTypeOrbit,
    ePlatformTypeSpline,
    ePlatformTypeMP,
    ePlatformTypeMech,
    ePlatformTypePen,
    ePlatformTypeConvBelt,
    ePlatformTypeFalling,
    ePlatformTypeFR,
    ePlatformTypeBreakaway,
    ePlatformTypeSpringboard,
    ePlatformTypeTeeter,
    ePlatformTypePaddle,
    ePlatformTypeFM
};

void zPlatform_Init(void* plat, void* asset);
void zPlatform_Init(zPlatform* plat, xEntAsset* asset);
void zPlatform_Setup(zPlatform* plat, xScene* sc);
void zPlatform_Save(zPlatform* ent, xSerial* s);
void zPlatform_Load(zPlatform* ent, xSerial* s);
void zPlatform_Reset(zPlatform* plat, xScene* sc);
void zPlatform_PaddleStartRotate(xEnt* entplat, S32 direction, S32 stutter);
U32 zPlatform_PaddleCollide(xCollis* coll, const xVec3* hitsource, const xVec3* hitvel, U32 worldSpaceNorm);
void zPlatform_Update(xEnt* entplat, xScene* sc, F32 dt);
void zPlatform_Move(xEnt* entplat, xScene* s, F32 dt, xEntFrame* frame);
void zPlatform_Shake(zPlatform* plat, F32, F32 ampl, F32 freq);
void zPlatform_Mount(zPlatform* plat);
void zPlatform_Dismount(zPlatform* plat);
S32 zPlatformEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget);
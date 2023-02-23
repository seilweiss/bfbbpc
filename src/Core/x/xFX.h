#pragma once

#include "xMath3.h"

#include <rwcore.h>
#include <rpworld.h>

void xFXInit();
void xFX_SceneEnter(RpWorld* world);
void xFXPreAllocMatFX(RpClump* clump);
void xFXanimUVSetTranslation(const xVec3* trans);
void xFXanimUVSetScale(const xVec3* scale);
void xFXanimUVSetAngle(F32 angle);
void xFXanimUV2PSetTranslation(const xVec3* trans);
void xFXanimUV2PSetScale(const xVec3* scale);
void xFXanimUV2PSetAngle(F32 angle);
void xFXanimUV2PSetTexture(RwTexture* texture);
RpAtomic* xFXanimUVAtomicSetup(RpAtomic* atomic);
void xFXStartup();
void xFXShutdown();
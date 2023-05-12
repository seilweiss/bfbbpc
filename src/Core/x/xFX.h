#pragma once

#include "xMath3.h"
#include "xColor.h"

#include <rwcore.h>
#include <rpworld.h>

void xFXInit();
void xFX_SceneEnter(RpWorld* world);
void xFXPreAllocMatFX(RpClump* clump);
RpAtomic* xFXBubbleRender(RpAtomic* atomic);
void xFXanimUVSetTranslation(const xVec3* trans);
void xFXanimUVSetScale(const xVec3* scale);
void xFXanimUVSetAngle(F32 angle);
void xFXanimUV2PSetTranslation(const xVec3* trans);
void xFXanimUV2PSetScale(const xVec3* scale);
void xFXanimUV2PSetAngle(F32 angle);
void xFXanimUV2PSetTexture(RwTexture* texture);
RpAtomic* xFXanimUVAtomicSetup(RpAtomic* atomic);
U32 xFXStreakStart(F32 frequency, F32 alphaFadeRate, F32 alphaStart, U32 textureID, const xColor* edge_a, const xColor* edge_b, S32 taper);
void xFXStreakStop(U32 id);
void xFXStreakUpdate(U32 id, const xVec3* a, const xVec3* b);
void xFXStartup();
void xFXShutdown();
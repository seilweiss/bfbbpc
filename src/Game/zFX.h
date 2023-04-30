#pragma once

#include "xMath3.h"

struct xEnt;

void zFXGooEventSetWarb(xEnt* ent, const F32* warb);
void zFXGooEventSetFreezeDuration(xEnt* ent, F32 duration);
void zFXGooEventMelt(xEnt* ent);
void zFX_SpawnBubbleTrail(const xVec3* pos, U32 num, const xVec3* pos_rnd, const xVec3* vel_rnd);
void zFXPopOn(xEnt& ent, F32 rate, F32 time);
void zFXPopOff(xEnt& ent, F32 rate, F32 time);
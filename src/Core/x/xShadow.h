#pragma once

#include "types.h"

#include <rwcore.h>
#include <rpworld.h>

struct xEnt;

void xShadowInit();
void xShadowSetWorld(RpWorld* world);
void xShadow_ListAdd(xEnt* ent);
void xShadowManager_Add(xEnt* ent);
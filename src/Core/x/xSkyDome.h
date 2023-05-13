#pragma once

#include "types.h"

struct xEnt;

void xSkyDome_Setup();
void xSkyDome_AddEntity(xEnt* ent, S32 sortorder, S32 lockY);
void xSkyDome_Render();
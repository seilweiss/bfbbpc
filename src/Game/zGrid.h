#pragma once

#include "xGrid.h"

struct xEnt;
struct zScene;

extern xGrid colls_grid;
extern xGrid colls_oso_grid;
extern xGrid npcs_grid;

void zGridReset(zScene* s);
void zGridInit(zScene* s);
void zGridExit(zScene* s);
void zGridUpdateEnt(xEnt* ent);
#pragma once

#include "iCollideFast.h"

#include "xMath3.h"

void xCollideFastInit(xScene* sc);
U32 xRayHitsSphereFast(const xRay3* r, const xSphere* s);
U32 xRayHitsBoxFast(const xRay3* r, const xBox* b);
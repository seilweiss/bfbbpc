#pragma once

#include "xMath3.h"

#include <rwcore.h>

void xScrFxInit();
void xScrFxUpdate(RwCamera* cam, F32 dt);
void xScrFxLetterbox(S32 enable);
S32 xScrFXGlareAdd(xVec3* pos, F32 life, F32 intensity, F32 size, F32 r, F32 g, F32 b, F32 a, RwRaster* raster);
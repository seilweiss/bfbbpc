#pragma once

#include "types.h"

#include <rwcore.h>

void xScrFxInit();
void xScrFxUpdate(RwCamera* cam, F32 dt);
void xScrFxLetterbox(S32 enable);
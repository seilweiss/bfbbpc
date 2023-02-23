#pragma once

#include "types.h"

#include <rwcore.h>

void iScrFxCameraCreated(RwCamera* pCamera);
void iScrFxCameraEndScene(RwCamera* pCamera);
void iScrFxPostCameraEnd(RwCamera* pCamera);
S32 iScrFxCameraDestroyed(RwCamera*);
#pragma once

#include "types.h"

#include <rwcore.h>

void xRenderStateSetTexture(RwTexture* texture);
void xRenderStateSetSrcBlendMode(S32 xmode);
void xRenderStateSetDstBlendMode(S32 xmode);
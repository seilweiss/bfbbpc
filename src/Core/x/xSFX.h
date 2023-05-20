#pragma once

#include "xBase.h"

struct xSFXAsset;

struct xSFX : xBase
{
    xSFXAsset* asset;
    U32 sndID;
    F32 cachedOuterDistSquared;
};
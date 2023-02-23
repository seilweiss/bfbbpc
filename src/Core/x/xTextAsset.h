#pragma once

#include "types.h"

struct xTextAsset
{
    U32 len;
};

#define xTextGetString(asset) ((const char*)((xTextAsset*)(asset) + 1))
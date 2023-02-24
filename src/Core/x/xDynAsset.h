#pragma once

#include "xBase.h"

struct xDynAsset : xBaseAsset
{
    U32 type;
    U16 version;
    U16 handle;
};
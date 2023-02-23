#pragma once

#include "types.h"

#include <rwcore.h>

struct iFogParams
{
    RwFogType type;
    F32 start;
    F32 stop;
    F32 density;
    RwRGBA fogcolor;
    RwRGBA bgcolor;
    U8* table;
};
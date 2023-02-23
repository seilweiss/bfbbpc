#pragma once

#include "xBase.h"

struct xEnt;

struct xSurface : xBase
{
    U32 idx;
    U32 type;
    union
    {
        U32 mat_idx;
        xEnt* ent;
        void* obj;
    };
    F32 friction;
    U8 state;
    U8 pad[3];
    void* moprops;
};

#define k_XSURFACETYPE_MAT 0
#define k_XSURFACETYPE_ENT 1
#define k_XSURFACETYPE_OBJ 2
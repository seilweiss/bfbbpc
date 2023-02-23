#pragma once

#include "xEnt.h"

struct xEntDrive
{
    struct tri_data : xCollis::tri_data
    {
        xVec3 loc;
        F32 yaw;
        xCollis* coll;
    };

    U32 flags;
    F32 otm;
    F32 otmr;
    F32 os;
    F32 tm;
    F32 tmr;
    F32 s;
    xEnt* odriver;
    xEnt* driver;
    xEnt* driven;
    xVec3 op;
    xVec3 p;
    xVec3 q;
    F32 yaw;
    xVec3 dloc;
    tri_data tri;
};
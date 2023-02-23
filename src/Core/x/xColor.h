#pragma once

#include "iColor.h"

typedef iColor xColor;

extern xColor g_RED;
extern xColor g_GREEN;
extern xColor g_BLUE;
extern xColor g_CYAN;
extern xColor g_YELLOW;
extern xColor g_BLACK;
extern xColor g_WHITE;
extern xColor g_GRAY50;
extern xColor g_GRAY80;
extern xColor g_NEON_RED;
extern xColor g_NEON_GREEN;
extern xColor g_NEON_BLUE;
extern xColor g_PIMP_GOLD;
extern xColor g_ORANGE;
extern xColor g_LAVENDER;
extern xColor g_PINK;

inline void xColorInit(xColor* rToInit, U8 r, U8 g, U8 b, U8 a)
{
    rToInit->r = r;
    rToInit->g = g;
    rToInit->b = b;
    rToInit->a = a;
}

inline xColor xColorFromRGBA(U8 r, U8 g, U8 b, U8 a)
{
    xColor c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}
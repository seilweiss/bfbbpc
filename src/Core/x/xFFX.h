#pragma once

#include "xEnt.h"

typedef void(*xFFXDoEffectCallback)(xEnt*, xScene*, F32, void*);

struct xFFX
{
    U32 flags;
    xFFXDoEffectCallback doEffect;
    void* fdata;
    xFFX* next;
};

struct xFFXShakeState
{
    xVec3 disp;
    F32 dur;
    F32 freq;
    F32 tmr;
    F32 alpha;
    F32 lval;
    xFFXShakeState* next;
};

struct xFFXRotMatchState
{
    S32 lgrounded;
    xVec3 lfup;
    xVec3 lfat;
    xVec3 plfat;
    F32 tmr;
    F32 mrate;
    F32 tmatch;
    F32 rrate;
    F32 trelax;
    F32 max_decl;
    xFFXRotMatchState* next;
};

xFFX* xFFXAlloc();
S16 xFFXAddEffect(xEnt* ent, xFFX* f);
void xFFXApply(xEnt* ent, xScene* sc, F32 dt);
void xFFXShakeUpdateEnt(xEnt* ent, xScene* sc, F32 dt, void* fdata);
xFFXShakeState* xFFXShakeAlloc();
void xFFXShakeFree(xFFXShakeState* s);
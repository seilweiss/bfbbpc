#pragma once

#include "xBase.h"

struct xTimerAsset;
struct xScene;

struct xTimer : xBase
{
    xTimerAsset* tasset;
    U8 state;
    U8 runsInPause;
    U16 flags;
    F32 secondsLeft;
};

#define k_XTIMER_STATE_IDLE    0
#define k_XTIMER_STATE_RUN     1
#define k_XTIMER_STATE_EXPIRED 2

void xTimerInit(void* b, void* tasset);
void xTimerInit(xBase* b, xTimerAsset* tasset);
void xTimerReset(xTimer* t);
void xTimerSave(xTimer* ent, xSerial* s);
void xTimerLoad(xTimer* ent, xSerial* s);
S32 xTimerEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget);
void xTimerUpdate(xBase* to, xScene* sc, F32 dt);
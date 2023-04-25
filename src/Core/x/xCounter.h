#pragma once

#include "xBase.h"

struct xCounterAsset;

typedef struct _xCounter : xBase
{
    xCounterAsset* asset;
    S16 count;
    U8 state;
    U8 counterFlags;
} xCounter;

#define k_XCOUNTER_STATE_IDLE    0
#define k_XCOUNTER_STATE_EXPIRED 1

void xCounterInit();
void xCounterInit(void* b, void* asset);
void xCounterInit(xBase* b, xCounterAsset* asset);
void xCounterReset(xBase* b);
void xCounterSave(xCounter* ent, xSerial* s);
void xCounterLoad(xCounter* ent, xSerial* s);
S32 xCounterEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget);
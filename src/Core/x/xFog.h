#pragma once

#include "xBase.h"

struct xFogAsset;
struct xScene;

typedef struct _xFog : xBase
{
    xFogAsset* tasset;
} xFog;

void xFogClearFog();
void xFogInit(void* b, void* tasset);
void xFogInit(xBase* b, xFogAsset* tasset);
void xFogReset(xFog* t);
void xFogSave(xFog* ent, xSerial* s);
void xFogLoad(xFog* ent, xSerial* s);
S32 xFogEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget);
void xFogUpdate(xBase* to, xScene* sc, F32 dt);
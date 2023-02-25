#pragma once

#include "xBase.h"

extern char zEventLogBuf[20][256];

void zEntEvent(char* to, U32 toEvent);
void zEntEvent(U32 toID, U32 toEvent);
void zEntEvent(U32 toID, U32 toEvent, F32 toParam0, F32 toParam1, F32 toParam2, F32 toParam3);
void zEntEvent(xBase* to, U32 toEvent);
void zEntEvent(xBase* to, U32 toEvent, F32 toParam0, F32 toParam1, F32 toParam2, F32 toParam3);
void zEntEvent(xBase* to, U32 toEvent, const F32* toParam);
void zEntEvent(xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget);
void zEntEvent(xBase* from, xBase* to, U32 toEvent);
void zEntEvent(xBase* from, xBase* to, U32 toEvent, const F32* toParam);
void zEntEvent(xBase* from, U32 fromEvent, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget, S32 forceEvent);
#pragma once

#include "types.h"

#if defined(DEBUG) || defined(RELEASE)
void iDebugInit();
void iDebugUpdate();
void iDebugWriteProfile();
void iDebugVSync();
void iDebugStackTrace();
void iDebugIDEOutput(char* buf);

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define iDebugBreak() DebugBreak()
#else
#error "Unknown platform"
#endif
#else
#define iDebugInit()
#define iDebugUpdate()
#define iDebugWriteProfile()
#define iDebugVSync()
#define iDebugStackTrace()
#define iDebugIDEOutput(buf)
#define iDebugBreak()
#endif
#pragma once

#include "types.h"

#if defined(DEBUG) || defined(RELEASE)
void xOutCrit(const char* fmt, ...);
void xOutErr(char* name, const char* fmt, ...);
void xOutWarn(char* name, const char* fmt, ...);
void xOutInfo(char* name, const char* fmt, ...);
void xOutBabble(char* name, const char* fmt, ...);
#else
#define xOutCrit
#define xOutErr
#define xOutWarn
#define xOutInfo
#define xOutBabble
#endif
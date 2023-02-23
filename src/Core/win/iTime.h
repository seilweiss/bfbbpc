#pragma once

#include "types.h"

#include <time.h>

typedef S64 iTime;

extern iTime sStartupTime;

#define iTimeToSec(t) ((F32)t / CLOCKS_PER_SEC)
#define iTimeFromSec(s) ((iTime)(s * CLOCKS_PER_SEC))

void iTimeInit();
void iTimeExit();
iTime iTimeGet();
F32 iTimeDiffSec(iTime time);
F32 iTimeDiffSec(iTime t0, iTime t1);
F32 iTimeGetGame();
void iTimeGameAdvance(F32 elapsed);
void iTimeSetGame(F32 time);
void iProfileClear(U32 sceneID);
void iFuncProfileDump();
void iFuncProfileParse(char*, S32);
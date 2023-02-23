#pragma once

#include "types.h"

void zMenuInit(U32 theSceneID);
void zMenuExit();
void zMenuSetup();
U32 zMenuLoop();
S32 zMenuIsFirstBoot();
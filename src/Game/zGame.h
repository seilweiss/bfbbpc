#pragma once

#include "types.h"

void zGameInit(U32 theSceneID);
void zGameExit();
void zGameSetup();
void zGameLoop();
S32 zGameIsPaused();
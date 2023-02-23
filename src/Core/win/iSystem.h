#pragma once

#include "types.h"

void iVSync();
void iSystemInit(U32 options);
void iSystemExit();
void iSystemPollEvents();
U32 iSystemShouldQuit();
U32 iSystemIsFullScreen();
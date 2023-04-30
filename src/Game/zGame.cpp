#include "zGame.h"

#include "iSystem.h"

void zGameInit(U32 theSceneID) WIP
{
}

void zGameExit() WIP
{
}

void zGameSetup() WIP
{
}

void zGameLoop() WIP
{
    while (true) {
        iSystemPollEvents();
        if (iSystemShouldQuit()) break;
    }
}

S32 zGameIsPaused() WIP
{
    return 0;
}
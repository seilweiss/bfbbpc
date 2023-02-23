#pragma once

#include "xScene.h"

#include "zPortal.h"
#include "zEnt.h"
#include "zEnv.h"

struct zScene : xScene
{
    zPortal* pendingPortal;
    union
    {
        U32 num_ents;
        U32 num_base;
    };
    union
    {
        xBase** base;
        zEnt** ents;
    };
    U32 num_update_base;
    xBase** update_base;
    U32 baseCount[72];
    xBase* baseList[72];
    zEnv* zen;
};

void zSceneInit(U32 theSceneID, S32 reloadInProgress);
xBase* zSceneFindObject(U32 gameID);
void zSceneMemLvlChkCB();
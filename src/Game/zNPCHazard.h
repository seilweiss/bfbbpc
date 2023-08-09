#pragma once

#include "xAnim.h"

void zNPCHazard_Startup();
void zNPCHazard_Shutdown();
void zNPCHazard_ScenePrepare();
void zNPCHazard_SceneFinish();
void zNPCHazard_SceneReset();
void zNPCHazard_ScenePostInit();
void zNPCHazard_Timestep(F32 dt);
xAnimTable* ZNPC_AnimTable_HazardStd();
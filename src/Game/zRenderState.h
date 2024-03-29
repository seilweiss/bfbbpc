#pragma once

#include "types.h"

typedef enum _SDRenderState
{
    SDRS_Unknown,
    SDRS_Default,
    SDRS_OpaqueModels,
    SDRS_AlphaModels,
    SDRS_Bubble,
    SDRS_Projectile,
    SDRS_Font,
    SDRS_HUD,
    SDRS_Particles,
    SDRS_Lightning,
    SDRS_Streak,
    SDRS_SkyBack,
    SDRS_Environment,
    SDRS_Fill,
    SDRS_NPCVisual,
    SDRS_OOBFade,
    SDRS_OOBPlayerZ,
    SDRS_OOBPlayerAlpha,
    SDRS_OOBHand,
    SDRS_Glare,
    SDRS_Newsfish,
    SDRS_CruiseHUD,
    SDRS_DiscoFloorGlow,
    SDRS_Total = -1
} SDRenderState;

void zRenderStateInit();
SDRenderState zRenderStateCurrent();
void zRenderState(SDRenderState next);
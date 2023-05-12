#pragma once

#include "xPad.h"

typedef enum _tagSDRumbleType
{
    SDR_None,
    SDR_Test,
    SDR_Damage,
    SDR_Bounce,
    SDR_EventLight,
    SDR_EventMedium,
    SDR_EventHeavy,
    SDR_DustDestroyedObj,
    SDR_XploDestroyedObj,
    SDR_WebDestroyed,
    SDR_Tiki,
    SDR_LassoDestroy,
    SDR_DamageByEGen,
    SDR_BounceHit,
    SDR_CruiseBubbleLaunch,
    SDR_CruiseBubbleExplode,
    SDR_TeleportStart,
    SDR_Teleport,
    SDR_TeleportEject,
    SDR_Total
} SDRumbleType;

void zRumbleStart(S32 pad_id, SDRumbleType rumble_type);
void zRumbleStartDistance(S32 pad_id, F32 real_dist, F32 max_dist, xRumbleType type, F32 maxTime);
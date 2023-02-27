#pragma once

#include "xVec3.h"

enum sound_category
{
    SND_CAT_INVALID = -1,
    SND_CAT_GAME = 0,
    SND_CAT_DIALOG,
    SND_CAT_MUSIC,
    SND_CAT_CUTSCENE,
    SND_CAT_UI,
    SND_CAT_NUM_CATEGORIES
};

void xSndInit();
void xSndExit();

U32 xSndPlay3D(U32 id, F32 vol, F32 pitch, U32 priority, U32 flags, const xVec3* pos, F32 innerRadius, F32 outerRadius, sound_category category, F32 delay);

inline U32 xSndPlay3D(U32 id, F32 vol, F32 pitch, U32 priority, U32 flags, const xVec3* pos, F32 radius, sound_category category, F32 delay) NONMATCH("https://decomp.me/scratch/WcXTe")
{
    return xSndPlay3D(id, vol, pitch, priority, flags, pos, radius * 0.25f, radius, category, delay);
}
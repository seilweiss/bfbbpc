#pragma once

#include "iSnd.h"
#include "xVec3.h"

struct xEnt;

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
U32 xSndPlay(U32 id, F32 vol, F32 pitch, U32 priority, U32 flags, U32 parentID, sound_category category, F32 delay);
U32 xSndPlay3D(U32 id, F32 vol, F32 pitch, U32 priority, U32 flags, xEnt* parent, F32 innerRadius, F32 outerRadius, sound_category category, F32 delay);
U32 xSndPlay3D(U32 id, F32 vol, F32 pitch, U32 priority, U32 flags, const xVec3* pos, F32 innerRadius, F32 outerRadius, sound_category category, F32 delay);
void xSndStopChildren(U32 pid);

inline U32 xSndPlay3D(U32 id, F32 vol, F32 pitch, U32 priority, U32 flags, const xVec3* pos, F32 radius, sound_category category, F32 delay) NONMATCH("https://decomp.me/scratch/WcXTe")
{
    return xSndPlay3D(id, vol, pitch, priority, flags, pos, radius * 0.25f, radius, category, delay);
}

inline S32 xSndIsPlaying(U32 assetID, U32 parid)
{
    return iSndIsPlaying(assetID, parid);
}
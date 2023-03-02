#pragma once

#include "xBaseAsset.h"
#include "xVec3.h"

struct xEntAsset : xBaseAsset
{
    U8 flags;
    U8 subtype;
    U8 pflags;
    U8 moreFlags;
    U8 pad;
    U32 surfaceID;
    xVec3 ang;
    xVec3 pos;
    xVec3 scale;
    F32 redMult;
    F32 greenMult;
    F32 blueMult;
    F32 seeThru;
    F32 seeThruSpeed;
    U32 modelInfoID;
    U32 animListID;
};
#pragma once

#include "xBase.h"

struct xGroupAsset : xBaseAsset
{
    U16 itemCount;
    U16 groupFlags;
};

struct xGroup : xBase
{
    xGroupAsset* asset;
    xBase** item;
    U32 last_index;
    S32 flg_group;
};

U32 xGroupGetItem(xGroup* g, U32 index);
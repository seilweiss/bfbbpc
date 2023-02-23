#pragma once

#include "xBase.h"

struct xPortalAsset : xBaseAsset
{
    U32 assetCameraID;
    U32 assetMarkerID;
    F32 ang;
    U32 sceneID;
};

typedef struct _zPortal zPortal;
struct _zPortal : xBase
{
    xPortalAsset* passet;
};
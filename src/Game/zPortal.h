#pragma once

#include "xBase.h"

struct xPortalAsset;

typedef struct _zPortal zPortal;
struct _zPortal : xBase
{
    xPortalAsset* passet;
};
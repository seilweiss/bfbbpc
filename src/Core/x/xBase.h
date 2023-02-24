#pragma once

#include "xserializer.h"

struct xBase;

typedef S32(*xBaseEventCallback)(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget);

struct xLinkAsset
{
    U16 srcEvent;
    U16 dstEvent;
    U32 dstAssetID;
    F32 param[4];
    U32 paramWidgetAssetID;
    U32 chkAssetID;
};

struct xBaseAsset
{
    U32 id;
    U8 baseType;
    U8 linkCount;
    U16 baseFlags;
};

struct xBase
{
    U32 id;
    U8 baseType;
    U8 linkCount;
    U16 baseFlags;
    const xLinkAsset* link;
    xBaseEventCallback eventFunc;
};

#define k_XBASE_IS_ENABLED                       ((U16)(1 << 0))
#define k_XBASE_IS_PERSISTENT                    ((U16)(1 << 1))
#define k_XBASE_IS_VALID                         ((U16)(1 << 2))
#define k_XBASE_IS_VISIBLE_IN_CUTSCENES          ((U16)(1 << 3))
#define k_XBASE_RECEIVES_SHADOWS                 ((U16)(1 << 4))
#define k_XBASE_IS_ENTITY                        ((U16)(1 << 5))
#define k_XBASE_IS_NPC                           ((U16)(1 << 8))

void xBaseInit(xBase* xb, xBaseAsset* asset);
void xBaseSetup(xBase* xb);
void xBaseSave(xBase* ent, xSerial* s);
void xBaseLoad(xBase* ent, xSerial* s);
void xBaseReset(xBase* xb, xBaseAsset* asset);

inline void xBaseEnable(xBase* xb)
{
    xb->baseFlags |= k_XBASE_IS_ENABLED;
}

inline void xBaseDisable(xBase* xb)
{
    xb->baseFlags &= (U16)~k_XBASE_IS_ENABLED;
}

inline bool xBaseIsEnabled(const xBase* xb)
{
    return xb->baseFlags & k_XBASE_IS_ENABLED;
}

inline void xBaseValidate(xBase* xb)
{
    xb->baseFlags |= k_XBASE_IS_VALID;
}

inline U32 xBaseIsValid(const xBase* xb)
{
    return xb->baseFlags & k_XBASE_IS_VALID;
}
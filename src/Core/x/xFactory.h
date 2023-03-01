#pragma once

#include "xRMemData.h"
#include "xordarray.h"

struct xFactoryInst;

struct XGOFTypeInfo
{
    S32 tid;
    xFactoryInst*(*creator)(S32, RyzMemGrow*, void*);
    void(*destroyer)(xFactoryInst*);
};

struct xFactory : RyzMemData
{
protected:
    XGOFTypeInfo* infopool;
    XORDEREDARRAY infolist;
    xFactoryInst* products;
    RyzMemGrow growContextData;
};

struct xFactoryInst : RyzMemData
{
protected:
    S32 itemType;
    xFactoryInst* nextprod;
    xFactoryInst* prevprod;
};
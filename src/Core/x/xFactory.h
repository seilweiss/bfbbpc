#pragma once

#include "xRMemData.h"
#include "xordarray.h"

struct xFactoryInst : RyzMemData
{
protected:
    S32 itemType;
    xFactoryInst* nextprod;
    xFactoryInst* prevprod;

    friend struct xFactory;

public:
    xFactoryInst()
    {
        itemType = 0;
        nextprod = prevprod = NULL;
    }

    ~xFactoryInst() {}
};

typedef xFactoryInst*(*xFactoryInstCreator)(S32, RyzMemGrow*, void*);
typedef void(*xFactoryInstDestroyer)(xFactoryInst*);

struct XGOFTypeInfo
{
    S32 tid;
    xFactoryInstCreator creator;
    xFactoryInstDestroyer destroyer;
};

struct xFactory : RyzMemData
{
protected:
    XGOFTypeInfo* infopool;
    st_XORDEREDARRAY infolist;
    xFactoryInst* products;
    RyzMemGrow growContextData;

public:
    xFactory(S32 maxTypes);
    ~xFactory();
    S32 RegItemType(XGOFTypeInfo* info);
    S32 RegItemType(S32 tid, xFactoryInstCreator create, xFactoryInstDestroyer destroy);
    void GrowDataEnable(xBase* user, S32 isResume);
    void GrowDataDisable();
    xFactoryInst* CreateItem(S32 typeID, void* userdata, RyzMemGrow* callerzgrow);
    void DestroyAll();
    void DestroyItem(xFactoryInst* item);
};
#include "xFog.h"

#include "xFogAsset.h"
#include "iCamera.h"
#include "zEvent.h"

void xFogClearFog() NONMATCH("https://decomp.me/scratch/9FqCo")
{
    iCameraSetFogParams(NULL, 0.0f);
}

void xFogInit(void* b, void* tasset)
{
    xFogInit((xBase*)b, (xFogAsset*)tasset);
}

void xFogInit(xBase* b, xFogAsset* tasset)
{
    xFog* t = (xFog*)b;

    xBaseInit(t, tasset);

    t->eventFunc = xFogEventCB;
    t->tasset = tasset;

    if (t->linkCount) {
        t->link = (xLinkAsset*)(t->tasset + 1);
    } else {
        t->link = NULL;
    }
}

void xFogReset(xFog* t)
{
    xBaseReset(t, t->tasset);
}

void xFogSave(xFog* ent, xSerial* s)
{
    xBaseSave(ent, s);
}

void xFogLoad(xFog* ent, xSerial* s)
{
    xBaseLoad(ent, s);
}

S32 xFogEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget)
{
    xFog* t = (xFog*)to;

    switch (toEvent) {
    case eEventOn:
    {
        iFogParams fog;
        fog.type = rwFOGTYPELINEAR;
        fog.start = t->tasset->fogStart;
        fog.stop = t->tasset->fogStop;
        fog.density = t->tasset->fogDensity;
        fog.fogcolor.red = t->tasset->fogColor[0];
        fog.fogcolor.green = t->tasset->fogColor[1];
        fog.fogcolor.blue = t->tasset->fogColor[2];
        fog.fogcolor.alpha = t->tasset->fogColor[3];
        fog.bgcolor.red = t->tasset->bkgndColor[0];
        fog.bgcolor.green = t->tasset->bkgndColor[1];
        fog.bgcolor.blue = t->tasset->bkgndColor[2];
        fog.bgcolor.alpha = t->tasset->bkgndColor[3];
        fog.table = NULL;
        iCameraSetFogParams(&fog, t->tasset->transitionTime);
        break;
    }
    case eEventOff:
        iCameraSetFogParams(NULL, 0.0f);
        break;
    case eEventReset:
        xFogReset(t);
        break;
    }

    return 1;
}

void xFogUpdate(xBase* to, xScene* sc, F32 dt)
{
}
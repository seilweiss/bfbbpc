#include "xLightKit.h"

#include "xMath.h"
#include "xModelBucket.h"

#include <rwframesync.h>

#include <string.h>

xLightKit* gLastLightKit;

xLightKit* xLightKit_Prepare(void* data) NONMATCH("https://decomp.me/scratch/AiEOV")
{
    xLightKit* lkit = (xLightKit*)data;
    lkit->lightList = (xLightKitLight*)(lkit + 1);
    
    xLightKitLight* currlight = (xLightKitLight*)(lkit + 1);
    for (U32 i = 0; i < lkit->lightCount; i++) {
        if (currlight->platLight) {
            return lkit;
        }

        if (currlight->color.red > 1.0f || currlight->color.green > 1.0f || currlight->color.blue > 1.0f) {
            F32 s = xmax(xmax(currlight->color.red, currlight->color.green), currlight->color.blue);
            s = xmax(s, 0.00001f);
            s = 1.0f / s;

            currlight->color.red *= s;
            currlight->color.green *= s;
            currlight->color.blue *= s;
        }

        switch (currlight->type) {
        case 1:
            currlight->platLight = RpLightCreate(rpLIGHTAMBIENT);
            break;
        case 2:
            currlight->platLight = RpLightCreate(rpLIGHTDIRECTIONAL);
            break;
        case 3:
            currlight->platLight = RpLightCreate(rpLIGHTPOINT);
            break;
        case 4:
            currlight->platLight = RpLightCreate(rpLIGHTSPOTSOFT);
            break;
        }

        RpLightSetColor(currlight->platLight, &currlight->color);

        if (currlight->type >= 2) {
            RwFrame* frame = RwFrameCreate();
            
            RwMatrix tmpmat;
            memset(&tmpmat, 0, sizeof(tmpmat));

            tmpmat.right.x = -currlight->matrix[0];
            tmpmat.right.y = -currlight->matrix[1];
            tmpmat.right.z = -currlight->matrix[2];
            tmpmat.up.x = currlight->matrix[4];
            tmpmat.up.y = currlight->matrix[5];
            tmpmat.up.z = currlight->matrix[6];
            tmpmat.at.x = -currlight->matrix[8];
            tmpmat.at.y = -currlight->matrix[9];
            tmpmat.at.z = -currlight->matrix[10];
            tmpmat.pos.x = currlight->matrix[12];
            tmpmat.pos.y = currlight->matrix[13];
            tmpmat.pos.z = currlight->matrix[14];

            RwV3dNormalize(&tmpmat.right, &tmpmat.right);
            RwV3dNormalize(&tmpmat.up, &tmpmat.up);
            RwV3dNormalize(&tmpmat.at, &tmpmat.at);

            RwFrameTransform(frame, &tmpmat, rwCOMBINEREPLACE);

            RpLightSetFrame(currlight->platLight, frame);
        }

        if (currlight->type >= 3) {
            RpLightSetRadius(currlight->platLight, currlight->radius);
        }

        if (currlight->type >= 4) {
            RpLightSetConeAngle(currlight->platLight, currlight->angle);
        }

        currlight++;
    }
    
    return lkit;    
}

void xLightKit_Enable(xLightKit* lkit, RpWorld* world)
{
    U32 i;
    
    if (lkit == gLastLightKit) return;

    if (gLastLightKit) {
        for (i = 0; i < gLastLightKit->lightCount; i++) {
            RpWorldRemoveLight(world, gLastLightKit->lightList[i].platLight);
        }
    }

    gLastLightKit = lkit;
    
    if (lkit) {
        iModelHack_DisablePrelight = 1;

        for (i = 0; i < lkit->lightCount; i++) {
            RpWorldAddLight(world, lkit->lightList[i].platLight);
        }
    } else {
        iModelHack_DisablePrelight = 0;
    }
}

xLightKit* xLightKit_GetCurrent()
{
    return gLastLightKit;
}

void xLightKit_Destroy(xLightKit* lkit)
{
    if (!lkit) return;

    U32 i;
    xLightKitLight* currLight = lkit->lightList;

    for (i = 0; i < lkit->lightCount; i++) {
        if (currLight->platLight) {
            _rwFrameSyncDirty();

            RwFrame* tframe = RpLightGetFrame(currLight->platLight);
            if (tframe) {
                RpLightSetFrame(currLight->platLight, NULL);
                RwFrameDestroy(tframe);
            }

            RpLightDestroy(currLight->platLight);
            currLight->platLight = NULL;
        }

        currLight++;
    }
}
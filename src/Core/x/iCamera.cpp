#include "iCamera.h"

#include "iSystem.h"
#include "iScrFX.h"

#include "xGlobals.h"
#include "xShadow.h"

#include "rwframesync.h"

#include <rpworld.h>

F32 sCameraNearClip = 0.05f;
F32 sCameraFarClip = 400.0f;
RwCamera* globalCamera;
static RwCamera* sMainGameCamera;

RwCamera* iCameraCreate(S32 width, S32 height, S32 mainGameCamera)
{
    RwV2d vw;

    RwCamera* camera = RwCameraCreate();
    RwCameraSetFrame(camera, RwFrameCreate());
    RwCameraSetRaster(camera, RwRasterCreate(width, height, 0, rwRASTERTYPECAMERA));
    RwCameraSetZRaster(camera, RwRasterCreate(width, height, 0, rwRASTERTYPEZBUFFER));
    RwCameraSetFarClipPlane(camera, sCameraFarClip);
    RwCameraSetNearClipPlane(camera, sCameraNearClip);

    vw.x = 1.0f, vw.y = 1.0f;
    RwCameraSetViewWindow(camera, &vw);

    if (mainGameCamera) {
        iScrFxCameraCreated(camera);
        sMainGameCamera = camera;
    }

    return camera;
}

void iCameraDestroy(RwCamera* camera)
{
    _rwFrameSyncDirty();

    RpWorld* pWorld = RwCameraGetWorld(camera);
    if (pWorld) {
        RpWorldRemoveCamera(pWorld, camera);
    }

    if (camera == sMainGameCamera) {
        iScrFxCameraDestroyed(camera);
        sMainGameCamera = NULL;
    }

    if (camera) {
        RwRaster* raster;
        RwFrame* frame;

        frame = RwCameraGetFrame(camera);
        if (frame) {
            RwCameraSetFrame(camera, NULL);
            RwFrameDestroy(frame);
        }

        raster = RwCameraGetRaster(camera);
        if (raster) {
            RwRasterDestroy(raster);
            RwCameraSetRaster(camera, NULL);
        }

        raster = RwCameraGetZRaster(camera);
        if (raster) {
            RwRasterDestroy(raster);
            RwCameraSetZRaster(camera, NULL);
        }

        RwCameraDestroy(camera);
    }
}

void iCameraBegin(RwCamera* cam, S32 clear)
{
    if (clear) {
        if (xglobals->fog.type != rwFOGTYPENAFOGTYPE) {
            RwCameraClear(cam, &xglobals->fog.bgcolor, rwCAMERACLEARIMAGE | rwCAMERACLEARZ);
        } else {
            RwCameraClear(cam, NULL, rwCAMERACLEARZ);
        }
    }

    RwCameraSetNearClipPlane(cam, sCameraNearClip);
    RwCameraBeginUpdate(cam);
}

void iCameraEnd(RwCamera* cam)
{
    iScrFxCameraEndScene(cam);
    RwCameraEndUpdate(cam);
    iScrFxPostCameraEnd(cam);
}

void iCameraShowRaster(RwCamera* cam)
{
    RwCameraShowRaster(cam, NULL, 0);
}

void iCameraFrustumPlanes(RwCamera* cam, xVec4* frustplane)
{
    RwFrustumPlane* rwPlane;

    rwPlane = &cam->frustumPlanes[2];
    frustplane[0].x = rwPlane->plane.normal.x;
    frustplane[1].x = rwPlane->plane.normal.y;
    frustplane[2].x = rwPlane->plane.normal.z;
    frustplane[3].x = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[4];
    frustplane[0].y = rwPlane->plane.normal.x;
    frustplane[1].y = rwPlane->plane.normal.y;
    frustplane[2].y = rwPlane->plane.normal.z;
    frustplane[3].y = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[5];
    frustplane[0].z = rwPlane->plane.normal.x;
    frustplane[1].z = rwPlane->plane.normal.y;
    frustplane[2].z = rwPlane->plane.normal.z;
    frustplane[3].z = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[3];
    frustplane[0].w = rwPlane->plane.normal.x;
    frustplane[1].w = rwPlane->plane.normal.y;
    frustplane[2].w = rwPlane->plane.normal.z;
    frustplane[3].w = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[1];
    frustplane[4].x = rwPlane->plane.normal.x;
    frustplane[5].x = rwPlane->plane.normal.y;
    frustplane[6].x = rwPlane->plane.normal.z;
    frustplane[7].x = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[0];
    frustplane[4].y = rwPlane->plane.normal.x;
    frustplane[5].y = rwPlane->plane.normal.y;
    frustplane[6].y = rwPlane->plane.normal.z;
    frustplane[7].y = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[2];
    frustplane[4].z = rwPlane->plane.normal.x;
    frustplane[5].z = rwPlane->plane.normal.y;
    frustplane[6].z = rwPlane->plane.normal.z;
    frustplane[7].z = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[4];
    frustplane[4].w = rwPlane->plane.normal.x;
    frustplane[5].w = rwPlane->plane.normal.y;
    frustplane[6].w = rwPlane->plane.normal.z;
    frustplane[7].w = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[2];
    frustplane[8].x = rwPlane->plane.normal.x;
    frustplane[9].x = rwPlane->plane.normal.y;
    frustplane[10].x = rwPlane->plane.normal.z;
    frustplane[11].x = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[4];
    frustplane[8].y = rwPlane->plane.normal.x;
    frustplane[9].y = rwPlane->plane.normal.y;
    frustplane[10].y = rwPlane->plane.normal.z;
    frustplane[11].y = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[5];
    frustplane[8].z = rwPlane->plane.normal.x;
    frustplane[9].z = rwPlane->plane.normal.y;
    frustplane[10].z = rwPlane->plane.normal.z;
    frustplane[11].z = rwPlane->plane.distance;

    rwPlane = &cam->frustumPlanes[3];
    frustplane[8].w = rwPlane->plane.normal.x;
    frustplane[9].w = rwPlane->plane.normal.y;
    frustplane[10].w = rwPlane->plane.normal.z;
    frustplane[11].w = rwPlane->plane.distance;
}

void iCameraUpdatePos(RwCamera* cam, xMat4x3* pos)
{
    RwFrame* f = RwCameraGetFrame(cam);
    RwMatrix* m;

    m = RwFrameGetMatrix(f);
    xMat4x3Copy((xMat4x3*)m, pos);

    m = RwFrameGetLTM(f);
    xMat4x3Copy((xMat4x3*)m, pos);

    RwFrameOrthoNormalize(f);
    RwFrameUpdateObjects(f);
}

void iCameraSetFOV(RwCamera* cam, F32 fov) NONMATCH("https://decomp.me/scratch/1pNVv")
{
    F32 view_window = itan(xdeg2rad(0.5f * fov));
    F32 fAspectRatio = (F32)FB_YRES/FB_XRES;
    RwV2d vw;
    vw.x = view_window;
    vw.y = fAspectRatio * view_window;
    RwCameraSetViewWindow(cam, &vw);
}

void iCameraAssignEnv(RwCamera* camera, iEnv* env_geom)
{
    globalCamera = camera;
    RpWorldAddCamera(env_geom->world, camera);
    xShadowSetWorld(env_geom->world);
}

void iCamGetViewMatrix(RwCamera* camera, xMat4x3* view_matrix)
{
    memset(view_matrix, 0, sizeof(xMat4x3));

    RwMatrix* rw_view = RwCameraGetViewMatrix(camera);
    view_matrix->right.x = rw_view->right.x;
    view_matrix->right.y = rw_view->right.y;
    view_matrix->right.z = rw_view->right.z;
    view_matrix->up.x = rw_view->up.x;
    view_matrix->up.y = rw_view->up.y;
    view_matrix->up.z = rw_view->up.z;
    view_matrix->at.x = rw_view->at.x;
    view_matrix->at.y = rw_view->at.y;
    view_matrix->at.z = rw_view->at.z;
    view_matrix->pos.x = rw_view->pos.x;
    view_matrix->pos.y = rw_view->pos.y;
    view_matrix->pos.z = rw_view->pos.z;
}

void iCameraSetNearFarClip(F32 nearPlane, F32 farPlane) NONMATCH("https://decomp.me/scratch/6zf6w")
{
    if (nearPlane <= 0.0f) nearPlane = 0.05f;
    sCameraNearClip = nearPlane;

    if (farPlane <= 0.0f) farPlane = 400.0f;
    sCameraFarClip = farPlane;
}

void iCameraSetFogParams(iFogParams* fp, F32 time) NONMATCH("https://decomp.me/scratch/Tr5VP")
{
    if (!fp || fp->type == rwFOGTYPENAFOGTYPE) {
        xglobals->fog.type = rwFOGTYPENAFOGTYPE;
        xglobals->fogA.type = rwFOGTYPENAFOGTYPE;
    } else if (time == 0.0f || fp->type != xglobals->fogA.type) {
        xglobals->fog = *fp;
        xglobals->fogA = *fp;
        xglobals->fog_t0 = 0;
    } else {
        xglobals->fogA = xglobals->fog;
        xglobals->fogB = *fp;
        xglobals->fog_t0 = iTimeGet();
        xglobals->fog_t1 = xglobals->fog_t0 + iTimeFromSec(time);
    }
}

void iCameraUpdateFog(RwCamera*, iTime t)
{
    iFogParams* fp = &xglobals->fog;
    iFogParams* fa = &xglobals->fogA;
    iFogParams* fb = &xglobals->fogB;

    if (xglobals->fog.type != rwFOGTYPENAFOGTYPE && xglobals->fog_t0 != 0) {
        t = iTimeGet();

        F32 a = iTimeDiffSec(xglobals->fog_t0, t) /
                iTimeDiffSec(xglobals->fog_t0, xglobals->fog_t1);
        a = xclamp(a, 0.0f, 1.0f);

        fp->type = xglobals->fogB.type;
        fp->table = xglobals->fogB.table;
        fp->start = xlerp(fa->start, fb->start, a);
        fp->stop = xlerp(fa->stop, fb->stop, a);
        fp->density = xlerp(fa->density, fb->density, a);

        RwRGBA ca, cb;
        ca = fa->fogcolor;
        cb = fb->fogcolor;

        RwRGBA color;
        color.red = (RwUInt8)xlerp(ca.red, cb.red, a);
        color.green = (RwUInt8)xlerp(ca.green, cb.green, a);
        color.blue = (RwUInt8)xlerp(ca.blue, cb.blue, a);
        color.alpha = (RwUInt8)xlerp(ca.alpha, cb.alpha, a);

        fp->fogcolor = color;

        ca = fa->bgcolor;
        cb = fb->bgcolor;

        color.red = (RwUInt8)xlerp(ca.red, cb.red, a);
        color.green = (RwUInt8)xlerp(ca.green, cb.green, a);
        color.blue = (RwUInt8)xlerp(ca.blue, cb.blue, a);
        color.alpha = (RwUInt8)xlerp(ca.alpha, cb.alpha, a);

        fp->bgcolor = color;

        if (a == 1.0f) {
            xglobals->fog_t0 = 0;
            xglobals->fogA = xglobals->fogB;
        }
    }
}

void iCameraSetFogRenderStates()
{
    RwCamera* pCamera = RwCameraGetCurrentCamera();
    iFogParams* pFogParams = &xglobals->fog;
    
    if (pFogParams->type == rwFOGTYPENAFOGTYPE) {
        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
        RwCameraSetFarClipPlane(pCamera, sCameraFarClip);
    } else {
        U32 bite_me = RWRGBALONG(pFogParams->fogcolor.red,
                                 pFogParams->fogcolor.green,
                                 pFogParams->fogcolor.blue,
                                 pFogParams->fogcolor.alpha);

        RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATEFOGTYPE, (void*)pFogParams->type);
        RwRenderStateSet(rwRENDERSTATEFOGCOLOR, (void*)bite_me);
        RwRenderStateSet(rwRENDERSTATEFOGDENSITY, (void*)&pFogParams->density);
        RwCameraSetFogDistance(pCamera, pFogParams->start);
        RwCameraSetFarClipPlane(pCamera, pFogParams->stop);
    }
}
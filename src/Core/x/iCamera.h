#pragma once

#include "iFog.h"
#include "iEnv.h"
#include "iTime.h"

#include "xMath3.h"

#include <rwcore.h>

extern F32 sCameraNearClip;
extern F32 sCameraFarClip;
extern RwCamera* globalCamera;

RwCamera* iCameraCreate(S32 width, S32 height, S32 mainGameCamera);
void iCameraDestroy(RwCamera* camera);
void iCameraBegin(RwCamera* cam, S32 clear);
void iCameraEnd(RwCamera* cam);
void iCameraShowRaster(RwCamera* cam);
void iCameraFrustumPlanes(RwCamera* cam, xVec4* frustplane);
void iCameraUpdatePos(RwCamera* cam, xMat4x3* pos);
void iCameraSetFOV(RwCamera* cam, F32 fov);
void iCameraAssignEnv(RwCamera* camera, iEnv* env_geom);
void iCamGetViewMatrix(RwCamera* camera, xMat4x3* view_matrix);
void iCameraSetNearFarClip(F32 nearPlane, F32 farPlane);
void iCameraSetFogParams(iFogParams* fp, F32 time);
void iCameraUpdateFog(RwCamera*, iTime t);
void iCameraSetFogRenderStates();
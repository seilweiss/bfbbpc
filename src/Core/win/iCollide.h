#pragma once

#include "xMath3.h"

#include <rwcore.h>
#include <rpworld.h>
#include <rpcollis.h>

struct xScene;
struct xCollis;
struct xEnv;
struct xModelInstance;

void iCollideInit(xScene* sc);
RpCollisionTriangle* sphereHitsEnvCB(RpIntersection* isx, RpWorldSector* sector, RpCollisionTriangle* tri, F32 dist, void* data);
U32 iSphereHitsEnv(const xSphere* b, const xEnv* env, xCollis* coll);
S32 iSphereHitsEnv3(const xSphere* b, const xEnv* env, xCollis* colls, U8 ncolls, F32 sth);
S32 iSphereHitsEnv4(const xSphere* b, const xEnv* env, const xMat3x3* mat, xCollis* colls);
S32 iSphereHitsModel3(const xSphere* b, const xModelInstance* m, xCollis* colls, U8 ncolls, F32 sth);
U32 iRayHitsEnv(const xRay3* r, const xEnv* env, xCollis* coll);
U32 iRayHitsModel(const xRay3* r, const xModelInstance* m, xCollis* coll);
void iSphereForModel(xSphere* o, const xModelInstance* m);
void iBoxForModel(xBox* o, const xModelInstance* m);
void iBoxForModelLocal(xBox* o, const xModelInstance* m);
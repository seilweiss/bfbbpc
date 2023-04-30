#include "zEntTrigger.h"

#include "xTriggerAsset.h"
#include "xLinkAsset.h"
#include "zScene.h"
#include "zEvent.h"

void zEntTriggerInit(void* ent, void* asset)
{
    zEntTriggerInit((zEntTrigger*)ent, (xEntAsset*)asset);
}

void zEntTriggerInit(zEntTrigger* ent, xEntAsset* asset) NONMATCH("https://decomp.me/scratch/pKxUP")
{
    zEntInit(ent, asset, 'TRIG');
    
    if (ent->subType == 0) {
        xMat3x3Euler(&ent->triggerMatrix, &asset->ang);
        
        xVec3 boxUpper, boxLower, center;
        const xTriggerAsset* tasset = (xTriggerAsset*)(asset + 1);

        boxUpper = tasset->p[1];
        boxLower = tasset->p[0];
        xVec3Sub(&boxUpper, &boxUpper, &asset->pos);
        xVec3Sub(&boxLower, &boxLower, &asset->pos);
        xVec3Lerp(&center, &boxLower, &boxUpper, 0.5f);
        xMat3x3RMulVec(&center, &ent->triggerMatrix, &center);
        
        xVec3Add(&ent->triggerMatrix.pos, &center, &asset->pos);

        ent->triggerBox.lower.x = -0.5f * (boxUpper.x - boxLower.x);
        ent->triggerBox.lower.y = -0.5f * (boxUpper.y - boxLower.y);
        ent->triggerBox.lower.z = -0.5f * (boxUpper.z - boxLower.z);
        ent->triggerBox.upper.x = -ent->triggerBox.lower.x;
        ent->triggerBox.upper.y = -ent->triggerBox.lower.y;
        ent->triggerBox.upper.z = -ent->triggerBox.lower.z;
    }

    ent->move = NULL;
    ent->update = (xEntUpdateCallback)zEntTriggerUpdate;
    ent->eventFunc = zEntTriggerEventCB;

    if (ent->linkCount) {
        ent->link = (xLinkAsset*)((U8*)ent->asset + sizeof(xEntAsset) + sizeof(xTriggerAsset));
    } else {
        ent->link = NULL;
    }

    ent->entered = 0;
}

void zEntTriggerUpdate(zEntTrigger* trig, xScene* sc, F32 dt) NONMATCH("https://decomp.me/scratch/A1Ylk")
{
    if (!xBaseIsEnabled(trig)) return;

    U32 i;
    const xLinkAsset* link = trig->link;
    xTriggerAsset* tasset = (xTriggerAsset*)(trig->asset + 1);

    for (i = 0; i < trig->linkCount; i++) {
        if (!link->chkAssetID) continue;
        if (link->srcEvent < eEventEnterEntity) continue;
        if (link->srcEvent > eEventExitEntityFLAG) continue;

        xIsect isect;
        S32 collide = 0;

        if (link->chkAssetID) {
            xEnt* chkEnt = (xEnt*)zSceneFindObject(link->chkAssetID);
            if (chkEnt) {
                xVec3* chkPos;
                if (chkEnt->bound.type == k_XBOUNDTYPE_SPHERE) {
                    chkPos = &chkEnt->bound.sph.center;
                } else {
                    chkPos = (xVec3*)&chkEnt->model->Mat->pos;
                }

                switch (trig->subType) {
                case eEntTriggerTypeBox:
                {
                    xVec3 xformVec;
                    xMat4x3Tolocal(&xformVec, &trig->triggerMatrix, chkPos);
                    iBoxIsectVec(&trig->triggerBox, &xformVec, &isect);
                    
                    collide = (isect.penned <= 0.0f);
                    break;
                }
                case eEntTriggerTypeSphere:
                {
                    xSphere sphere;
                    sphere.center = tasset->p[0];
                    sphere.r = tasset->p[1].x;
                    
                    xIsect isect;
                    iSphereIsectVec(&sphere, chkPos, &isect);
                    
                    collide = (isect.penned <= 0.0f);
                    break;
                }
                case eEntTriggerTypeCylinder:
                {
                    xCylinder cylinder;
                    cylinder.center = tasset->p[0];
                    cylinder.r = tasset->p[1].x;
                    cylinder.h = tasset->p[1].y;
                    
                    xIsect isect;
                    iCylinderIsectVec(&cylinder, chkPos, &isect);

                    collide = (isect.penned <= 0.0f);
                    break;
                }
                case eEntTriggerTypeCircle:
                {
                    xSphere sphere;
                    sphere.center = tasset->p[0];
                    sphere.r = tasset->p[1].x;
                    
                    xIsect isect;
                    iSphereIsectVec(&sphere, chkPos, &isect);

                    collide = (isect.penned <= 0.0f);
                    break;
                }
                }
            }
        }

        if (collide) {
            if (link->srcEvent == eEventEnterEntity) {
                ((xLinkAsset*)link)->srcEvent = eEventEnterEntityFLAG;
                zEntEvent(trig, eEventEnterEntity,
                          zSceneFindObject(link->dstAssetID), link->dstEvent,
                          link->param, zSceneFindObject(link->paramWidgetAssetID), 0);
            }
            if (link->srcEvent == eEventExitEntity) {
                ((xLinkAsset*)link)->srcEvent = eEventExitEntityFLAG;
            }
        } else {
            if (link->srcEvent == eEventExitEntityFLAG) {
                ((xLinkAsset*)link)->srcEvent = eEventExitEntity;
                zEntEvent(trig, eEventExitEntity,
                          zSceneFindObject(link->dstAssetID), link->dstEvent,
                          link->param, zSceneFindObject(link->paramWidgetAssetID), 0);
            }
            if (link->srcEvent == eEventEnterEntityFLAG) {
                ((xLinkAsset*)link)->srcEvent = eEventEnterEntity;
            }
        }
        
        link++;
    }
}

S32 zEntTriggerEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget)
{
    zEntTrigger* trig = (zEntTrigger*)to;

    switch (toEvent) {
    case eEventReset:
        zEntTriggerReset(trig);
        break;
    }

    return 1;
}

void zEntTriggerSave(zEntTrigger* ent, xSerial* s)
{
    zEntSave(ent, s);
}

void zEntTriggerLoad(zEntTrigger* ent, xSerial* s)
{
    zEntLoad(ent, s);
}

void zEntTriggerReset(zEntTrigger* ent)
{
    ent->entered = 0;

    zEntReset(ent);
}

bool zEntTriggerHitsSphere(const zEntTrigger& trig, const xSphere& o, const xVec3& dir)
{
    if (!xBaseIsEnabled(&trig)) {
        return false;
    }

    const xTriggerAsset& asset = zEntTriggerAsset(trig);
    if ((asset.flags & 0x1) && dir.dot(asset.direction) <= 0.0f) {
        return false;
    }

    switch (trig.subType) {
    case eEntTriggerTypeBox:
        return xSphereHitsOBB(o, trig.triggerBox, trig.triggerMatrix);
    case eEntTriggerTypeSphere:
        return xSphereHitsSphere(o.center, o.r, asset.p[0], asset.p[1].x);
    case eEntTriggerTypeCylinder:
        return xSphereHitsVCylinder(o.center, o.r, asset.p[0], asset.p[1].x, asset.p[1].y);
    case eEntTriggerTypeCircle:
        return xSphereHitsVCircle(o, asset.p[0], asset.p[1].x);
    }

    return false;
}
#include "xEntBoulder.h"

#include "xShadow.h"
#include "xEntAsset.h"
#include "xEntBoulderAsset.h"
#include "xString.h"
#include "xFX.h"
#include "xPlatformAsset.h"
#include "xSnd.h"
#include "xutil.h"
#include "xLinkAsset.h"
#include "xstransvc.h"
#include "xMarkerAsset.h"
#include "zEnt.h"
#include "zGlobals.h"
#include "zBase.h"
#include "zGrid.h"
#include "zEvent.h"
#include "zFX.h"
#include "zGoo.h"
#include "zEntButton.h"
#include "zPlatform.h"
#include "zEntDestructObj.h"
#include "zNPCTypeCommon.h"
#include "zSurface.h"
#include "zSurfAsset.h"
#include "zCollGeom.h"
#include "zShrapnelAsset.h"

static xEntCollis sBoulderCollis;
static U32 sBubbleStreakID = 0xDEAD;

void xEntBoulder_FitToModel(xEntBoulder* ent)
{
    xVec3Copy(&ent->bound.sph.center, (xVec3*)&ent->model->Data->boundingSphere.center);
    ent->bound.sph.r = ent->model->Data->boundingSphere.radius;

    xVec3Copy(&ent->localCenter, &ent->bound.sph.center);
    xVec3AddTo(&ent->bound.sph.center, (xVec3*)&ent->model->Mat->pos);
}

void xEntBoulder_Render(xEnt* ent) NONMATCH("https://decomp.me/scratch/bhvbY")
{
    xModelInstance* model = ent->model;
    
    if (!model || !xEntIsVisible(ent) || (model->Flags & 0x400)) return;

    if (ent->flags & k_XENT_0x40) {
        if (!iModelCull(model->Data, model->Mat)) {
            xModelRender(model);
        }
        return;
    }
    
    S32 shadowResult;
    xVec3 shadVec;
    shadVec.x = model->Mat->pos.x;
    shadVec.y = model->Mat->pos.y - 10.0f;
    shadVec.z = model->Mat->pos.z;

    if (!iModelCullPlusShadow(model->Data, model->Mat, &shadVec, &shadowResult)) {
        xModelRender(model);
    }

    if (shadowResult) return;

    if (ent->flags & k_XENT_0x10) {
        xShadowManager_Add(ent);
    } else {
        F32 radius = ent->model->Data->boundingSphere.radius;
        if (radius > 0.75f) radius = 0.75f;

        xShadowSimple_Add(ent->simpShadow, ent, 2.0f * radius, 1.0f);
    }
}

void xEntBoulder_Init(void* ent, void* asset)
{
    xEntBoulder_Init((xEntBoulder*)ent, (xEntAsset*)asset);
}

void xEntBoulder_Init(xEntBoulder* ent, xEntAsset* asset)
{
    xEntInit(ent, asset);

    ent->collType = k_XENT_COLLTYPE_DYN;
    ent->collLev = 4;
    ent->bound.type = k_XBOUNDTYPE_SPHERE;
    ent->moreFlags |= k_MORE_FLAGS_HITTABLE;

    zEntParseModelInfo(ent, asset->modelInfoID);

    xEntInitShadow(*ent, ent->entShadow_embedded);

    ent->simpShadow = &ent->simpShadow_embedded;
    xShadowSimple_CacheInit(ent->simpShadow, ent, 80);

    ent->frame = (xEntFrame*)xMALLOC(sizeof(xEntFrame));
    memset(ent->frame, 0, sizeof(xEntFrame));

    ent->collis = NULL;
    
    xEntBoulderAsset* basset = (xEntBoulderAsset*)(asset + 1);
    ent->basset = basset;

    ent->eventFunc = xEntBoulderEventCB;
    ent->update = (xEntUpdateCallback)xEntBoulder_Update;
    ent->bupdate = xEntBoulder_BUpdate;
    ent->render = xEntBoulder_Render;

    if (ent->linkCount) {
        ent->link = (xLinkAsset*)((U8*)ent->asset + sizeof(xEntAsset) + sizeof(xEntBoulderAsset));
    } else {
        ent->link = NULL;
    }

    xEntBoulder_FitToModel(ent);

    if (basset->mass <= 0.0f) {
        basset->mass = 1.0f;
    }

    xEntBoulder_Reset(ent, globals.sceneCur);

    if (ent->asset->id == xStrHash("BUBBLEBOWL")) {
        ent->update = NULL;

        xEntHide(ent);

        ent->chkby = 0;
        ent->penby = 0;
        ent->collis_chk = 0;
        ent->collis_pen = 0;

        globals.player.bubblebowl = ent;

        RpAtomicSetRenderCallBack(ent->model->Data, xFXBubbleRender);
    }

    ent->lastRolling = 0.0f;
    ent->rollingID = 0;

    if (ent->asset->modelInfoID == xStrHash("cannon_puffer")) {
        ent->rollingID = xStrHash("Boulder_Bounce");
    }

    ent->baseFlags |= k_XBASE_IS_ENTITY;
}

void xEntBoulder_ApplyForces(xEntCollis* collis)
{
    for (S32 i = collis->dyn_sidx; i < collis->dyn_eidx; i++) {
        if (collis->colls[i].optr && ((xEnt*)collis->colls[i].optr)->baseType == eBaseTypeBoulder) {
            xVec3 f;
            xEntBoulder* boul = (xEntBoulder*)collis->colls[i].optr;

            xVec3SMul(&f, &collis->colls[i].norm, -5.0f);

            xEntDrive* drv = &globals.player.drv;
            if (boul == drv->odriver || boul == drv->driver) {
                if (collis->colls[i].norm.y > 0.5f) {
                    f.x = -f.x;
                    f.z = -f.z;
                }
            }

            xEntBoulder_AddForce(boul, &f);

            if (boul->basset->flags & 0x2) {
                zEntPlayer_DamageNPCKnockBack(boul, 1, &boul->bound.sph.center);
            }
        }
    }
}

void xEntBoulder_AddInstantForce(xEntBoulder* ent, xVec3* force)
{
    if (!ent->update) return;

    xVec3AddTo(&ent->instForce, force);
}

void xEntBoulder_AddForce(xEntBoulder* ent, xVec3* force)
{
    if (!ent->update) return;

    xVec3AddTo(&ent->force, force);
}

void xEntBoulder_BUpdate(xEnt*, xVec3*)
{
}

void xEntBoulder_RealBUpdate(xEnt* ent, xVec3* pos)
{
    xEntBoulder* boul = (xEntBoulder*)ent;
    
    xVec3 rotatedLC;
    xMat3x3RMulVec(&rotatedLC, (xMat3x3*)boul->model->Mat, &boul->localCenter);

    xVec3Add(&boul->bound.sph.center, pos, &rotatedLC);
    xBoundUpdate(&boul->bound);
    
    zGridUpdateEnt(boul);
}

void xEntBoulder_Update(xEntBoulder* ent, xScene* sc, F32 dt) NONMATCH("https://decomp.me/scratch/cGZaG")
{
    if (ent->timeToLive > 0.0f) {
        ent->timeToLive -= dt;
        if (ent->timeToLive <= 0.0f) {
            zEntEvent(ent, eEventKill);
            return;
        }
    }

    S32 i;
    zEnt* plent = &globals.player.ent;

    if (ent == globals.player.bubblebowl) {
        F32 dist;
        xVec3Dist2Macro(&plent->bound.sph.center, (xVec3*)&ent->model->Mat->pos, &dist);

        if (dist > xsqr(60.0f)) {
            zEntEvent(ent, eEventKill);
            return;
        }
    }

    if (!ent->collis) {
        ent->collis = &sBoulderCollis;
    }

    ent->collis->chk = ent->collis_chk;
    ent->collis->pen = ent->collis_pen;
    ent->collis->post = NULL;
    ent->collis->depenq = NULL;

    xVec3 rotatedLC;
    xMat3x3RMulVec(&rotatedLC, (xMat3x3*)ent->model->Mat, &ent->localCenter);
    xVec3Add(&ent->bound.sph.center, (xVec3*)&ent->model->Mat->pos, &rotatedLC);

    if (sBubbleStreakID != 0xDEAD && globals.player.bubblebowl == ent) {
        xVec3 a, b;
        a = ent->bound.sph.center - ent->rotVec * ent->bound.sph.r;
        b = ent->bound.sph.center + ent->rotVec * ent->bound.sph.r;
        xFXStreakUpdate(sBubbleStreakID, &a, &b);
    }

    {
        xVec3 tmp;
        tmp.x = tmp.y = tmp.z = 4.0f * ent->bound.sph.r;

        F32 fn = 0.1f * xVec3LengthFast(&ent->vel);
        U32 n = (fn >= 1.0f) ? (U32)fn : (xurand() < fn);
        if (n) {
            zFX_SpawnBubbleTrail(&ent->bound.sph.center, n, NULL, &tmp);
        }
    }

    xVec3 oldPos;
    xVec3Copy(&oldPos, &ent->bound.sph.center);

    ent->vel.y -= ent->basset->gravity * dt;

    xVec3AddScaled(&ent->vel, &ent->instForce, 1.0f / ent->basset->mass);
    xVec3Init(&ent->instForce, 0.0f, 0.0f, 0.0f);
    
    xVec3AddScaled(&ent->vel, &ent->force, dt / ent->basset->mass);
    xVec3Init(&ent->force, 0.0f, 0.0f, 0.0f);

    xVec3 velNorm;
    xVec3Normalize(&velNorm, &ent->vel);

    xVec3AddScaled(&ent->bound.sph.center, &ent->vel, dt);

    xQuickCullForBound(&ent->bound.qcd, &ent->bound);
    zGridUpdateEnt(ent);

    zGooCollsBegin();
    xEntBeginUpdate(ent, sc, dt);

    ent->collType = k_XENT_COLLTYPE_PC;
    xEntCollide(ent, sc, dt);
    ent->collType = k_XENT_COLLTYPE_DYN;

    xEntEndUpdate(ent, sc, dt);
    zGooCollsEnd();

    if (xEntBoulder_KilledBySurface(ent, sc, dt)) {
        if (ent->collis == &sBoulderCollis) {
            ent->collis = NULL;
        }
        return;
    }

    S32 numDepens = 0;
    xVec3 depen;

    xVec3 var_E0; // unused
    xVec3Init(&var_E0, 0.0f, 0.0f, 0.0f);

    if (ent->collis->env_eidx > ent->collis->env_sidx ||
        ent->collis->dyn_eidx > ent->collis->dyn_sidx ||
        ent->collis->npc_eidx > ent->collis->npc_sidx ||
        ent->collis->stat_eidx > ent->collis->stat_sidx) {
        xVec3Init(&depen, 0.0f, 0.0f, 0.0f);

        for (i = ent->collis->env_sidx; i < ent->collis->env_eidx; i++) {
            if (ent->basset->flags & 0x1) {
                xVec3AddTo(&depen, &ent->collis->colls[i].depen);
            } else {
                F32 penComp = xVec3Dot(&ent->collis->colls[i].norm, &ent->collis->colls[i].depen);

                if (ent == globals.player.bubblebowl &&
                    xVec3Dot(&ent->collis->colls[i].norm, &velNorm) < -0.70710677f &&
                    ent->timeToLive > 0.05f) {
                    ent->timeToLive = 0.05f;
                }

                xVec3AddScaled(&depen, &ent->collis->colls[i].norm, penComp);
            }

            numDepens++;
        }

        for (i = ent->collis->dyn_sidx; i < ent->collis->dyn_eidx; i++) {
            if (ent->basset->flags & 0x1) {
                xVec3AddTo(&depen, &ent->collis->colls[i].depen);
            } else {
                F32 penComp = xVec3Dot(&ent->collis->colls[i].norm, &ent->collis->colls[i].depen);
                xVec3AddScaled(&depen, &ent->collis->colls[i].norm, penComp);
            }

            xEnt* xb = (xEnt*)ent->collis->colls[i].optr;
            
            if (xb->baseType == eBaseTypeBoulder) {
                xEntBoulder* boul = (xEntBoulder*)xb;

                xVec3 force;
                xVec3Normalize(&force, &ent->collis->colls[i].depen);

                F32 v1 = xVec3Dot(&ent->vel, &force);
                F32 v2 = xVec3Dot(&boul->vel, &force);
                F32 forceMag = 2.0f *
                               ent->basset->mass * boul->basset->mass *
                               (v2 - v1) /
                               (ent->basset->mass + boul->basset->mass);

                xVec3SMulBy(&force, forceMag);

                xVec3 toAdd;
                xVec3SMul(&toAdd, &force, ent->basset->bounce);
                xEntBoulder_AddInstantForce(ent, &toAdd);

                xVec3SMul(&toAdd, &force, -boul->basset->bounce);
                xEntBoulder_AddInstantForce(boul, &toAdd);

                if (boul == boulderVehicle && (ent->basset->flags & 0x2)) {
                    zEntPlayer_Damage(boul, 1);
                }
            }

            if (xb->baseType == eBaseTypeButton) {
                zEntButton_Press((zEntButton*)xb, 0x8);
                zEntButton_Hold((zEntButton*)xb, 0x1000);
            }

            if (ent == globals.player.bubblebowl &&
                (xb->moreFlags & k_MORE_FLAGS_HITTABLE) &&
                xb->baseType == eBaseTypePlatform &&
                xb->subType == ePlatformTypePaddle &&
                ((zPlatform*)xb)->passet->paddle.paddleFlags & 0x10) {
                zPlatform_PaddleCollide(&ent->collis->colls[i], (xVec3*)&ent->model->Mat->pos, &ent->vel, 1);
            }

            numDepens++;
        }

        for (i = ent->collis->stat_sidx; i < ent->collis->stat_eidx; i++) {
            if (ent->basset->flags & 0x1) {
                xVec3AddTo(&depen, &ent->collis->colls[i].depen);
            } else {
                F32 penComp = xVec3Dot(&ent->collis->colls[i].norm, &ent->collis->colls[i].depen);

                if (ent == globals.player.bubblebowl &&
                    xVec3Dot(&ent->collis->colls[i].norm, &velNorm) < -0.70710677f &&
                    ent->timeToLive > 0.05f) {
                    ent->timeToLive = 0.05f;
                }

                xVec3AddScaled(&depen, &ent->collis->colls[i].norm, penComp);
            }

            xEnt* xb = (xEnt*)ent->collis->colls[i].optr;

            if ((ent->basset->flags & 0x4) && xb->baseType == eBaseTypeDestructObj) {
                if (zEntDestructObj_GetHit((zEntDestructObj*)xb, 0x8000) &&
                    ent == globals.player.bubblebowl &&
                    ent->timeToLive > 0.05f) {
                    ent->timeToLive = 0.05f;
                }

                zEntDestructObj_Hit((zEntDestructObj*)xb, 0x8000);
            } else if (ent == globals.player.bubblebowl &&
                       (xb->moreFlags & k_MORE_FLAGS_HITTABLE) &&
                       (xb->baseType != eBaseTypeBoulder || (((xEntBoulder*)xb)->basset->flags & 0x100))) {
                zEntEvent(ent, xb, eEventHit_BubbleBowl);
                zEntEvent(ent, xb, eEventHit);

                if (ent->timeToLive > 0.05f) {
                    ent->timeToLive = 0.05f;
                }
            }
            
            numDepens++;
        }

        for (i = ent->collis->npc_sidx; i < ent->collis->npc_eidx; i++) {
            zNPCCommon* npc = (zNPCCommon*)ent->collis->colls[i].optr;

            if (ent->basset->flags & 0x1) {
                xVec3AddTo(&depen, &ent->collis->colls[i].depen);
            } else {
                F32 penComp = xVec3Dot(&ent->collis->colls[i].norm, &ent->collis->colls[i].depen);

                if (ent != globals.player.bubblebowl ||
                    (npc->SelfType() & 0xFFFFFF00) != 'NTT\0') {
                    xVec3AddScaled(&depen, &ent->collis->colls[i].norm, penComp);
                }
            }

            if (ent->basset->flags & 0x8) {
                zEntEvent(ent, npc, eEventHit, NULL);

                zEntPlayer_SNDPlayStreamRandom(0, 16, ePlayerStreamSnd_BowlComment1, ePlayerStreamSnd_BowlComment3, 0.1f);
                zEntPlayer_SNDPlayStreamRandom(16, 35, ePlayerStreamSnd_BowlComment1, ePlayerStreamSnd_BowlComment4, 0.1f);
                zEntPlayer_SNDPlayStreamRandom(36, 100, ePlayerStreamSnd_BowlComment1, ePlayerStreamSnd_BowlComment5, 0.1f);

                if (ent == globals.player.bubblebowl &&
                    ((npc->SelfType() & 0xFFFFFF00) != 'NTT\0' || npc->SelfType() == 'NTT4') &&
                    ent->timeToLive > 0.05f) {
                    ent->timeToLive = 0.05f;
                }
            }

            numDepens++;
        }

        if (numDepens) {
            xVec3AddScaled(&ent->bound.sph.center, &depen, 1.0f / numDepens);
        }
    }

    xVec3 newVel;
    xVec3Sub(&newVel, &ent->bound.sph.center, &oldPos);
    xVec3SMul(&newVel, &newVel, 1.0f / dt);
    
    F32 newMag, oldMag;
    oldMag = xVec3Length(&ent->vel);
    newMag = xVec3Length(&newVel);

    if (oldMag > ent->basset->maxVel) {
        oldMag = ent->basset->maxVel;
    }

    if (newMag > oldMag) {
        xVec3SMulBy(&newVel, oldMag / newMag);
    }

    xVec3 depenNorm;
    F32 depenComp; // BUG: uninitialized (fixed in Rat)

    if (numDepens) {
        xVec3Normalize(&depenNorm, &depen);
        depenComp = xVec3Dot(&ent->vel, &depenNorm);
    }

    if (!(ent->basset->flags & 0x1) && numDepens) {
        if (ent->basset->bounce && depenComp / oldMag < -1.0f + ent->basset->bounce) {
            F32 afterBounce = xVec3Dot(&newVel, &depenNorm);
            if (-depenComp > ent->basset->bounceDamp) {
                xVec3AddScaled(&newVel, &depenNorm, -ent->basset->bounce * depenComp - afterBounce);
            }
        }
        if (ent->basset->friction) {
            xVec3 fricComp;
            xVec3Copy(&fricComp, &ent->vel);
            xVec3AddScaled(&fricComp, &depenNorm, -depenComp);
            xVec3AddScaled(&newVel, &fricComp, -ent->basset->friction * dt);
        }
    }

    xVec3Copy(&ent->vel, &newVel);

    if (numDepens) {
        xVec3 newRotVec;
        if (xVec3Length2(&depen) < 0.001f) {
            xVec3Init(&newRotVec, ent->vel.z, 0.0f, -ent->vel.x);
        } else {
            xVec3Cross(&newRotVec, &depen, &ent->vel);
        }
        xVec3Normalize(&newRotVec, &newRotVec);

        xVec3SMulBy(&ent->rotVec, 1.0f - ent->basset->stickiness);
        xVec3AddScaled(&ent->rotVec, &newRotVec, ent->basset->stickiness);
        
        if (xVec3Normalize(&ent->rotVec, &ent->rotVec) < 0.00001f) {
            ent->angVel = 0.0f;
        } else {
            F32 newAngVel = xVec3Length(&ent->vel) / ent->bound.sph.r;
            ent->angVel = (1.0f - ent->basset->stickiness) * ent->angVel + ent->basset->stickiness * newAngVel;
        }
    }

    if (ent->angVel > ent->basset->maxAngVel) {
        ent->angVel = ent->basset->maxAngVel;
    }

    if (ent->angVel < -ent->basset->maxAngVel) {
        ent->angVel = -ent->basset->maxAngVel;
    }

    if (ent->angVel > 0.075f || ent->angVel < -0.075f) {
        xMat3x3 rotM;
        xMat3x3Rot(&rotM, &ent->rotVec, ent->angVel * dt);

        xMat3x3Mul((xMat3x3*)ent->model->Mat, (xMat3x3*)ent->model->Mat, &rotM);
    }

    xMat3x3RMulVec(&rotatedLC, (xMat3x3*)ent->model->Mat, &ent->localCenter);
    xVec3Sub((xVec3*)&ent->model->Mat->pos, &ent->bound.sph.center, &rotatedLC);

    if (ent->basset->soundID && numDepens && ent->lastRolling > 0.25f && -depenComp > ent->basset->minSoundVel) {
        F32 vol = (-depenComp - ent->basset->minSoundVel) / xmax(0.00001f, ent->basset->maxSoundVel - ent->basset->minSoundVel);
        if (vol > 1.0f) vol = 1.0f;
        vol *= 0.77f * ent->basset->volume;

        xSndPlay3D(ent->basset->soundID, vol, 0.0f, 0, 0,
                   (xVec3*)&ent->model->Mat->pos, ent->basset->innerRadius, ent->basset->outerRadius,
                   SND_CAT_GAME, 0.0f);
    }

    if (ent->rollingID && numDepens && ent->lastRolling > 0.25f) {
        xSndPlay3D(ent->rollingID, 0.77f, 0.0f, 0, 0,
                   (xVec3*)&ent->model->Mat->pos, 20.0f,
                   SND_CAT_GAME, 0.0f);
    }

    if (numDepens) {
        ent->lastRolling = 0.0f;
    } else {
        ent->lastRolling += dt;
    }

    if (ent->collis == &sBoulderCollis) {
        ent->collis = NULL;
    }
}

S32 xEntBoulder_KilledBySurface(xEntBoulder* ent, xScene*, F32)
{
    if (!(ent->basset->flags & 0x70)) {
        return 0;
    }
    
    xCollis* coll = &ent->collis->colls[0];
    xCollis* cend = coll + ent->collis->idx;

    while (coll < cend) {
        if (coll->flags & k_HIT_IT) {
            if ((ent->basset->flags & 0x40) && coll->optr) {
                F32 temp;
                if (zGooIs((xEnt*)coll->optr, temp, 0)) {
                    xVec3AddScaled(&ent->vel, &coll->norm,
                                   -(1.0f + ent->basset->bounce) * xVec3Dot(&ent->vel, &coll->norm));
                    zEntEvent(ent, eEventKill);
                    return 1;
                }
            }
            
            xSurface* surf = zSurfaceGetSurface(coll);
            if (surf && surf->state == 0) {
                zSurfaceProps* prop = (zSurfaceProps*)surf->moprops;
                if (prop && prop->asset) {
                    if (ent->basset->flags & 0x10) {
                        switch (prop->asset->game_damage_type) {
                        case 0:
                            break;
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 6:
                            ent->hitpoints--;
                            if (ent->hitpoints <= 0) {
                                xVec3AddScaled(&ent->vel, &coll->norm,
                                               -(1.0f + ent->basset->bounce) * xVec3Dot(&ent->vel, &coll->norm));
                                zEntEvent(ent, eEventKill);
                                return 1;
                            }
                            break;
                        case 5:
                            xVec3AddScaled(&ent->vel, &coll->norm,
                                           -(1.0f + ent->basset->bounce) * xVec3Dot(&ent->vel, &coll->norm));
                            zEntEvent(ent, eEventKill);
                            return 1;
                        }
                    }

                    if ((ent->basset->flags & 0x20) &&
                        (prop->asset->phys_flags & 0x10)) {
                        xVec3AddScaled(&ent->vel, &coll->norm,
                                       -(1.0f + ent->basset->bounce) * xVec3Dot(&ent->vel, &coll->norm));
                        zEntEvent(ent, eEventKill);
                        return 1;
                    }
                }
            }
        }

        coll++;
    }

    return 0;
}

void xEntBoulder_Kill(xEntBoulder* ent)
{
    ent->chkby = 0;
    ent->penby = 0;
    ent->collis_chk = 0;
    ent->collis_pen = 0;
    ent->update = NULL;

    xEntHide(ent);

    if (ent == globals.player.bubblebowl && sBubbleStreakID != 0xDEAD) {
        xFXStreakStop(sBubbleStreakID);
        sBubbleStreakID = 0xDEAD;
    }
}

void xEntBoulder_BubbleBowl(F32 multiplier) NONMATCH("https://decomp.me/scratch/82zNR")
{
    xEntBoulder* ent = globals.player.bubblebowl;
    if (!ent) return;

    if (ent->update == (xEntUpdateCallback)xEntBoulder_Update) {
        zEntEvent(ent, eEventKill);
    }

    xEntBoulder_Reset(ent, globals.sceneCur);

    ent->update = (xEntUpdateCallback)xEntBoulder_Update;

    if (sBubbleStreakID != 0xDEAD) {
        xFXStreakStop(sBubbleStreakID);
        sBubbleStreakID = 0xDEAD;
    }

    xVec3Copy((xVec3*)&ent->model->Mat->pos, (xVec3*)&globals.player.ent.model->Mat->pos);
    xVec3AddScaled((xVec3*)&ent->model->Mat->pos, (xVec3*)&globals.player.ent.model->Mat->right, globals.player.g.BubbleBowlLaunchPosLeft);
    xVec3AddScaled((xVec3*)&ent->model->Mat->pos, (xVec3*)&globals.player.ent.model->Mat->up, globals.player.g.BubbleBowlLaunchPosUp);
    xVec3AddScaled((xVec3*)&ent->model->Mat->pos, (xVec3*)&globals.player.ent.model->Mat->at, globals.player.g.BubbleBowlLaunchPosAt);

    xVec3Copy(&ent->bound.sph.center, (xVec3*)&ent->model->Mat->pos);
    xVec3Copy(&ent->frame->mat.pos, (xVec3*)&ent->model->Mat->pos);

    xVec3SMul(&ent->vel, (xVec3*)&globals.player.ent.model->Mat->right, globals.player.g.BubbleBowlLaunchVelLeft);
    xVec3AddScaled(&ent->vel, (xVec3*)&globals.player.ent.model->Mat->up, globals.player.g.BubbleBowlLaunchVelUp);
    xVec3AddScaled(&ent->vel, (xVec3*)&globals.player.ent.model->Mat->at, globals.player.g.BubbleBowlLaunchVelAt);
    xVec3SMulBy(&ent->vel, multiplier);

    xVec3Copy(&ent->rotVec, (xVec3*)&globals.player.ent.model->Mat->right);

    ent->angVel = 10.0f * multiplier;

    xRay3 ray;
    xVec3Copy(&ray.origin, &globals.player.ent.bound.sph.center);
    xVec3Sub(&ray.dir, (xVec3*)&ent->model->Mat->pos, &ray.origin);
    ray.max_t = xVec3Normalize(&ray.dir, &ray.dir);
    ray.min_t = 0.01f + globals.player.ent.bound.sph.r;
    ray.flags = XRAY3_USE_MIN | XRAY3_USE_MAX;
    
    xCollis rayCollis;
    xRayHitsScene(globals.sceneCur, &ray, &rayCollis);

    if (rayCollis.flags & k_HIT_IT) {
        xEnt* optr = (xEnt*)rayCollis.optr;

        if (optr) {
            if (optr->baseType == eBaseTypeDestructObj && !zEntDestructObj_GetHit((zEntDestructObj*)optr, 0x8000)) {
            } else if ((optr->moreFlags & k_MORE_FLAGS_HITTABLE) && optr->baseType != eBaseTypeStatic) {
                goto dont_kill_me;
            }

            if (optr->baseType == eBaseTypeNPC) {
                goto dont_kill_me;
            }
        }

        if (optr && optr->collLev == 5) {
            iRayHitsModel(&ray, optr->model, &rayCollis);
            if (rayCollis.flags & k_HIT_IT) {
                ent->timeToLive = 0.0001f;
            }
        } else {
            ent->timeToLive = 0.0001f;
        }
    }
    
dont_kill_me:

    ent->lastRolling = 0.0f;
    ent->rollingID = xStrHash("sound_bubblebowl_loop");

    sBubbleStreakID = xFXStreakStart(0.01f, 4.0f, 1.0f, 0, NULL, NULL, 1);
}

void xEntBoulder_Setup(xEntBoulder* ent) NONMATCH("https://decomp.me/scratch/Rddt5")
{
    ent->asset->redMult = 1.0f;
    ent->asset->greenMult = 1.0f;
    ent->asset->blueMult = 1.0f;

    if (ent->model) {
        ent->asset->seeThru = ent->model->Alpha;
    } else {
        ent->asset->seeThru = 1.0f;
    }

    xEntSetup(ent);
}

void xEntBoulder_Reset(xEntBoulder* ent, xScene*)
{
    ent->chkby = (k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC | k_XENT_COLLTYPE_DYN);
    ent->penby = k_XENT_COLLTYPE_PC;
    ent->collis_chk = 0x2E;
    ent->collis_pen = 0;

    xEntReset(ent);

    if (ent->id == 0x5F2E37B4 ||
        ent->id == 0x5F2E37B5 ||
        ent->id == 0x5F2E37B6 ||
        ent->id == 0x5F2E37B7) {
        ent->flags |= k_MORE_FLAGS_HITTABLE;
    }

    xVec3Init(&ent->force, 0.0f, 0.0f, 0.0f);
    xVec3Init(&ent->instForce, 0.0f, 0.0f, 0.0f);
    xVec3Init(&ent->vel, 0.0f, 0.0f, 0.0f);
    xVec3Init(&ent->rotVec, 1.0f, 0.0f, 0.0f);

    ent->angVel = 0.0f;

    if (ent->basset->flags & 0x200) {
        ent->timeToLive = ent->basset->killtimer;
    } else {
        ent->timeToLive = -1.0f;
    }

    ent->hitpoints = ent->basset->hitpoints;

    xEntBoulder_RealBUpdate(ent, (xVec3*)&ent->model->Mat->pos);

    // Mermalair - Rolling Ball Room
    if (globals.sceneCur->sceneID == IDTAG('B','C','0','4') &&
        ent->id == xStrHash("BALL_BOULDER")) {
        ent->collis_chk &= (U8)~0x08;
    }
}

S32 xEntBoulderEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget)
{
    xEntBoulder* s = (xEntBoulder*)to;

    switch (toEvent) {
    case eEventVisible:
    case eEventFastVisible:
        xEntShow(s);
        if (toParam && (S32)(0.5f + toParam[0]) == 77) {
            zFXPopOn(*s, toParam[1], toParam[2]);
        }
        break;
    case eEventInvisible:
    case eEventFastInvisible:
        xEntHide(s);
        if (toParam && (S32)(0.5f + toParam[0]) == 77) {
            zFXPopOff(*s, toParam[1], toParam[2]);
        }
        break;
    case eEventCollisionOn:
        s->chkby |= k_XENT_COLLTYPE_PC;
        break;
    case eEventCollisionOff:
        s->chkby &= (U8)~k_XENT_COLLTYPE_PC;
        break;
    case eEventCollision_Visible_On:
        s->chkby |= k_XENT_COLLTYPE_PC;
        xEntShow(s);
        if (toParam && (S32)(0.5f + toParam[0]) == 77) {
            zFXPopOn(*s, toParam[1], toParam[2]);
        }
        break;
    case eEventCollision_Visible_Off:
        s->chkby &= (U8)~k_XENT_COLLTYPE_PC;
        xEntHide(s);
        if (toParam && (S32)(0.5f + toParam[0]) == 77) {
            zFXPopOff(*s, toParam[1], toParam[2]);
        }
        break;
    case eEventCameraCollideOn:
        zCollGeom_CamEnable(s);
        break;
    case eEventCameraCollideOff:
        zCollGeom_CamDisable(s);
        break;
    case eEventReset:
        xEntBoulder_Reset(s, globals.sceneCur);
        if (xEntIsVisible(s)) {
            s->update = (xEntUpdateCallback)xEntBoulder_Update;
        }
        break;
    case eEventHit:
        if (s->update) {
            s->hitpoints--;
            if (s->hitpoints <= 0) {
                zEntEvent(s, eEventKill);
            }
        }
        break;
    case eEventKill:
        xEntBoulder_Kill(s);
        if (s == globals.player.bubblebowl) {
            zFX_SpawnBubbleHit(&s->bound.sph.center, xrand() % 64 + 36);
        }
        break;
    case eEventSetUpdateDistance:
        if (globals.updateMgr) {
            if (toParam[0] <= 0.0f) {
                xUpdateCull_SetCB(globals.updateMgr, s, xUpdateCull_AlwaysTrueCB, NULL);
            } else {
                FloatAndVoid dist;
                dist.f = xsqr(toParam[0]);
                xUpdateCull_SetCB(globals.updateMgr, s, xUpdateCull_DistanceSquaredCB, dist.v);
            }
        }
        break;
    case eEventLaunchShrapnel:
        if (toParamWidget) {
            zShrapnelAsset* shrap = (zShrapnelAsset*)toParamWidget;
            if (shrap->initCB) {
                shrap->initCB(shrap, s->model, &s->vel, NULL);
            }
        }
        break;
    case eEventSetLightKit:
        s->lightKit = (xLightKit*)toParamWidget;
        break;
    }
    
    return 1;
}

static void RecurseChild(xBase* child, xEntBoulder** boulList, S32& currBoul);

static S32 RecurseLinks(xLinkAsset* link, S32 count, xEntBoulder** boulList)
{
    S32 i;
    S32 numInList = 0;

    for (i = 0; i < count; i++) {
        xLinkAsset* currLink = &link[i];
        if (currLink->dstEvent == eEventConnectToChild) {
            xBase* mychild = zSceneFindObject(currLink->dstAssetID);
            if (mychild) {
                RecurseChild(mychild, boulList, numInList);
            }
        }
    }

    return numInList;
}

static void RecurseChild(xBase* child, xEntBoulder** boulList, S32& currBoul)
{
    switch (child->baseType) {
    case eBaseTypeBoulder:
        if (boulList) {
            boulList[currBoul] = (xEntBoulder*)child;
        }
        currBoul++;
        break;
    case eBaseTypeGroup:
    {
        S32 i, cnt;
        xGroup* grp = (xGroup*)child;

        cnt = xGroupGetCount(grp);
        for (i = 0; i < cnt; i++) {
            xBase* grpitem = xGroupGetItemPtr(grp, i);
            if (grpitem) {
                if (grpitem->baseType == eBaseTypeBoulder) {
                    if (boulList) {
                        boulList[currBoul] = (xEntBoulder*)grpitem;
                    }
                    currBoul++;
                } else if (grpitem->baseType == eBaseTypeGroup) {
                    RecurseChild(grpitem, boulList, currBoul);
                }
            }
        }
        
        break;
    }
    }
}

void xBoulderGenerator_Init(xBase& data, xDynAsset& asset, size_t asset_size)
{
    xBoulderGenerator_Init((xBoulderGenerator*)&data, (xBoulderGeneratorAsset*)&asset);
}

void xBoulderGenerator_Init(xBoulderGenerator* bg, xBoulderGeneratorAsset* asset) NONMATCH("https://decomp.me/scratch/8MBMb")
{
    xBaseInit(bg, asset);

    bg->eventFunc = xBoulderGenerator_EventCB;
    bg->bgasset = asset;

    if (bg->linkCount) {
        bg->link = (xLinkAsset*)(bg->bgasset + 1);
    } else {
        bg->link = NULL;
    }
    
    U32 size = 0;
    bg->objectPtr = xSTFindAsset(asset->object, &size);

    if (size == sizeof(xMarkerAsset)) {
        bg->isMarker = 1;
    } else {
        bg->isMarker = 0;
        bg->objectPtr = zSceneFindObject(asset->object);
    }

    bg->numBoulders = RecurseLinks(bg->link, bg->linkCount, NULL);
    bg->nextBoulder = bg->numBoulders;
    bg->boulderList = (xEntBoulder**)xMALLOC(bg->numBoulders * sizeof(xEntBoulder*) +
                                             bg->numBoulders * sizeof(S32));
    bg->boulderAges = (S32*)(bg->boulderList + bg->numBoulders);
    
    S32 boulCount = RecurseLinks(bg->link, bg->linkCount, bg->boulderList);

    xVec3Normalize(&bg->bgasset->initaxis, &bg->bgasset->initaxis);

    bg->lengthOfInitVel = xVec3Length(&bg->bgasset->initvel);

    if (bg->lengthOfInitVel > 0.00001f) {
        bg->perp1.x = bg->bgasset->initvel.z - bg->bgasset->initvel.y;
        bg->perp1.y = bg->bgasset->initvel.x - bg->bgasset->initvel.z;
        bg->perp1.z = bg->bgasset->initvel.y - bg->bgasset->initvel.x;

        xVec3Normalize(&bg->perp1, &bg->perp1);
        xVec3Cross(&bg->perp2, &bg->bgasset->initvel, &bg->perp1);
        xVec3SMulBy(&bg->perp1, bg->lengthOfInitVel);
    }

    xBoulderGenerator_Reset(bg);
}

void xBoulderGenerator_Reset(xBoulderGenerator* bg)
{
    for (S32 i = 0; i < bg->numBoulders; i++) {
        xEntBoulder_Kill(bg->boulderList[i]);
        bg->boulderAges[i] = 0;
    }
}

static void BoulderGen_GiveBirth(xBoulderGenerator* bg, S32 indx)
{
    for (S32 i = 0; i < bg->numBoulders; i++) {
        bg->boulderAges[i]++;
    }

    bg->boulderAges[indx] = 1;
}

static S32 GetBoulderForGenerating(xBoulderGenerator* bg)
{
    // BUG: i uninitialized
    S32 i, j;
    S32 oldestCulled = -1;
    S32 minAge = bg->numBoulders >> 1;
    S32 numList = bg->numBoulders;

    for (j = 0; j < numList; j++) {
        i = j + bg->nextBoulder;
        if (i >= numList) i -= numList;

        if (!xEntIsVisible(bg->boulderList[i])) {
            break;
        }
        
        if (bg->boulderAges[i] < minAge) {
            continue;
        }

        if (oldestCulled < 0) {
            oldestCulled = i;
            continue;
        }

        if (bg->boulderList[oldestCulled]->isCulled) {
            if (bg->boulderList[i]->isCulled && bg->boulderAges[oldestCulled] < bg->boulderAges[i]) {
                oldestCulled = i;
            }
        } else {
            if (bg->boulderList[i]->isCulled || bg->boulderAges[oldestCulled] < bg->boulderAges[i]) {
                oldestCulled = i;
            }
        }
    }

    if (j == numList) {
        if (oldestCulled < 0) {
            i = (S32)((numList - 1) * xurand());
            if (i >= numList) i = numList - 1;
        } else {
            i = oldestCulled;
        }
    }

    bg->nextBoulder = i + 1;
    if (bg->nextBoulder >= numList) {
        bg->nextBoulder = 0;
    }

    return i;
}

void xBoulderGenerator_Launch(xBoulderGenerator* bg, xVec3* pnt, F32 t) NONMATCH("https://decomp.me/scratch/DePQQ")
{
    S32 i = GetBoulderForGenerating(bg);
    xEntBoulder* b = bg->boulderList[i];

    if (b != bg->objectPtr || !xEntIsVisible(b)) {
        xEntBoulder_Reset(b, globals.sceneCur);
        zEntEvent(b, eEventBorn);

        b->update = (xEntUpdateCallback)xEntBoulder_Update;

        if (bg->isMarker) {
            xVec3Copy((xVec3*)&b->model->Mat->pos, &((xMarkerAsset*)bg->objectPtr)->pos);
        } else {
            xVec3Copy((xVec3*)&b->model->Mat->pos, (xVec3*)&((xEnt*)bg->objectPtr)->model->Mat->pos);
        }

        xVec3AddTo((xVec3*)&b->model->Mat->pos, &bg->bgasset->offset);
        xVec3Copy(&b->rotVec, &bg->bgasset->initaxis);

        b->angVel = bg->bgasset->angvel;
    }

    F32 invTime = 1.0f / t;

    b->vel.x = (pnt->x - b->model->Mat->pos.x) * invTime;
    b->vel.z = (pnt->z - b->model->Mat->pos.z) * invTime;
    b->vel.y = (pnt->y - b->model->Mat->pos.y) * invTime + b->basset->gravity * t * 0.5f;

    BoulderGen_GiveBirth(bg, i);
}

S32 xBoulderGenerator_EventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget)
{
    xBoulderGenerator* bg = (xBoulderGenerator*)to;
    xVec3 pnt;

    switch (toEvent) {
    case eEventGenerateBoulder:
        xBoulderGenerator_GenBoulder(bg);
        break;
    case eEventLaunchBoulderAtWidget:
        switch (toParamWidget->baseType) {
        case eBaseTypeVillain:
        case eBaseTypePlayer:
        case eBaseTypePickup:
        case eBaseTypePlatform:
        case eBaseTypeDoor:
        case eBaseTypeStatic:
        case eBaseTypeDynamic:
        case eBaseTypePendulum:
        case eBaseTypeHangable:
        case eBaseTypeButton:
        case eBaseTypeDestructObj:
        case eBaseTypeNPC:
        case eBaseTypeBoulder:
            xVec3Copy(&pnt, xEntGetPos((xEnt*)toParamWidget));
            if (toParam[1]) {
                pnt.x += 2.0f * (xurand() - 0.5f) * toParam[1];
                pnt.y += 2.0f * (xurand() - 0.5f) * toParam[1];
                pnt.z += 2.0f * (xurand() - 0.5f) * toParam[1];
            }
            xBoulderGenerator_Launch(bg, &pnt, toParam[0]);
            break;
        case eBaseTypeMovePoint:
            xVec3Copy(&pnt, zMovePointGetPos((zMovePoint*)toParamWidget));
            if (toParam[1]) {
                pnt.x += 2.0f * (xurand() - 0.5f) * toParam[1];
                pnt.y += 2.0f * (xurand() - 0.5f) * toParam[1];
                pnt.z += 2.0f * (xurand() - 0.5f) * toParam[1];
            }
            xBoulderGenerator_Launch(bg, &pnt, toParam[0]);
            break;
        }
        break;
    case eEventLaunchBoulderAtPoint:
        pnt.x = toParam[0];
        pnt.y = toParam[1];
        pnt.z = toParam[2];
        xBoulderGenerator_Launch(bg, &pnt, toParam[3]);
        break;
    case eEventLaunchBoulderAtPlayer:
        xVec3Copy(&pnt, (xVec3*)&globals.player.ent.model->Mat->pos);
        xVec3AddScaled(&pnt, &globals.player.ent.frame->dpos, toParam[0] * toParam[1] / globals.update_dt);
        if (toParam[2]) {
            pnt.x += 2.0f * (xurand() - 0.5f) * toParam[2];
            pnt.y += 2.0f * (xurand() - 0.5f) * toParam[2];
            pnt.z += 2.0f * (xurand() - 0.5f) * toParam[2];
        }
        xBoulderGenerator_Launch(bg, &pnt, toParam[0]);
        break;
    case eEventReset:
        xBoulderGenerator_Reset(bg);
        break;
    }

    return 1;
}

void xBoulderGenerator_GenBoulder(xBoulderGenerator* bg)
{
    S32 i = GetBoulderForGenerating(bg);
    xEntBoulder* b = bg->boulderList[i];

    if (bg->objectPtr == globals.player.bubblebowl) {
        xEntBoulder_Reset(b, globals.sceneCur);
        zEntEvent(b, eEventBorn);

        b->update = (xEntUpdateCallback)xEntBoulder_Update;
        
        xEntBoulder* bb = globals.player.bubblebowl;

        xVec3Copy((xVec3*)&b->model->Mat->pos, (xVec3*)&bb->model->Mat->pos);
        xVec3Copy(&b->rotVec, &bb->rotVec);
        b->angVel = bb->angVel;
        xVec3Copy(&b->vel, &bb->vel);

        xEntBoulder_Kill(bb);
    } else {
        if (b != bg->objectPtr || !xEntIsVisible(b)) {
            xEntBoulder_Reset(b, globals.sceneCur);
            zEntEvent(b, eEventBorn);

            b->update = (xEntUpdateCallback)xEntBoulder_Update;

            if (bg->isMarker) {
                xVec3Copy((xVec3*)&b->model->Mat->pos, &((xMarkerAsset*)bg->objectPtr)->pos);
            } else {
                xVec3Copy((xVec3*)&b->model->Mat->pos, (xVec3*)&((xEnt*)bg->objectPtr)->model->Mat->pos);
            }

            xVec3AddTo((xVec3*)&b->model->Mat->pos, &bg->bgasset->offset);

            if (bg->bgasset->offsetRand) {
                b->model->Mat->pos.x += 2.0f * (xurand() - 0.5f) * bg->bgasset->offsetRand;
                b->model->Mat->pos.y += 2.0f * (xurand() - 0.5f) * bg->bgasset->offsetRand;
                b->model->Mat->pos.z += 2.0f * (xurand() - 0.5f) * bg->bgasset->offsetRand;
            }

            xVec3Copy(&b->rotVec, &bg->bgasset->initaxis);

            b->angVel = bg->bgasset->angvel;
        }

        if (bg->lengthOfInitVel > 0.00001f && bg->bgasset->velAngleRand > 0.00001f) {
            F32 p1c = xurand() - 0.5f;
            F32 p2c = xurand() - 0.5f;
            F32 nf = 1.0f / xsqrt(xsqr(p1c) + xsqr(p2c));
            if (nf < 0.00001f) {
                p1c = 1.0f;
                p2c = 0.0f;
            } else {
                p1c *= nf;
                p2c *= nf;
            }

            xVec3 perpRand;
            xVec3SMul(&perpRand, &bg->perp1, p1c);
            xVec3AddScaled(&perpRand, &bg->perp2, p2c);

            F32 randAng = bg->bgasset->velAngleRand * (xurand() - 0.5f);
            randAng = xdeg2rad(randAng);

            p1c = icos(randAng);
            p2c = isin(randAng);

            xVec3SMul(&b->vel, &bg->bgasset->initvel, p1c);
            xVec3AddScaled(&b->vel, &perpRand, p2c);
        } else {
            xVec3Copy(&b->vel, &bg->bgasset->initvel);
        }

        if (bg->bgasset->velMagRand > 0.00001f) {
            if (bg->lengthOfInitVel > 0.00001f) {
                F32 sclMag = bg->bgasset->velMagRand * xurand() + 1.0f;
                xVec3SMulBy(&b->vel, sclMag);
            } else {
                b->vel.x = 2.0f * (xurand() - 0.5f) * bg->bgasset->velMagRand;
                b->vel.y = 2.0f * (xurand() - 0.5f) * bg->bgasset->velMagRand;
                b->vel.z = 2.0f * (xurand() - 0.5f) * bg->bgasset->velMagRand;
            }
        }
    }

    BoulderGen_GiveBirth(bg, i);
}
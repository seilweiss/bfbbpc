#include "zEntButton.h"

#include "xEntAsset.h"
#include "xstransvc.h"
#include "xString.h"
#include "xSnd.h"
#include "zEntButtonAsset.h"
#include "zEvent.h"
#include "zEntPlayer.h"
#include "zGlobals.h"
#include "zFX.h"
#include "zCollGeom.h"
#include "zShrapnelAsset.h"

static F32 sRedMultiplier = 1.0f;
static F32 sGreenMultiplier = 1.0f;
static F32 sBlueMultiplier = 1.0f;
static F32 sColorMultiplier = 1.0f;
static S32 sColorMultiplierSign = 1;

void zEntButton_Init(void* ent, void* asset)
{
    zEntButton_Init((zEntButton*)ent, (xEntAsset*)asset);
}

void zEntButton_Init(zEntButton* ent, xEntAsset* asset) NONMATCH("https://decomp.me/scratch/wUmKC")
{
    zEntInit(ent, asset, 'BUTN');
    
    zEntButtonAsset* passet = (zEntButtonAsset*)(asset + 1);
    xEntMotionAsset* emasset = (xEntMotionAsset*)(passet + 1);

    ent->basset = passet;
    
    U32 bufsize;
    void* info = xSTFindAsset(passet->modelPressedInfoID, &bufsize);

    if (info) {
        ent->modelPressed = xEntLoadModel(NULL, (RpAtomic*)info);
    } else {
        ent->modelPressed = NULL;
    }

    if (asset->modelInfoID == xStrHash("button")) {
        ent->topHeight = 0.36f;
    } else if (asset->modelInfoID == xStrHash("plate_pressure")) {
        ent->topHeight = 0.32524f;
    } else {
        ent->topHeight = 0.0f;
    }

    ent->pflags |= k_XENT_IS_MOVING;
    ent->penby |= k_XENT_COLLTYPE_PC;
    ent->chkby |= k_XENT_COLLTYPE_PC;
    ent->update = (xEntUpdateCallback)zEntButton_Update;
    ent->move = (xEntMoveCallback)zEntButton_Move;
    ent->eventFunc = zEntButtonEventCB;
    ent->render = (xEntRenderCallback)zEntButton_Render;

    if (ent->linkCount) {
        ent->link = (xLinkAsset*)(emasset + 1);
    } else {
        ent->link = NULL;
    }

    ent->eventFunc = zEntButtonEventCB; // redundant
    ent->state = 0;
    ent->oldState = ent->state;
    ent->holdTimer = 0.0f;
    ent->counter = ent->basset->resetDelay;

    xEntReset(ent);

    xEntMotionInit(&ent->motion, ent, emasset);
    xEntMotionRun(&ent->motion);

    ent->oldMotState = ent->motion.mech.state;
}

void zEntButton_Move(zEntButton* ent, xScene* s, F32 dt, xEntFrame* frame)
{
    if (ent->driver) {
        xEntFrame* dframe = ent->driver->frame;
        if (ent->driveMode == 0) {
            xVec3 dpos;
            xVec3Sub(&dpos, &dframe->mat.pos, &dframe->oldmat.pos);
            xVec3Add(&ent->motion.er.a, &ent->motion.er.a, &dpos);
            xVec3Add(&ent->motion.er.b, &ent->motion.er.b, &dpos);
        }
    }

    xEntMotionMove(&ent->motion, s, dt, frame);
}

void zEntButton_Setup(zEntButton* ent, xScene* sc)
{
    zEntSetup(ent);
}

void zEntButton_Save(zEntButton* ent, xSerial* s)
{
    zEntSave(ent, s);

    if (ent->state == 3) {
        s->Write_b1(1);
    } else {
        s->Write_b1(0);
    }

    if (ent->state == 1) {
        s->Write_b1(1);
    } else {
        s->Write_b1(0);
    }
}

void zEntButton_Load(zEntButton* ent, xSerial* s)
{
    zEntLoad(ent, s);

    S32 pressed = 0;
    S32 pushing = 0;

    s->Read_b1(&pressed);
    if (pressed) {
        ent->state = 1;
    }

    s->Read_b1(&pushing);
    if (pushing) {
        ent->state = 1;
    }
}

void zEntButton_Reset(zEntButton* ent, xScene* sc)
{
    xEntReset(ent);

    xEntMotionInit(&ent->motion, ent, (xEntMotionAsset*)(ent->basset + 1));
    xEntMotionReset(&ent->motion, sc);

    ent->pflags |= k_XENT_IS_MOVING;
    ent->penby |= k_XENT_COLLTYPE_PC;

    if (ent->asset->flags & k_XENT_IS_VISIBLE) {
        ent->flags |= k_XENT_IS_VISIBLE;
    }

    ent->chkby |= k_XENT_COLLTYPE_PC;
    ent->bupdate(ent, (xVec3*)&ent->model->Mat->pos);
    ent->state = 0;

    xEntMotionStop(&ent->motion);

    ent->oldState = 5;
    ent->oldMotState = 6;
}

void zEntButton_Update(zEntButton* ent, xScene* sc, F32 dt) NONMATCH("https://decomp.me/scratch/9LhwT")
{
    if (ent->modelPressed) {
        ent->modelPressed->Mat = ent->model->Mat;
    }

    xEntUpdate(ent, sc, dt);

    if (ent->motion.mech.state != ent->oldMotState) {
        switch (ent->motion.mech.state) {
        case 0:
            if (ent->state == 4) {
                ent->state = 0;
            }
            break;
        case 2:
            if (!xEntMotionIsStopped(&ent->motion)) {
                ent->state = 3;
            }
            break;
        case 3:
            ent->state = 4;
            break;
        case 4:
        case 5:
        case 6:
            break;
        }
    }

    ent->oldMotState = ent->motion.mech.state;

    if (ent->state != ent->oldState) {
        switch (ent->state) {
        case 1:
            xEntMotionRun(&ent->motion);
            break;
        case 3:
            xEntMotionStop(&ent->motion);
            break;
        case 4:
            xEntMotionRun(&ent->motion);
            break;
        case 0:
            if (ent->motion.mech.state == 0) {
                xEntMotionStop(&ent->motion);
            }
            if (ent->basset->actMethod == 1) {
                zEntEvent(ent, ent, eEventUnpress);
            }
            break;
        }

        ent->oldState = ent->state;
    }

    if (ent->basset->actMethod == 0) {
        if (ent->basset->isReset && ent->state != 0) {
            ent->counter += dt;
        }
        if (ent->counter > ent->basset->resetDelay && ent->state == 3) {
            zEntEvent(ent, ent, eEventUnpress);
        }
    } else if (ent->basset->actMethod == 1) {
        if (!ent->hold) {
            zEntButton_SetReady(ent);
        }
    }

    ent->holdTimer -= dt;
    if (ent->holdTimer < 0.0f) {
        ent->holdTimer = 0.5f;
        ent->hold = 0;
    }
}

void zEntButton_Render(zEntButton* ent)
{
    xModelInstance* model = ent->model;

    if (ent->state == 3 && ent->modelPressed) {
        ent->model = ent->modelPressed;
    } else if (ent->basset->actMethod == 0) {
        model->RedMultiplier = sRedMultiplier;
        model->GreenMultiplier = sGreenMultiplier;
        model->BlueMultiplier = sBlueMultiplier;
    }

    if (ent->collType != k_XENT_COLLTYPE_TRIG) {
        xEntRender(ent);
    }

    ent->model = model;
}

void zEntButton_SetReady(zEntButton* ent)
{
    if (ent->state != 0) {
        ent->state = 4;
    }
}

static void zEntButton_Press(zEntButton* ent) NONMATCH("https://decomp.me/scratch/22UDT")
{
    xSndPlay3D(xStrHash("Button_press"), 0.77f, 0.0f, 128, 0,
               (xVec3*)&ent->model->Mat->pos, 1.0f, 30.0f,
               SND_CAT_GAME, 0.0f);

    if (ent->state == 0) {
        ent->state = 1;
        ent->counter = 0.0f;
    }
}

static void _PressButtonSound(U32 mask)
{
    if (mask & 0x8) {
        zEntPlayer_SNDPlayStreamRandom(0, 16, ePlayerStreamSnd_BowlComment1, ePlayerStreamSnd_BowlComment3, 0.1f);
        zEntPlayer_SNDPlayStreamRandom(16, 35, ePlayerStreamSnd_BowlComment1, ePlayerStreamSnd_BowlComment4, 0.1f);
        zEntPlayer_SNDPlayStreamRandom(36, 100, ePlayerStreamSnd_BowlComment1, ePlayerStreamSnd_BowlComment5, 0.1f);
    } else if (!(mask & 0x800) && xrand() % 4 == 3) {
        zEntPlayer_SNDPlayStreamRandom(ePlayerStreamSnd_PushButton1, ePlayerStreamSnd_PushButton3, 0.2f);
    }
}

void zEntButton_Press(zEntButton* ent, U32 mask)
{
    if ((ent->basset->buttonActFlags & mask) && ent->state == 0) {
        zEntEvent(ent, eEventPress);
        _PressButtonSound(mask);
    }
}

void zEntButton_Hold(zEntButton* ent, U32 mask)
{
    if (!(ent->basset->buttonActFlags & mask)) {
        return;
    }
    
    if (mask & 0x400) {
        switch (ent->bound.type) {
        case k_XBOUNDTYPE_SPHERE:
            break;
        case k_XBOUNDTYPE_BOX:
            break;
        case k_XBOUNDTYPE_CYL:
            break;
        case k_XBOUNDTYPE_OBB:
        {
            xVec3* player = (xVec3*)&globals.player.ent.model->Mat->pos;
            xBound* bound = &ent->bound;

            xVec3 lv;
            xMat4x3Tolocal(&lv, bound->mat, player);
            lv.y = 0.0f;

            if (!xPointInBox(&bound->box.box, &lv)) {
                return;
            }
            
            break;
        }
        }
    }

    if (ent->state == 0) {
        zEntEvent(ent, eEventPress);
        _PressButtonSound(0);
    } else {
        ent->hold = 1;
    }
}

void zEntButton_SceneUpdate(F32 dt) NONMATCH("https://decomp.me/scratch/rDchi")
{
    sColorMultiplier += dt * sColorMultiplierSign * 2.5f;
    
    if (sColorMultiplier > 1.0f) {
        sColorMultiplierSign *= -1;
        sColorMultiplier = 1.0f;
    }

    if (sColorMultiplier < 0.0f) {
        sColorMultiplierSign *= -1;
        sColorMultiplier = 0.0f;
    }

    sRedMultiplier   = 0.6f + 0.4f * sColorMultiplier;
    sGreenMultiplier = 0.6f + 0.4f * sColorMultiplier;
    sBlueMultiplier  = 0.6f + 0.4f * sColorMultiplier;
}

S32 zEntButtonEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget)
{
    zEntButton* s = (zEntButton*)to;

    switch (toEvent) {
    case eEventVisible:
    case eEventFastVisible:
    case eEventCollision_Visible_On:
        s->chkby |= k_XENT_COLLTYPE_PC;
        xEntShow(s);
        s->bupdate(s, (xVec3*)&s->model->Mat->pos);
        if (toParam && (S32)(0.5f + toParam[0]) == 77) {
            zFXPopOn(*s, toParam[1], toParam[2]);
        }
        break;
    case eEventInvisible:
    case eEventFastInvisible:
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
        zEntButton_Reset(s, globals.sceneCur);
        break;
    case eEventPress:
        zEntButton_Press(s);
        break;
    case eEventUnpress:
        if (s->state != 0) {
            xSndPlay3D(xStrHash("Button_up"), 0.77f, 0.0f, 128, 0,
                       (xVec3*)&s->model->Mat->pos, 10.0f, 30.0f,
                       SND_CAT_GAME, 0.0f);
            zEntButton_SetReady(s);
        }
        break;
    case eEventToggle:
        if (s->state != 0) {
            zEntEvent(s, s, eEventUnpress);
        } else {
            zEntEvent(s, s, eEventPress);
        }
        break;
    case eEventAnimPlay:
    case eEventAnimPlayLoop:
    case eEventAnimStop:
    case eEventAnimPause:
    case eEventAnimResume:
    case eEventAnimTogglePause:
    case eEventAnimPlayRandom:
    case eEventAnimPlayMaybe:
        zEntAnimEvent(s, toEvent, toParam);
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
                shrap->initCB(shrap, s->model, NULL, NULL);
            }
        }
        break;
    }
    
    return 1;
}
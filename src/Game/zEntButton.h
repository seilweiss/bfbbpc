#pragma once

#include "zEnt.h"
#include "xEntMotion.h"

struct zEntButtonAsset;

typedef struct _zEntButton : zEnt
{
    zEntButtonAsset* basset;
    xEntMotion motion;
    U32 state;
    F32 speed;
    U32 oldState;
    S32 oldMotState;
    F32 counter;
    xModelInstance* modelPressed;
    F32 holdTimer;
    U32 hold;
    F32 topHeight;
} zEntButton;

void zEntButton_Init(void* ent, void* asset);
void zEntButton_Init(zEntButton* ent, xEntAsset* asset);
void zEntButton_Move(zEntButton* ent, xScene* s, F32 dt, xEntFrame* frame);
void zEntButton_Setup(zEntButton* ent, xScene* sc);
void zEntButton_Save(zEntButton* ent, xSerial* s);
void zEntButton_Load(zEntButton* ent, xSerial* s);
void zEntButton_Reset(zEntButton* ent, xScene* sc);
void zEntButton_Update(zEntButton* ent, xScene* sc, F32 dt);
void zEntButton_Render(zEntButton* ent);
void zEntButton_SetReady(zEntButton* ent);
void zEntButton_Press(zEntButton* ent, U32 mask);
void zEntButton_Hold(zEntButton* ent, U32 mask);
void zEntButton_SceneUpdate(F32 dt);
S32 zEntButtonEventCB(xBase* from, xBase* to, U32 toEvent, const F32* toParam, xBase* toParamWidget);
#pragma once

#include "xVec3.h"

#include <rwcore.h>

enum en_nparptyp
{
    NPAR_TYP_UNKNOWN,
    NPAR_TYP_OILBUB,
    NPAR_TYP_TUBESPIRAL,
    NPAR_TYP_TUBECONFETTI,
    NPAR_TYP_GLOVEDUST,
    NPAR_TYP_MONSOONRAIN,
    NPAR_TYP_SLEEPYZEEZ,
    NPAR_TYP_CHUCKSPLASH,
    NPAR_TYP_TARTARGUNK,
    NPAR_TYP_DOGBREATH,
    NPAR_TYP_VISSPLASH,
    NPAR_TYP_FIREWORKS,
    NPAR_TYP_NOMORE,
    NPAR_TYP_FORCE = FORCEENUMSIZEINT
};

struct NPARData
{
    xVec3 pos;
    F32 xy_size[2];
    F32 uv_tl[2];
    F32 uv_br[2];
    RwRGBA color;
    F32 tmr_remain;
    F32 tym_exist;
    F32 fac_abuse;
    xVec3 vel;
    S32 flg_popts : 24;
    S32 nparmode : 8;
    F32 unused[3];
};

struct NPARXtraData
{
};

struct NPARMgmt
{
    en_nparptyp typ_npar;
    S32 flg_npar;
    NPARData* par_buf;
    S32 cnt_active;
    S32 num_max;
    RwTexture* txtr;
    NPARXtraData* xtra_data;
    void** user_data;
};

void NPCSupplement_Startup();
void NPCSupplement_Shutdown();
void NPCSupplement_ScenePrepare();
void NPCSupplement_SceneFinish();
void NPCSupplement_ScenePostInit();
void NPCSupplement_SceneReset();
void NPCSupplement_Timestep(F32 dt);
void NPCC_MakeASplash(const xVec3* pos, F32 radius);
NPARMgmt* NPAR_PartySetup(en_nparptyp parType, void** userData, NPARXtraData* xtraData);
void NPAR_EmitFWExhaust(const xVec3* pos, const xVec3* vel);
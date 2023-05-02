#pragma once

#include "xMath3.h"

struct xParEmitterAsset;

struct xPar
{
    xPar* m_next;
    xPar* m_prev;
    F32 m_lifetime;
    U8 m_c[4];
    xVec3 m_pos;
    F32 m_size;
    xVec3 m_vel;
    F32 m_sizeVel;
    U8 m_flag;
    U8 m_mode;
    U8 m_texIdx[2];
    U8 m_rotdeg[3];
    U8 pad8;
    F32 totalLifespan;
    xParEmitterAsset* m_asset;
    F32 m_cvel[4];
    F32 m_cfl[4];
};
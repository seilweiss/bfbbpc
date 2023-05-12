#pragma once

#include "types.h"

struct xEnt;

S32 zGooAdd(xEnt* obj, F32 depth, S32 freezeGroup);
S32 zGooIs(xEnt* obj, F32& depth, U32 playerCheck);
void zGooCollsBegin();
void zGooCollsEnd();
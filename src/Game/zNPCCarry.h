#pragma once

#include "types.h"

enum en_NPC_CARRY_STATE
{
    zNPCCARRY_NONE,
    zNPCCARRY_PICKUP,
    zNPCCARRY_THROW,
    zNPCCARRY_ATTEMPTPICKUP,
    zNPCCARRY_FORCEINT = FORCEENUMSIZEINT
};
typedef enum en_NPC_CARRY_STATE NPC_CARRY_STATE;
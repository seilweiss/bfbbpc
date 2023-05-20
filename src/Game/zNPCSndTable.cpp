#include "zNPCSndTable.h"

void NPCS_Startup() WIP
{
}

void NPCS_Shutdown() WIP
{
}

void NPCS_SndTimersUpdate(F32 dt) WIP
{
}

void NPCS_SndTimersReset() WIP
{
}

void NPCS_SndTypePlayed(en_NPC_SOUND sndtype, F32 delayNext) WIP
{
}

S32 NPCS_SndOkToPlay(en_NPC_SOUND sndtype) WIP
{
    return 0;
}

void NPCS_SndTablePrepare(NPCSndTrax* trax) WIP
{
}

NPCSndProp* NPCS_SndFindProps(en_NPC_SOUND sndtype) WIP
{
    return NULL;
}

en_NPC_SOUND NPCS_SndTypeFromHash(U32 aid_snd, NPCSndTrax* cust, NPCSndTrax* share) WIP
{
    return NPC_STYP_LISTEND;
}

U32 NPCS_SndPickSimilar(en_NPC_SOUND sndtype, NPCSndTrax* cust, NPCSndTrax* share) WIP
{
    return 0;
}
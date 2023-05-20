#pragma once

#include "zNPCSnd.h"

void NPCS_Startup();
void NPCS_Shutdown();
void NPCS_SndTimersUpdate(F32 dt);
void NPCS_SndTimersReset();
void NPCS_SndTypePlayed(en_NPC_SOUND sndtype, F32 delayNext);
S32 NPCS_SndOkToPlay(en_NPC_SOUND sndtype);
void NPCS_SndTablePrepare(NPCSndTrax* trax);
NPCSndProp* NPCS_SndFindProps(en_NPC_SOUND sndtype);
en_NPC_SOUND NPCS_SndTypeFromHash(U32 aid_snd, NPCSndTrax* cust, NPCSndTrax* share);
U32 NPCS_SndPickSimilar(en_NPC_SOUND sndtype, NPCSndTrax* cust, NPCSndTrax* share);
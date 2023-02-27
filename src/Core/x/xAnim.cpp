// https://decomp.me/scratch/ZvUwZ

#include "xAnim.h"

#include "iModel.h"

#include "xDebug.h"
#include "xString.h"
#include "xModel.h"

#include <stdio.h>

static xMemPool sxAnimTempTranPool;
U32 gxAnimUseGrowAlloc;

#if defined(DEBUG) || defined(RELEASE)
static U32 sxMaxTempTranPool;
#endif

enum
{
    MAX_SUBST_TRANSITION = 32
};

static bool _xSingleCompare(const char ch, const char pattern)
{
    switch (pattern)
    {
    case '?':
    case '*':
    case '+':
        return true;
    case '#':
        return (ch >= '0' && ch <= '9');
    }

    return (ch == pattern);
}

static bool _xSingleCompare(const char ch, const char* pattern)
{
    while (*pattern != '\0') {
        if (_xSingleCompare(ch, *pattern))
            return true;
        pattern++;
    }
    return false;
}

static bool _xCharIn(const char ch, const char* string)
{
    for (int i = 0; string[i] != '\0'; i++)
        if (ch == string[i])
            return true;
    return false;
}

static bool _xCheckAnimNameInner(const char* name, const char* pattern, int patternSize, char* extra, int* nameOut, int* extraOut)
{
    const char* startExtra = NULL;
    const char* initialExtra = extra;

    int patternCurrent = 0, nameCurrent = 0;
    while (patternCurrent < patternSize) {
        const char masterCharacter = pattern[patternCurrent];
        switch (masterCharacter) {

        case '+':
        case '?':
            patternCurrent++;
            if (nameCurrent == 0)
                return false;
            nameCurrent++;
            if (masterCharacter == '?')
                break;



        case '*':
        {
            int check = patternCurrent + 1;
            char nextPattern[128] = { pattern[check] };
            while (nextPattern[0] == '{' || nextPattern[0] == '}' || nextPattern[0] == '<' || nextPattern[0] == '>')
                nextPattern[0] = pattern[++check];
            if (nextPattern[0] == '(') {
                int nextPatternCount = 0;



                bool first = true;
                int parenCount = 0;
                while (pattern[check] != '\0' && parenCount >= 0) {
                    if (pattern[check] == '(')
                        parenCount++;
                    else if (pattern[check] == ')')
                        parenCount--;
                    else if (parenCount == 0) {
                        const char* IGNORE_PATTERNS = "{}()<>";

                        if (pattern[check] == '|')
                            first = true;
                        else if (first && !_xCharIn(pattern[check], IGNORE_PATTERNS)) {
                            nextPattern[nextPatternCount++] = pattern[check];
                            first = false;
                        }
                    }
                    check++;
                }
                nextPattern[nextPatternCount] = '\0';
            }
            while (name[nameCurrent] != '\0' && !_xSingleCompare(name[nameCurrent], nextPattern))
                nameCurrent++;
            patternCurrent++;

            break;
        }
        case '#':
            if (name[nameCurrent] < '0' || name[nameCurrent] > '9')
                return false;
            nameCurrent++;
            while (name[nameCurrent] >= '0' && name[nameCurrent] <= '9')
                nameCurrent++;
            patternCurrent++;
            break;

        case '{':
            xASSERT(startExtra == NULL);
            xASSERT(extra != NULL);
            startExtra = name + nameCurrent;
            patternCurrent++;
            break;

        case '}':
        {
            xASSERT(startExtra != NULL);
            xASSERT(extra != NULL);
            const char* endExtra = name + nameCurrent;
            int length = endExtra - startExtra;

            if (extra != NULL) {
                memcpy(extra, startExtra, length);
                extra[length] = 0;
                extra += length + 1;
                *extra = 1;
            }
            startExtra = NULL;
            patternCurrent++;

            break;
        }


        case '(':
        {
            bool done = false;
            patternCurrent++; const char* current = pattern + patternCurrent;
            while (*current != ')' && *current != '\0') {
                const char* startPattern = current;
                while (*current != '\0' && *current != ')' && *current != '|') {

                    if (*current == '(') {
                        int pc = 1;
                        while (*current != '\0' && pc > 0) {
                            if (*current == ')')
                                pc--;
                            else if (*current == '(')
                                pc++;
                            current++;
                        }
                        if (*current != '\0')
                            current++;
                    } else current++;
                }
                if (current != startPattern) {
                    int nameOut, extraOut;
                    bool ret = (!done && _xCheckAnimNameInner(name + nameCurrent, startPattern, current - startPattern, extra, &nameOut, &extraOut));

                    if (ret) {
                        nameCurrent += nameOut;
                        extra += extraOut;


                        done = true;
                    } else if (extra)
                        *extra = 1;
                }
                if (*current == '|')
                    current++;

            }


            if (*current != '\0')
                current++;

            patternCurrent += current - (pattern + patternCurrent);

            if (!done)
                return false;

            break;
        }
        case '<':
        {
            patternCurrent++;


            const char* current = pattern + patternCurrent;
            const char* positive = current;
            while (*current != '\0' && *current != ';' && *current != '>')
                current++;

            const char* positiveEnd = current;
            const char* negative = NULL;
            const char* negativeEnd = NULL;
            if (*current == ';') {
                current++;
                negative = current;
                while (*current != '\0' && *current != '>')
                    current++;
                negativeEnd = current;
            }

            int nameOut;
            int extraOut;
            bool matched = _xCheckAnimNameInner(name + nameCurrent, positive, positiveEnd - positive, extra, &nameOut, &extraOut);

            if (matched) {
                if (negative != NULL && _xCheckAnimNameInner(name + nameCurrent, negative, negativeEnd - negative, NULL, NULL, NULL)) {

                    if (extra != NULL)
                        *extra = 1;
                    matched = false;
                } else {
                    nameCurrent += nameOut;
                    extra += extraOut;
                }
            }


            if (*current != '\0')
                current++;

            patternCurrent += current - (pattern + patternCurrent);

            if (!matched)
                return false;

            break;
        }
        case '\0':
            return false;

        default:
            if (name[nameCurrent] != pattern[patternCurrent])
                return false;
            nameCurrent++;
            patternCurrent++;
        }
    }


    if (nameOut != NULL)
        *nameOut = nameCurrent;
    if (extraOut != NULL)
        *extraOut = extra - initialExtra;
    return true;
}

static bool _xCheckAnimName(const char* name, const char* pattern, char* extra)
{

    *extra = 1;


    int patternLength = strlen(pattern);


    int sizeOut;
    bool ret = _xCheckAnimNameInner(name, pattern, patternLength, extra, &sizeOut, NULL);


    return (ret && name[sizeOut] == '\0');
}

#ifdef DEBUG
static void _xAnimDebugModeService()
{
#if 0
    xprintf("Temp transitions: %d / %d\n", sxAnimTempTranPool.Allocated, sxMaxTempTranPool);
#endif
}
#endif

void xAnimInit()
{


    xASSERTEQ((sizeof(xAnimPlay) & 0xf), 0);
    xASSERTEQ((sizeof(xAnimSingle) & 0xf), 0);
    xASSERTEQ((sizeof(xAnimActiveEffect) & 0x7), 0);


#ifdef DEBUG
    xDebugModeAdd("DM_ANIMMEM", _xAnimDebugModeService);
#endif


    iAnimInit();


    memset(&sxAnimTempTranPool, 0, sizeof(sxAnimTempTranPool));
#if defined(DEBUG) || defined(RELEASE)
    sxMaxTempTranPool = 0;
#endif
}

void xAnimTempTransitionInit(U32 count)
{
    xASSERT(sxAnimTempTranPool.Buffer == 0);
    xMemPoolSetup(&sxAnimTempTranPool, xMALLOC(count * sizeof(xAnimTransition)), 0, 0, NULL, sizeof(xAnimTransition), count, count / 2);
#if defined(DEBUG) || defined(RELEASE)
    sxMaxTempTranPool = count;
#endif
}

static F32 CalcRecipBlendMax(U16* timeOffs) NONMATCH("https://decomp.me/scratch/ZvUwZ")
{
    F32 test, max = 0.0f;
    while (timeOffs[0] != 0xFFFF) {
        test = timeOffs[0] / 1024.0f + ((timeOffs[1] == 0) ? (0.0f) : (1.0f / (timeOffs[1] / 1024.0f)));
        if (test > max) max = test;
        timeOffs += 2;
    }
    if (max == 0.0f)
        return 0.0f;

    return 1.0f / max;
}

static U32 StateHasTransition(xAnimState* state, xAnimTransition* tran)
{

    xAnimTransitionList* curr = state->List;
    while (curr != NULL) {
        if (curr->T == tran) return TRUE;
        curr = curr->Next;
    }
    return FALSE;
}

static U32 DefaultHasTransition(xAnimState* state, xAnimTransition* tran, U32* allocCount)
{
    U32 needalloc = 1;

    xAnimTransitionList* curr = state->Default;
    while (curr != NULL) {
        if (curr->T == tran) return TRUE;
        if (curr->T->Conditional == tran->Conditional && curr->T->UserFlags == tran->UserFlags) {

            needalloc = 0;
        }
        curr = curr->Next;
    }
    *allocCount += needalloc;
    return FALSE;
}

static U32 DefaultOverride(xAnimState* state, xAnimTransition* tran)
{
    xAnimTransitionList* curr;
    curr = state->Default;
    while (curr != NULL) {
        if (curr->T->Conditional == tran->Conditional && curr->T->UserFlags == tran->UserFlags) {
            curr->T = tran;
            return TRUE;
        }
        curr = curr->Next;
    }
    return FALSE;
}

#ifdef DEBUG
static U32 TableHasStateID(xAnimTable* table, U32 ID)
{
    xAnimState* curr = table->StateList;
    while (curr != NULL) {
        if (curr->ID == ID)
            return TRUE;
        curr = curr->Next;
    }
    return FALSE;
}
#endif

static void TransitionTimeInit(xAnimSingle* single, xAnimTransition* tran)
{
    if (tran->Flags & AnimTransition_0x20) {
        xAnimFile* src = single->State->Data;
        xAnimFile* dest = tran->Dest->Data;
        if ((src->FileFlags ^ dest->FileFlags) & AnimFile_Reverse)
            single->Time = dest->Duration - single->Time;
    } else {
        single->Time = ((tran->Dest->Flags & AnimTransition_0x100) && tran->DestTime == 0.0f) ? (single->State->Data->Duration * xurand()) : tran->DestTime;
    }

    single->LastTime = single->Time;
}

xAnimFile* xAnimFileNewBilinear(void** rawData, const char* name, U32 flags, xAnimFile** linkedList, U32 numX, U32 numY)
{
    S32 i;
    xAnimFile* afile = (xAnimFile*)xAnimMemAlloc((numX * numY) * sizeof(void*) + sizeof(xAnimFile));


    xASSERT(!((flags & AnimFile_Reverse) && (flags & AnimFile_BackAndForth)));


    if (numX > 1 || numY > 1)
        flags |= AnimFile_Bilinear;


    if (*(U32*)rawData[0] == 'QSPM')
        flags |= AnimFile_UseMorphSeq;

    if (linkedList != NULL) {
        afile->Next = *linkedList;
        *linkedList = afile;
    } else {
        afile->Next = NULL;
    }

    afile->RawData = (void**)(afile + 1);
    for (i = 0; i < (S32)(numX * numY); i++)
        afile->RawData[i] = rawData[i];

    afile->Name = name;
    afile->ID = xStrHash(name);
    afile->FileFlags = flags;
    afile->Duration = (afile->FileFlags & AnimFile_BackAndForth) ? (2.0f * xAnimFileGetRawDuration(afile)) : xAnimFileGetRawDuration(afile);
    afile->TimeOffset = 0.0f;
    afile->BoneCount = (flags & AnimFile_UseMorphSeq) ? 0 : iAnimBoneCount(rawData[0]);
    afile->NumAnims[0] = numX;
    afile->NumAnims[1] = numY;

    return afile;
}

xAnimFile* xAnimFileNew(void* rawData, const char* name, U32 flags, xAnimFile** linkedList)
{
    void** temp = &rawData;
    return xAnimFileNewBilinear(temp, name, flags, linkedList, 1, 1);
}

void xAnimFileSetTime(xAnimFile* data, F32 duration, F32 timeOffset)
{
    F32 rawDuration = xAnimFileGetRawDuration(data);
    if (timeOffset > rawDuration - 0.1f)
        timeOffset = rawDuration - 0.1f;


    rawDuration -= timeOffset; data->TimeOffset = timeOffset;
    if (duration > rawDuration)
        duration = rawDuration;
    data->Duration = (data->FileFlags & AnimFile_BackAndForth) ? (2.0f * duration) : (duration);
}

void xAnimFileEval(xAnimFile* data, F32 time, F32* bilinear, U32 flags, xVec3* tran, xQuat* quat, F32*) NONMATCH("https://decomp.me/scratch/ZvUwZ")
{
    S32 i;
    U32 numBones;



    xASSERT(data);
    U32 fileflags = data->FileFlags;
    time = xclamp(time, 0.0f, data->Duration);
    F32 evalTime = xAnimFileRawTime(data, time);


    if (data->FileFlags & AnimFile_UseMorphSeq)
        return;



    numBones = (flags & 0x1) ? 1 : data->BoneCount;
    if (flags & 0x2) numBones--;
    if (numBones == 0)
        return;

    if (bilinear != NULL && (fileflags & AnimFile_Bilinear)) {

        F32 bilerp[2];
        F32 lerp, tmpf;
        U32 biindex[2], biplus[2];
        for (i = 0; i < 2; i++) {
            lerp = xclamp(bilinear[i], 0.0f, data->NumAnims[i] - 1);
            tmpf = std::floorf(lerp);
            bilerp[i] = lerp - tmpf;
            biindex[i] = (U32)tmpf;
            biplus[i] = (biindex[i] + 1 < data->NumAnims[i]) ? (biindex[i] + 1) : biindex[i];
        }




        xQuat* q0 = (xQuat*)(giAnimScratch + 0x1560);
        xVec3* t0 = (xVec3*)((U8*)q0 + 0x410);
        if (bilerp[0] && bilerp[1]) {

            xQuat* q1 = (xQuat*)(giAnimScratch + 0x1C80);
            xVec3* t1 = (xVec3*)((U8*)q1 + 0x410);



            iAnimEval(data->RawData[biindex[0] + biindex[1] * data->NumAnims[0]], evalTime, flags, tran, quat);
            iAnimEval(data->RawData[biplus[0] + biindex[1] * data->NumAnims[0]], evalTime, flags, t0, q0);
            iAnimBlend(bilerp[0], 1.0f, NULL, NULL, numBones, tran, quat, t0, q0, tran, quat);

            iAnimEval(data->RawData[biindex[0] + biplus[1] * data->NumAnims[0]], evalTime, flags, t0, q0);
            iAnimEval(data->RawData[biplus[0] + biplus[1] * data->NumAnims[0]], evalTime, flags, t1, q1);
            iAnimBlend(bilerp[0], 1.0f, NULL, NULL, numBones, t0, q0, t1, q1, t0, q0);

            iAnimBlend(bilerp[1], 1.0f, NULL, NULL, numBones, tran, quat, t0, q0, tran, quat);

        } else if (bilerp[0]) {


            iAnimEval(data->RawData[biindex[0] + biindex[1] * data->NumAnims[0]], evalTime, flags, tran, quat);
            iAnimEval(data->RawData[biplus[0] + biindex[1] * data->NumAnims[0]], evalTime, flags, t0, q0);
            iAnimBlend(bilerp[0], 1.0f, NULL, NULL, numBones, tran, quat, t0, q0, tran, quat);

        } else if (bilerp[1]) {


            iAnimEval(data->RawData[biindex[0] + biindex[1] * data->NumAnims[0]], evalTime, flags, tran, quat);
            iAnimEval(data->RawData[biindex[0] + biplus[1] * data->NumAnims[0]], evalTime, flags, t0, q0);
            iAnimBlend(bilerp[0], 1.0f, NULL, NULL, numBones, tran, quat, t0, q0, tran, quat);

        } else {

            iAnimEval(data->RawData[biindex[0] + biindex[1] * data->NumAnims[0]], evalTime, flags, tran, quat);

        }
    } else {

        iAnimEval(data->RawData[0], evalTime, flags, tran, quat);
    }

}

xAnimEffect* xAnimStateNewEffect(xAnimState* state, U32 flags, F32 startTime, F32 endTime, xAnimEffectCallback callback, U32 userDataSize)
{
    xAnimEffect* curr, ** prev, * effect = (xAnimEffect*)xAnimMemAlloc(userDataSize + sizeof(xAnimEffect));
    effect->Flags = flags;
    effect->StartTime = startTime;
    effect->EndTime = endTime;
    effect->Callback = callback;


    prev = &state->Effects;
    curr = state->Effects;
    while (curr != NULL && startTime > curr->StartTime) {
        prev = &curr->Next;
        curr = curr->Next;
    }
    effect->Next = curr;
    *prev = effect;

    return effect;
}

xAnimTable* xAnimTableNew(const char* name, xAnimTable** linkedList, U32 userFlags)
{

    xAnimTable* table = (xAnimTable*)xMALLOC(sizeof(xAnimTable));
    if (linkedList != NULL) {
        table->Next = *linkedList;
        *linkedList = table;
    } else {
        table->Next = NULL;
    }
    table->Name = name;
    table->TransitionList = NULL;
    table->StateList = NULL;
    table->AnimIndex = 0;
    table->MorphIndex = 0;
    table->UserFlags = userFlags;
    return table;
}

void xAnimDefaultBeforeEnter(xAnimPlay*, xAnimState* state)
{
    if (state->MultiFile != NULL) {
        U32 entry = rand() % state->MultiFile->Count;
        state->Data = state->MultiFile->Files[entry].File;
    }
}

xAnimState* xAnimTableNewState(xAnimTable* table, const char* name, U32 flags, U32 userFlags, F32 speed, F32* boneBlend, F32* timeSnap, F32 fadeRecip, U16* fadeOffset, void* callbackData, xAnimBeforeEnterCallback beforeEnter, xAnimStateCallback stateCallback, xAnimBeforeAnimMatricesCallback beforeAnimMatrices) NONMATCH("https://decomp.me/scratch/ZvUwZ")
{


    xAnimState* state = (xAnimState*)xAnimMemAlloc(sizeof(xAnimState));
    if (table->StateList == NULL) {
        state->Next = NULL;
        table->StateList = state;
    } else {
        state->Next = table->StateList->Next;
        table->StateList->Next = state;
    }

#ifdef DEBUG
    xASSERTMSG(TableHasStateID(table, xStrHash(name)) == 0, "The animation state already exists");
#endif

    state->Name = name;
    state->ID = xStrHash(name);
    state->Flags = flags;
    state->UserFlags = userFlags;
    state->Speed = speed;
    state->Data = NULL;
    state->Effects = NULL;
    state->Default = NULL;
    state->List = NULL;
    state->BoneBlend = boneBlend;
    state->TimeSnap = timeSnap;
    state->FadeRecip = fadeOffset ? CalcRecipBlendMax(fadeOffset) : fadeRecip;
    state->FadeOffset = fadeOffset;

    state->MultiFile = NULL;

    state->CallbackData = callbackData;
    state->BeforeEnter = beforeEnter;
    state->StateCallback = stateCallback;
    state->BeforeAnimMatrices = beforeAnimMatrices;

    return state;
}

static void _xAnimTableAddTransitionHelper(xAnimState* state, xAnimTransition* tran, U32& allocCount, U32& stateCount, xAnimState** stateList)
{
    U32 flags = tran->Flags;

    if (flags & AnimTransition_0x10) {
        if (!DefaultHasTransition(state, tran, &allocCount)) {
            stateList[stateCount] = state;
            stateCount++;
        }
    } else {
        if (!StateHasTransition(state, tran)) {
            stateList[stateCount] = state;
            stateCount++;
            allocCount++;
        }
    }
}

static void _xAnimTableAddTransition(xAnimTable* table, xAnimTransition* tran, const char* source, const char* dest) NONMATCH("https://decomp.me/scratch/ZvUwZ")
{
    U8* buffer;
    xAnimState** stateList, * state;
    U32 i, stateCount, allocCount = 0;
    const char* stateName;
    xAnimTransitionList* tlist;
    xAnimTransition* substTransitionList[32];



    U32 substTransitionCount = 0;


    buffer = giAnimScratch;
    xASSERT(buffer != 0);

    stateCount = 0;
    stateList = (xAnimState**)(buffer + 0x400);
    bool hasSubst = false;

    if (dest != NULL) {
        for (S32 i = 0; dest[i] != '\0'; i++)
            if (dest[i] == '@' || dest[i] == '~') {
                hasSubst = true;
                break;
            }
    }




    stateName = xStrTokBuffer(source, " ,\t\n\r", buffer);
    while (stateName != NULL) {

        bool isComplex = (dest != NULL);
        if (!isComplex) {
            const char* COMPLEX_PATTERNS = "#+*?{}()<>|;";
            for (const char* search = stateName; *search != '\0'; search++) {
                if (_xCharIn(*search, COMPLEX_PATTERNS)) {
                    isComplex = true;
                    break;
                }
            }
        }


        if (isComplex) {

            xAnimState* state = table->StateList;
            S32 count = 0;
            while (state != NULL) {
                char extra[128];
                if (_xCheckAnimName(state->Name, stateName, extra)) {



                    if (hasSubst) {

                        char tempName[128];
                        char* tempIterator = tempName;
                        char* extraIterator = extra;
                        bool allowMissingState = false;
                        for (S32 i = 0; dest[i] != '\0'; i++)
                            if (dest[i] == '@' || dest[i] == '~') {
                                allowMissingState = (dest[i] == '~');
                                xASSERTMSG(*extraIterator != 1, "Missing argument for subsitutition");
                                size_t extraIteratorLength = strlen(extraIterator);
                                strcpy(tempIterator, extraIterator);
                                tempIterator += extraIteratorLength;
                                extraIterator += extraIteratorLength + 1;
                            } else
                                *(tempIterator++) = dest[i];
                            *tempIterator = '\0';


                            xAnimState* tempState = xAnimTableGetState(table, tempName);
                            if (!allowMissingState || tempState != NULL) {

                                xASSERTFMT(tempState != NULL, "Missing state: %s", tempName);


                                if (substTransitionCount) {
                                    xAnimTransition* duplicatedTransition = (xAnimTransition*)xAnimMemAlloc(sizeof(xAnimTransition));
                                    memcpy(duplicatedTransition, tran, sizeof(xAnimTransition));
                                    tran = duplicatedTransition;
                                }
                                xASSERT(substTransitionCount < MAX_SUBST_TRANSITION);
                                substTransitionList[substTransitionCount++] = tran;


                                tran->Dest = tempState;
                            } else
                                goto skip;
                    }
                    if (tran->Dest != state)
                        _xAnimTableAddTransitionHelper(state, tran, allocCount, stateCount, stateList);
                    count++;
                }
            skip:
                state = state->Next;
            }
            xASSERTMSG(count > 0, "no matched transitions");
        } else {
            xASSERT(!hasSubst);
            state = xAnimTableGetState(table, stateName);
            xASSERTFMT(state != NULL, "state %s not found", stateName);
            if (state != NULL) {

                if (tran->Dest != state) {
                    _xAnimTableAddTransitionHelper(state, tran, allocCount, stateCount, stateList);
                }
            }
        }


        stateName = xStrTokBuffer(NULL, " ,\t\n\r", buffer);
    }

    if (allocCount)
        tlist = (xAnimTransitionList*)xAnimMemAlloc(allocCount * sizeof(xAnimTransitionList));


    if (tran->Flags & AnimTransition_0x10) {
        for (i = 0; i < stateCount; i++) {
            if (!DefaultOverride(stateList[i], tran)) {
                if (tran->Conditional == NULL && stateList[i]->Default != NULL) {
                    xAnimTransitionList* curr = stateList[i]->Default;
                    while (curr->Next != NULL)
                        curr = curr->Next;
                    tlist->T = hasSubst ? substTransitionList[i] : tran;
                    tlist->Next = NULL;
                    curr->Next = tlist;
                    tlist++;
                } else {
                    tlist->T = hasSubst ? substTransitionList[i] : tran;
                    tlist->Next = stateList[i]->Default;
                    stateList[i]->Default = tlist;
                    tlist++;
                }
            }
        }

    } else {
        xASSERT(allocCount >= stateCount);
        for (i = 0; i < stateCount; i++) {
            tlist->T = hasSubst ? substTransitionList[i] : tran;
            tlist->Next = stateList[i]->List;
            stateList[i]->List = tlist;
            tlist++;
        }
    }
}

void xAnimTableAddTransition(xAnimTable* table, xAnimTransition* tran, const char* source)
{
    _xAnimTableAddTransition(table, tran, source, NULL);
}

xAnimTransition* xAnimTableNewTransition(xAnimTable* table, const char* source, const char* dest, xAnimTransitionConditionalCallback conditional, xAnimTransitionCallback callback, U32 flags, U32 userFlags, F32 srcTime, F32 destTime, U16 priority, U16 queuePriority, F32 fBlendTime, U16* blendOffset) NONMATCH("https://decomp.me/scratch/ZvUwZ")
{
    xAnimTransition* tran = (xAnimTransition*)xAnimMemAlloc(sizeof(xAnimTransition));
    if (fBlendTime != 0.0f) fBlendTime = (1.0f / fBlendTime);
    tran->Next = table->TransitionList;
    table->TransitionList = tran;

    bool isComplex = false;
    if (dest == NULL || *dest == '\0') {
        tran->Dest = NULL;
    } else {
        for (S32 i = 0; dest[i] != '\0'; i++) {
            if (dest[i] == '@' || dest[i] == '~') {
                isComplex = true;
                break;
            }
        }

        tran->Dest = isComplex ? NULL : xAnimTableGetState(table, dest);
        xASSERTFMT((isComplex || tran->Dest != NULL), "destination state not found %s", dest);
    }
    tran->Conditional = conditional;
    tran->Callback = callback;
    tran->Flags = flags;
    tran->UserFlags = userFlags;
    tran->SrcTime = srcTime;
    tran->DestTime = destTime;
    tran->Priority = priority;
    tran->QueuePriority = queuePriority;
    tran->BlendRecip = blendOffset ? CalcRecipBlendMax(blendOffset) : fBlendTime;
    tran->BlendOffset = blendOffset;

    _xAnimTableAddTransition(table, tran, source, isComplex ? dest : NULL);
    return tran;
}

void xAnimTableAddFile(xAnimTable* table, xAnimFile* file, const char* states)
{
    U8* buffer = giAnimScratch;
    xASSERT(buffer != 0);


    const char* stateName = xStrTokBuffer(states, " ,\t\n\r", buffer);
    while (stateName != NULL) {
        xAnimTableAddFileID(table, file, xStrHash(stateName), 0, 0);
        stateName = xStrTokBuffer(NULL, " ,\t\n\r", buffer);
    }
}

xAnimState* xAnimTableAddFileID(xAnimTable* table, xAnimFile* file, U32 stateID, U32 subStateID, U32 subStateCount)
{
    xAnimState* state = xAnimTableGetStateID(table, stateID);
    if (state != NULL) {

        bool setFile = true;
        if (subStateID != 0) {

            if (state->MultiFile == NULL) {
                state->MultiFile = (xAnimMultiFile*)xAnimMemAlloc(subStateCount * sizeof(xAnimMultiFileEntry) + sizeof(xAnimMultiFileBase));
                state->MultiFile->Count = 0;
            }


            U32 current = state->MultiFile->Count;
            xASSERT(current < subStateCount);
            state->MultiFile->Files[current].ID = subStateID;
            state->MultiFile->Files[current].File = file;
            state->MultiFile->Count++;
        }
        xASSERTMSG(subStateID != 0 || state->MultiFile == NULL, "It looks like you have both substates and regular animations");


        if (setFile) {
            state->Data = file;
        }

        if (file->FileFlags & AnimFile_UseMorphSeq) {
            table->MorphIndex |= (1 << (state->Flags & AnimState_0xFMask));
        } else {
            table->AnimIndex |= (1 << (state->Flags & AnimState_0xFMask));
        }


        xASSERT((table->MorphIndex & table->AnimIndex) == 0);
    }
    return state;
}

xAnimState* xAnimTableGetStateID(xAnimTable* table, U32 ID)
{
    xAnimState* curr = table->StateList;
    while (curr != NULL) {
        if (curr->ID == ID)
            return curr;
        curr = curr->Next;
    }
    return NULL;
}

xAnimState* xAnimTableGetState(xAnimTable* table, const char* name)
{
    return xAnimTableGetStateID(table, xStrHash(name));
}

static void EffectActiveInsert(xAnimSingle* single, xAnimActiveEffect* active)
{
    U32 index = 0, count = single->ActiveCount;
    xAnimActiveEffect* alist = single->ActiveList;

    while (index < count && alist->Effect != NULL) { alist++; index++; }
    xASSERT(index < count);

    *alist = *active;
    if (index < count - 1) (alist + 1)->Effect = NULL;
}

static void EffectActiveRemove(xAnimActiveEffect* active, U32 index, U32 count)
{

    if (index == count - 1 || (active + 1)->Effect == NULL) {
        active->Effect = NULL;
        return;
    }

    xAnimActiveEffect* alist = active;
    while (index < count - 1 && (alist + 1)->Effect != NULL) { alist++; index++; }

    *active = *alist;
    alist->Effect = NULL;
}

static U32 EffectPlaying(xAnimSingle* single, xAnimEffect* effect)
{

    for (U32 i = 0; i < single->ActiveCount && single->ActiveList[i].Effect != NULL; i++)
        if (single->ActiveList[i].Effect == effect)
            return TRUE;
    return FALSE;
}

static void EffectSingleStart(xAnimSingle* single)
{


    xAnimEffect* effect = single->State->Effects;
    xAnimActiveEffect tempActive;
    F32 time = single->Time;

    while (effect != NULL && effect->StartTime < time) {

        U32 flags = effect->Flags;
        if (flags & AnimEffect_0x1) {

            if (flags & AnimEffect_0x4) {


                tempActive.Effect = effect;
                tempActive.Handle = effect->Callback(1, &tempActive, single, single->Play->Object);
                if (effect->Flags & AnimEffect_0x2) {
                    if (effect->EndTime <= time) {
                        effect->Callback(3, &tempActive, single, single->Play->Object);
                    } else {
                        EffectActiveInsert(single, &tempActive);
                    }
                } else if (effect->Flags & AnimEffect_0x20) {


                    EffectActiveInsert(single, &tempActive);
                }

            } else if ((flags & AnimEffect_0x2) && !(flags & AnimEffect_0x10) && effect->EndTime > time) {



                tempActive.Effect = effect;
                tempActive.Handle = effect->Callback(1, &tempActive, single, single->Play->Object);
                EffectActiveInsert(single, &tempActive);
            }
        }

        effect = effect->Next;
    }

    single->Effect = effect;
}

static void EffectSingleDuration(xAnimSingle* single)
{
    F32 time = single->Time;
    xAnimActiveEffect* alist = single->ActiveList;
    U32 index = 0, count = single->ActiveCount;

    while (index < count && alist->Effect != NULL) {


        if (!(alist->Effect->Flags & AnimEffect_0x20) && alist->Effect->EndTime <= time) {

            alist->Effect->Callback(3, alist, single, single->Play->Object);
            EffectActiveRemove(alist, index, count);






        } else {
            alist->Effect->Callback(2, alist, single, single->Play->Object);
            alist++;
            index++;
        }
    }
}

static void EffectSingleRun(xAnimSingle* single)
{
    xAnimEffect* effect = single->Effect;
    xAnimActiveEffect tempActive;
    F32 time = single->Time;


    while (effect != NULL && effect->StartTime <= time) {
        U32 flags = effect->Flags;
        if ((flags & AnimEffect_0x1)) {

            if (!(flags & AnimEffect_0x20) || !EffectPlaying(single, effect)) {


                tempActive.Effect = effect;
                tempActive.Handle = effect->Callback(1, &tempActive, single, single->Play->Object);
                if (flags & AnimEffect_0x2) {
                    if (effect->EndTime <= time) {
                        effect->Callback(3, &tempActive, single, single->Play->Object);
                    } else {
                        EffectActiveInsert(single, &tempActive);
                    }
                } else if (flags & AnimEffect_0x20) {


                    EffectActiveInsert(single, &tempActive);
                }
            }
        }

        effect = effect->Next;
    }
    single->Effect = effect;
}

static void EffectSingleLoop(xAnimSingle* single) NONMATCH("https://decomp.me/scratch/ZvUwZ")
{

    EffectSingleRun(single);


    xAnimActiveEffect* alist = single->ActiveList;
    U32 index = 0, count = single->ActiveCount;

    while (index < count && alist->Effect != NULL) {
        if (!(alist->Effect->Flags & AnimEffect_0x20)) {

            alist->Effect->Callback(3, alist, single, single->Play->Object);
            EffectActiveRemove(alist, index, count);






        } else {
            alist++;
            index++;
        }
    }




    xAnimEffect* effect = single->State->Effects;
    while (effect != NULL && effect->StartTime < 0.0f) {
        effect = effect->Next;
    }
    single->Effect = effect;
}

static void EffectSingleStop(xAnimSingle* single)
{

    if (single->State == NULL || single->LastTime == -1.0f) {
        return;
    }


    for (U32 i = 0; i < single->ActiveCount && single->ActiveList[i].Effect != NULL; i++) {

        if (single->ActiveList[i].Effect->Flags & AnimEffect_0x2)
            single->ActiveList[i].Effect->Callback(3, &single->ActiveList[i], single, single->Play->Object);
    }
    single->ActiveList[0].Effect = NULL;


    xAnimEffect* effect = single->Effect;

    while (effect != NULL) {
        if ((effect->Flags & (AnimEffect_0x1 | AnimEffect_0x8)) == (AnimEffect_0x1 | AnimEffect_0x8)) {

            xAnimActiveEffect tempActive;
            tempActive.Effect = effect;
            tempActive.Handle = effect->Callback(1, &tempActive, single, single->Play->Object);
            if (effect->Flags & AnimEffect_0x2) {
                effect->Callback(3, &tempActive, single, single->Play->Object);
            }
        }
        effect = effect->Next;
    }
    single->Effect = NULL;
}

static void StopUpdate(xAnimSingle* single)
{
    F32 duration = single->State->Data->Duration;
    if (single->Time > duration) {
        single->Time = duration;
        single->CurrentSpeed = 0.0f;
    }
}

static void LoopUpdate(xAnimSingle* single)
{
    F32 time = single->Time;
    F32 duration = single->State->Data->Duration;


    if (single->Time > duration) {
        single->Time = duration;
        EffectSingleLoop(single);
        single->Time = time - duration;
    }
}

void xAnimPlaySetState(xAnimSingle* single, xAnimState* state, F32 startTime) NONMATCH("https://decomp.me/scratch/ZvUwZ")
{

    EffectSingleStop(single);
    if (single->Blend != NULL) {
        EffectSingleStop(single->Blend);
        single->Blend->State = NULL;
    }

    single->State = state;
    if (state == NULL) {
        return;
    }

    single->Time =
        ((state->Flags & AnimState_0x100) && startTime == 0.0f) ?
        (state->Data->Duration * xurand()) : (startTime);
    single->CurrentSpeed = state->Speed;
    single->BilinearLerp[0] = 0.0f;
    single->BilinearLerp[1] = 0.0f;
    single->Effect = NULL;
    memset(single->ActiveList, 0, single->ActiveCount * sizeof(xAnimActiveEffect));





    single->LastTime = -1.0f;
    single->Sync = NULL;
    if (single->Tran != NULL && (single->Tran->Flags & AnimTransition_0x2))
        xMemPoolFree(&sxAnimTempTranPool, single->Tran);
    single->Tran = NULL;
    single->BlendFactor = 0.0f;
}

static void SingleUpdate(xAnimSingle* single, F32 timeDelta) NONMATCH("https://decomp.me/scratch/ZvUwZ")
{
    U32 foundBlendstop = FALSE, firstStep = TRUE;
    xAnimTransition* foundTransition = NULL;
    xAnimSingle* bl = NULL;
    F32 tranDelta = 0.0f, blendDelta = 0.0f;


    if (single->State == NULL) {
        return;
    }

    F32 singleTime;
    void* object = single->Play->Object;

    if (single->LastTime == -1.0f) {
        EffectSingleStart(single);
    }


    single->LastTime = single->Time;
    singleTime = single->Time + timeDelta * single->CurrentSpeed;

    if (single->Blend != NULL && single->Blend->State != NULL) {
        bl = single->Blend;
        if (bl->LastTime == -1.0f) {
            EffectSingleStart(bl);
        }
        bl->LastTime = bl->Time;
    }


    F32 duration = single->State->Data->Duration;
    if (single->Sync != NULL) {

        if (single->Sync->SrcTime != 0.0f) {


            F32 timeCmp = single->Sync->SrcTime;
            if (timeCmp > duration) timeCmp = duration;
            if (single->LastTime <= timeCmp && singleTime >= timeCmp) {


                foundTransition = single->Sync;
                tranDelta = (singleTime - timeCmp) / single->CurrentSpeed;
                timeDelta -= tranDelta;
                if (timeDelta < 0.0f) timeDelta = 0.0f;

            } else if (single->LastTime <= timeCmp + duration && singleTime >= timeCmp + duration) {


                foundTransition = single->Sync;
                tranDelta = (singleTime - (timeCmp + duration)) / single->CurrentSpeed;
                timeDelta -= tranDelta;
                if (timeDelta < 0.0f) {
                    timeDelta = 0.0f;
                }
            }
        } else {



            if (bl == NULL) {
                foundTransition = single->Sync;
                tranDelta = timeDelta;
                timeDelta = 0.0f;
            }
        }

    } else {


        if ((single->State->Flags & (AnimState_0x10 | AnimState_0x20)) == AnimState_0x20) {


#ifdef DEBUG
            if (single->State->Default == NULL) {

                char tempText[1024];
                sprintf(tempText, "State \"%s\" missing end transition.\n", single->State->Name);
            }
#endif





            xASSERT(single->State);
            xASSERTFMT(single->State->Default, "'%s' end transition to nowhere", single->State->Name);
            F32 timeCmp = single->State->Default->T->SrcTime;
            if (timeCmp == 0.0f || timeCmp > duration) timeCmp = duration;

            if (singleTime >= timeCmp && (!(single->State->Default->T->Flags & AnimTransition_0x4) || bl == NULL)) {




                xAnimTransitionList* curr = single->State->Default;
                while (curr != NULL && curr->T->Conditional != NULL && curr->T->Conditional(curr->T, single, object) == 0)
                    curr = curr->Next;


                if (curr == NULL) {
                    fprintf(stderr, "State \"%s\" no default conditionals true!\n", single->State->Name);

                    curr = single->State->Default;
                }
                foundTransition = curr->T;
                if (single->LastTime < timeCmp) {
                    tranDelta = (singleTime - timeCmp) / single->CurrentSpeed;
                    timeDelta -= tranDelta;
                    if (timeDelta < 0.0f) timeDelta = 0.0f;
                } else {
                    tranDelta = timeDelta;
                    timeDelta = 0.0f;
                }
            }
        }
    }


    if (single->BlendFactor != 0.0f) {
        F32 recip;
        if (single->Tran != NULL)
            recip = single->Tran->BlendRecip;
        else
            recip = single->State->FadeRecip;

        if (recip * (single->BlendFactor + timeDelta) > 1.0f) {
            foundBlendstop = TRUE;
            blendDelta = single->BlendFactor + timeDelta - 1.0f / recip;
            timeDelta -= blendDelta;
            if (timeDelta < 0.0f) timeDelta = 0.0f;
            if (blendDelta < 0.0f) {
                blendDelta = 0.01f;
            }
        }
    }




    do {
        if (!firstStep) {

            if (foundBlendstop) {


                single->BlendFactor = 0.0f;

                if (bl != NULL) {
                    EffectSingleStop(bl);
                    bl->State = NULL;
                    bl = NULL;
                    if (single->Tran != NULL && (single->Tran->Flags & AnimTransition_0x2)) {
                        xMemPoolFree(&sxAnimTempTranPool, single->Tran);
                    }
                    single->Tran = NULL;
                } else {
                    if (single->Tran != NULL) {
                        if (single->Tran != NULL && (single->Tran->Flags & AnimTransition_0x2)) {
                            xMemPoolFree(&sxAnimTempTranPool, single->Tran);
                        }
                        single->Tran = NULL;
                    } else {

                        EffectSingleStop(single);
                        single->State = NULL;
                        return;
                    }
                }


                timeDelta = blendDelta;
                foundBlendstop = FALSE;

            } else {



                if (bl != NULL) {
                    EffectSingleStop(bl);
                    bl->State = NULL;
                    bl = NULL;
                    single->BlendFactor = 0.0f;
                }

                if (foundTransition->BlendRecip == 0.0f || single->Blend == NULL) {


                    EffectSingleStop(single);
                    if (single->Tran && (single->Tran->Flags & AnimTransition_0x2)) {
                        xMemPoolFree(&sxAnimTempTranPool, single->Tran);
                    }
                    single->Tran = NULL;

                } else {


                    bl = single->Blend;
                    bl->State = single->State;






                    bl->Time = single->Time,
                        bl->CurrentSpeed = single->CurrentSpeed,
                        bl->BilinearLerp[0] = single->BilinearLerp[0],
                        bl->BilinearLerp[1] = single->BilinearLerp[1],
                        bl->Effect = single->Effect,
                        bl->LastTime = single->LastTime;


                    xASSERT(bl->ActiveCount == single->ActiveCount);
                    memcpy(bl->ActiveList, single->ActiveList, single->ActiveCount * sizeof(xAnimActiveEffect));
                    single->ActiveList[0].Effect = NULL;


                    if (single->Tran && (single->Tran->Flags & AnimTransition_0x2)) {
                        xMemPoolFree(&sxAnimTempTranPool, single->Tran);
                    }
                    single->Tran = foundTransition;
                    single->BlendFactor = 0.0000001f;
                }



                TransitionTimeInit(single, foundTransition);


                single->State = foundTransition->Dest;
                single->CurrentSpeed = single->State->Speed,
                    single->BilinearLerp[0] = 0.0f,
                    single->BilinearLerp[1] = 0.0f;
                single->Sync = NULL;



                EffectSingleStart(single);


                if (foundTransition->Dest->BeforeEnter != NULL) {
                    foundTransition->Dest->BeforeEnter(single->Play, foundTransition->Dest);
                }

                if (foundTransition->Callback != NULL) {
                    foundTransition->Callback(foundTransition, single, single->Play->Object);
                }

                timeDelta = tranDelta;
                foundTransition = NULL;
            }
        }




        single->Time += timeDelta * single->CurrentSpeed;
        if (single->BlendFactor != 0.0f) {
            single->BlendFactor += timeDelta;
        }
        if ((single->State->Flags & (AnimState_0x10 | AnimState_0x20)) == AnimState_0x10) {
            LoopUpdate(single);
        } else {
            StopUpdate(single);
        }

        EffectSingleRun(single);

        if (bl != NULL) {
            if ((bl->State->Flags & (AnimState_0x10 | AnimState_0x20)) == AnimState_0x10) {
                LoopUpdate(bl);
            } else {
                StopUpdate(bl);
            }
            EffectSingleRun(bl);
        }
        firstStep = FALSE;


    } while (foundBlendstop || foundTransition != NULL);


    if (single->Tran == NULL && single->BlendFactor == 0.0f && (single->State->Flags & (AnimState_0x10 | AnimState_0x20)) == AnimState_0x30) {


        if (single->State->Flags & AnimState_0x200) {
            if (single->Time >= duration)
                single->BlendFactor = 0.0000001f;
        } else {
            if (single->State->FadeRecip * (duration - single->Time) < 1.0f)
                single->BlendFactor = 0.0000001f;
        }
    }


    EffectSingleDuration(single);
    if (bl != NULL) EffectSingleDuration(bl);
}

static void SingleEval(xAnimSingle* single, xVec3* tran, xQuat* quat)
{
    if (single->Play->BoneCount <= 1) {
        return;
    }

    tran++;
    quat++;
    xAnimFileEval(single->State->Data, single->Time, single->BilinearLerp, 0x2, tran, quat, NULL);


    if (single->Blend != NULL && single->Blend->State != NULL) {
        xQuat* blendquat = (xQuat*)(giAnimScratch + 0xE40);
        xVec3* blendtran = (xVec3*)((U8*)blendquat + 0x410);

        xAnimFileEval(single->Blend->State->Data, single->Blend->Time, single->Blend->BilinearLerp, 0x2, blendtran, blendquat, NULL);


        iAnimBlend(single->BlendFactor, single->Tran->BlendRecip, single->Tran->BlendOffset, NULL, single->Play->BoneCount - 1, blendtran, blendquat, tran, quat, tran, quat);
    }


}

void xAnimPlaySetup(xAnimPlay* play, void* object, xAnimTable* table, xModelInstance* modelInst)
{

    play->BoneCount = modelInst->BoneCount;
    play->Object = object;
    play->Table = table;
    play->ModelInst = modelInst;


    modelInst->Anim = play;
    modelInst->Flags |= (ModelInstance_0x4 | ModelInstance_0x100);



    if (table->MorphIndex != 0) {
        modelInst->Flags |= ModelInstance_0x80;
    }


    for (S32 i = 0; i < play->NumSingle; i++) {

        play->Single[i].SingleFlags = ((1 << i) & table->MorphIndex) ? AnimSingle_0x8000 : AnimSingle_0x1;


        play->Single[i].State = NULL;
        play->Single[i].Tran = NULL;
        if (play->Single[i].Blend)
            play->Single[i].Blend->State = NULL;
    }


    xAnimPlaySetState(play->Single, table->StateList, 0.0f);
}

void xAnimPlayChooseTransition(xAnimPlay* play)
{
    U32 i;
    void* object = play->Object;

    U8* tmpBuffer = giAnimScratch;

    xAnimTransition** found = (xAnimTransition**)tmpBuffer;
    memset(found, 0, play->NumSingle * sizeof(xAnimTransition*));


    for (i = 0; i < play->NumSingle; i++) {
        if (play->Single[i].State != NULL) {
            xAnimTransitionList* curr = play->Single[i].State->List;

            if (curr == NULL) continue;
            if (curr->T->Conditional == NULL) continue;
            while (curr != NULL) {
                xASSERT(curr->T->Conditional);
                if (curr->T->Conditional(curr->T, &play->Single[i], object)) {

                    U32 index = curr->T->Dest->Flags & AnimState_0xFMask;

                    xASSERT(index < play->NumSingle);
                    if (found[index] == NULL || curr->T->Priority > found[index]->Priority) {
                        found[index] = curr->T;
                    }
                }
                curr = curr->Next;
            }
        }
    }


    for (i = 0; i < play->NumSingle; i++) {
        if (found[i] != NULL && (play->Single[i].Sync == NULL || found[i]->Priority > play->Single[i].Sync->QueuePriority)) {
            xAnimPlayStartTransition(play, found[i]);
        }
    }
}

void xAnimPlayStartTransition(xAnimPlay* play, xAnimTransition* transition) NONMATCH("https://decomp.me/scratch/ZvUwZ")
{
    xASSERT(transition->Dest);

    U32 index = transition->Dest->Flags & AnimState_0xFMask;

    xASSERT(index < play->NumSingle);



    xAnimSingle* single = &play->Single[index];
    xAnimSingle* bl = single->Blend;
    if (transition->SrcTime != 0.0f || ((transition->Flags & AnimTransition_0x4) && bl != NULL && bl->State != NULL)) {


        single->Sync = transition;
        return;
    }


    if (bl != NULL && bl->State != NULL) {
        EffectSingleStop(bl);
        bl->State = NULL;
        single->BlendFactor = 0.0f;
    }

    if (transition->BlendRecip == 0.0f || single->Blend == NULL) {


        EffectSingleStop(single);
        if (single->Tran && (single->Tran->Flags & AnimTransition_0x2)) {
            xMemPoolFree(&sxAnimTempTranPool, single->Tran);
        }
        single->Tran = NULL;

    } else {

        if (single->State != NULL) {


            bl->State = single->State;
            bl->Time = single->Time,
                bl->CurrentSpeed = single->CurrentSpeed,
                bl->BilinearLerp[0] = single->BilinearLerp[0],
                bl->BilinearLerp[1] = single->BilinearLerp[1],
                bl->Effect = single->Effect,
                bl->LastTime = single->LastTime;

            xASSERT(bl->ActiveCount == single->ActiveCount);
            memcpy(bl->ActiveList, single->ActiveList, single->ActiveCount * sizeof(xAnimActiveEffect));
            single->ActiveList[0].Effect = NULL;
        }


        if (single->Tran && (single->Tran->Flags & AnimTransition_0x2)) {
            xMemPoolFree(&sxAnimTempTranPool, single->Tran);
        }
        single->Tran = transition;
        single->BlendFactor = 0.0000001f;
    }


    TransitionTimeInit(single, transition);


    single->State = transition->Dest;
    single->CurrentSpeed = single->State->Speed,
        single->BilinearLerp[0] = 0.0f,
        single->BilinearLerp[1] = 0.0f;
    single->Sync = NULL;



    EffectSingleStart(single);



    if (transition->Dest->BeforeEnter != NULL) {
        transition->Dest->BeforeEnter(play, transition->Dest);
    }

    if (transition->Callback != NULL) {
        transition->Callback(transition, single, single->Play->Object);
    }
}

void xAnimPlayUpdate(xAnimPlay* play, F32 timeDelta)
{


    for (U32 i = 0; i < play->NumSingle; i++) {
        xAnimSingle* single = &play->Single[i];

        SingleUpdate(single, timeDelta);


        if (single->State->StateCallback != NULL)
            single->State->StateCallback(single->State, single, play->Object);
    }
}

void xAnimPlayEval(xAnimPlay* play)
{







    U32 i, bone;
    xQuat* quatresult = (xQuat*)giAnimScratch;
    xVec3* tranresult = (xVec3*)((U8*)quatresult + 0x410);
    if (play->BoneCount > 1) {

        xQuat* quatblend = (xQuat*)(giAnimScratch + 0x720);
        xVec3* tranblend = (xVec3*)((U8*)quatblend + 0x410);

        SingleEval(play->Single, tranresult, quatresult);


        for (i = 1; i < play->NumSingle; i++) {
            xAnimSingle* si = &play->Single[i];
            if (si->State != NULL && !(si->SingleFlags & AnimSingle_0x8000)) {

                F32 blendF = 1.0f, blendR = 1.0f;
                U16* blendO = NULL;
                SingleEval(si, tranblend, quatblend);

                if ((si->Blend == NULL || si->Blend->State == NULL) && si->BlendFactor) {
                    if (si->Tran != NULL) {

                        blendF = si->BlendFactor;
                        blendR = si->Tran->BlendRecip;
                        blendO = si->Tran->BlendOffset;
                    } else {

                        blendF = -si->BlendFactor;
                        blendR = si->State->FadeRecip;
                        blendO = si->State->FadeOffset;
                    }
                }

                if ((si->SingleFlags & (AnimSingle_0x1 | AnimSingle_0x2)) == AnimSingle_0x2) {


                    iAnimBlend(blendF, blendR, blendO, si->State->BoneBlend, play->BoneCount - 1, tranblend + 1, quatblend + 1, NULL, NULL, tranblend + 1, quatblend + 1);



                    for (bone = 1; bone < play->BoneCount; bone++) {
                        tranresult[bone].x += tranblend[bone].x;
                        tranresult[bone].y += tranblend[bone].y;
                        tranresult[bone].z += tranblend[bone].z;
                        xQuatMul(&quatresult[bone], &quatresult[bone], &quatblend[bone]);
                    }

                } else {


                    iAnimBlend(blendF, blendR, blendO, si->State->BoneBlend, play->BoneCount - 1, tranresult + 1, quatresult + 1, tranblend + 1, quatblend + 1, tranresult + 1, quatresult + 1);
                }
            }




        }
    }


    memset(tranresult, 0, sizeof(xVec3));
    memset(quatresult, 0, sizeof(xQuat));


    if (play->Single->State->BeforeAnimMatrices != NULL)
        play->Single->State->BeforeAnimMatrices(play, quatresult, tranresult, play->BoneCount);
    if (play->BeforeAnimMatrices != NULL)
        play->BeforeAnimMatrices(play, quatresult, tranresult, play->BoneCount);


    iModelAnimMatrices(play->ModelInst->Data, quatresult, tranresult, play->ModelInst->Mat + 1);
}

void xAnimPoolCB(xMemPool* pool, void* data)
{
    S32 i;
    xAnimPlay* clone = (xAnimPlay*)pool->Buffer;
    xAnimPlay* play = (xAnimPlay*)data;
    xAnimSingle* clonesingle, * currsingle = (xAnimSingle*)(play + 1);

    play->NumSingle = clone->NumSingle;
    play->Single = currsingle;
    currsingle += clone->NumSingle;



    for (i = 0; i < clone->NumSingle; i++) {
        play->Single[i].Play = play;
        if (clone->Single[i].Blend != NULL) {
            play->Single[i].Blend = currsingle;
            currsingle->Blend = NULL;
            currsingle->Play = play;
            currsingle++;
        } else {
            play->Single[i].Blend = NULL;
        }
    }


    xAnimActiveEffect* curract = (xAnimActiveEffect*)currsingle;

    for (i = 0; i < play->NumSingle; i++) {

        currsingle = &play->Single[i];
        clonesingle = &clone->Single[i];

        while (currsingle != NULL) {
            currsingle->ActiveCount = clonesingle->ActiveCount;
            if (clonesingle->ActiveCount) {
                currsingle->ActiveList = curract;
                curract += clonesingle->ActiveCount;
            } else {
                currsingle->ActiveList = NULL;
            }

            clonesingle = clonesingle->Blend;
            currsingle = currsingle->Blend;
        }
    }


    xASSERT(((U8*)curract - (U8*)data) == pool->Size);


    play->Pool = pool;
}

#define _xAnimPoolInitMacro1(blendFlags, singles, x, y)\
    (((blendFlags) & 0xFFFF & ((1 << (singles)) - 1) >> (x)) >> (y) & 0x1)
#define _xAnimPoolInitMacro2(blendFlags, singles, x)\
    (_xAnimPoolInitMacro1((blendFlags), (singles), (x), 0) +\
     _xAnimPoolInitMacro1((blendFlags), (singles), (x), 1) +\
     _xAnimPoolInitMacro1((blendFlags), (singles), (x), 2) +\
     _xAnimPoolInitMacro1((blendFlags), (singles), (x), 3))
#define _xAnimPoolInitMacro3(blendFlags, singles)\
    (_xAnimPoolInitMacro2((blendFlags), (singles), 0) +\
     _xAnimPoolInitMacro2((blendFlags), (singles), 4) +\
     _xAnimPoolInitMacro2((blendFlags), (singles), 8) +\
     _xAnimPoolInitMacro2((blendFlags), (singles), 12))

void xAnimPoolInit(xMemPool* pool, U32 count, U32 singles, U32 blendFlags, U32 effectMax)
{
    U32 size, i;









    effectMax += effectMax % 2;
    size = sizeof(xAnimPlay) + (effectMax * sizeof(xAnimActiveEffect) + sizeof(xAnimSingle)) * (_xAnimPoolInitMacro3(blendFlags, singles) + singles);
    void* buffer = xMALLOC(count * size);
    xAnimPlay* play = (xAnimPlay*)buffer;
    xAnimSingle* currsingle = (xAnimSingle*)(play + 1);
    play->NumSingle = singles;
    play->Single = currsingle;

    currsingle += singles;







    for (i = 0; i < singles; i++) {
        if (blendFlags & (1 << i)) {
            play->Single[i].Blend = currsingle;
            currsingle->Blend = NULL;
            currsingle++;
        } else {
            play->Single[i].Blend = NULL;
        }
    }


    xAnimActiveEffect* curract = (xAnimActiveEffect*)currsingle;

    for (i = 0; i < play->NumSingle; i++) {

        currsingle = &play->Single[i];

        while (currsingle != NULL) {
            currsingle->ActiveCount = effectMax;
            if (effectMax != 0) {
                currsingle->ActiveList = curract;
                curract += effectMax;
            } else {
                currsingle->ActiveList = NULL;
            }
            currsingle = currsingle->Blend;
        }
    }


    xASSERT((U32)((U8*)curract - (U8*)buffer) == size);






    play->Pool = pool;
    xMemPoolSetup(pool, buffer, 0, 0x1, xAnimPoolCB, size, count, count >> 1);

}

xAnimPlay* xAnimPoolAlloc(xMemPool* pool, void* object, xAnimTable* table, xModelInstance* modelInst)
{
    xAnimPlay* play = (xAnimPlay*)xMemPoolAlloc(pool);
    xAnimPlaySetup(play, object, table, modelInst);
    return play;
}

void xAnimPoolFree(xAnimPlay* play)
{



    for (U32 i = 0; i < play->NumSingle; i++) {
        EffectSingleStop(&play->Single[i]);
        if (play->Single[i].Blend != NULL) {
            EffectSingleStop(play->Single[i].Blend);
        }
    }


    xMemPoolFree(play->Pool, play);
}
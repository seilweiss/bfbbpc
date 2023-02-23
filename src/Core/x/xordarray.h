#pragma once

#include "types.h"

typedef S32(*XOrdTestCallback)(const void*, void*);
typedef S32(*XOrdCompareCallback)(void*, void*);

typedef struct st_XORDEREDARRAY XORDEREDARRAY;
struct st_XORDEREDARRAY
{
    void** list;
    S32 cnt;
    S32 max;
    S32 warnlvl;
};

void XOrdInit(XORDEREDARRAY* array, S32 size, S32 tempAlloc);
void XOrdReset(XORDEREDARRAY* array);
void XOrdDone(XORDEREDARRAY* array, S32 wasTempAlloc);
void XOrdAppend(XORDEREDARRAY* array, void* elt);
void XOrdInsert(XORDEREDARRAY* array, void* elt, XOrdCompareCallback compare);
void* XOrdRemove(XORDEREDARRAY* array, void* elt, S32 index);
S32 XOrdLookup(XORDEREDARRAY* array, const void* key, XOrdTestCallback test);
void XOrdSort(XORDEREDARRAY* array, XOrdCompareCallback test);
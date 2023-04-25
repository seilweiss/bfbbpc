#include "xordarray.h"

#include "xMemMgr.h"
#include "xMath.h"

void XOrdInit(st_XORDEREDARRAY* array, S32 size, S32 tempAlloc) NONMATCH("https://decomp.me/scratch/JVhIL")
{
    size = (size >= 1) ? size : 1;
    if (tempAlloc) {
        array->list = (void**)xMemPushTemp(size * sizeof(void*));
    } else {
        array->list = (void**)xMALLOC(size * sizeof(void*));
    }
    array->cnt = 0;
    array->max = size;
    array->warnlvl = (S32)(0.95f * size);
    if (array->warnlvl == array->max) {
        array->warnlvl = xmax(0, array->max - 1);
    }
}

void XOrdReset(st_XORDEREDARRAY* array)
{
    array->cnt = 0;
}

void XOrdDone(st_XORDEREDARRAY* array, S32 wasTempAlloc)
{
    if (array->max && wasTempAlloc) {
        xMemPopTemp(array->list);
    }
    array->list = NULL;
    array->cnt = 0;
    array->max = 0;
    array->warnlvl = 0;
}

void XOrdAppend(st_XORDEREDARRAY* array, void* elt)
{
    if (array->cnt >= array->max) return;
    array->list[array->cnt++] = elt;
}

void XOrdInsert(st_XORDEREDARRAY* array, void* elt, XOrdCompareCallback compare)
{
    S32 i = 0;
    
    if (array->cnt >= array->max) return;
    array->cnt++;
    
    for (i = array->cnt - 1; i > 0; i--) {
        if (compare(array->list[i-1], elt) <= 0) {
            array->list[i] = elt;
            return;
        }
        array->list[i] = array->list[i-1];
    }

    array->list[0] = elt;
}

void* XOrdRemove(st_XORDEREDARRAY* array, void* elt, S32 index)
{
    S32 i = 0;

    if (!elt) {
        if (index < 0) return NULL;
        if (index >= array->max) return NULL;
    }

    if (index >= 0 && index < array->max) {
        elt = array->list[index];
    } else if (elt) {
        index = -1;
        for (i = 0; i < array->cnt; i++) {
            if (array->list[i] == elt) {
                index = i;
                break;
            }
        }
    }
    if (index < 0) return NULL;

    array->cnt--;
    
    for (i = index; i < array->cnt; i++) {
        array->list[i] = array->list[i+1];
    }
    
    return elt;
}

S32 XOrdLookup(st_XORDEREDARRAY* array, const void* key, XOrdTestCallback test)
{
    S32 da_idx = -1;
    S32 k0 = 0;
    S32 k1 = 0;
    S32 k = 0;
    S32 v = 0;

    k0 = 0;
    k1 = array->cnt;
    while (k1 > k0) {
        k = (k0 + k1) / 2;
        v = test(key, array->list[k]);
        if (v == 0) {
            da_idx = k;
            break;
        }
        if (v > 0) k0 = k + 1;
        else k1 = k;
    }

    return da_idx;
}

void XOrdSort(st_XORDEREDARRAY* array, XOrdCompareCallback test)
{
    void** list = array->list;
    S32 num = array->cnt;
    S32 i = 0;
    S32 j = 0;
    S32 h = 1;
    void* v = NULL;

    while (h <= num) h = h * 3 + 1;
    while (h != 1) {
        h /= 3;
        for (i = h; i < num; i++) {
            v = list[i];
            for (j = i; j >= h && test(v, list[j-h]) < 0; j -= h) {
                list[j] = list[j-h];
            }
            list[j] = v;
        }
    }
}
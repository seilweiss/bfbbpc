#pragma once

#include "iMemMgr.h"

#define XMEMMAXSTACK 12

typedef U32 xMemAddr;

enum
{
    XMEM_HEAP_0,
    XMEM_HEAP_1,
    XMEM_HEAP_2,
    XMEM_MAX_HEAPS
};

typedef struct xMemArea_tag
{
    xMemAddr addr;
    U32 size;
    U32 flags;
} xMemArea;

#define XMEMAREA_0x20 0x20
#define XMEMAREA_0x40 0x40
#define XMEMAREA_0x200 0x200
#define XMEMAREA_0x400 0x400
#define XMEMAREA_0x800 0x800

typedef struct xMemInfo_tag
{
    xMemArea system;
    xMemArea stack;
    xMemArea DRAM;
    xMemArea SRAM;
} xMemInfo;

typedef struct xHeapState_tag
{
    xMemAddr curr;
    U16 blk_ct;
    U16 pad;
    U32 used;
    U32 wasted;
} xHeapState;

typedef struct xMemBlock_tag
{
    xMemAddr addr;
    U32 size;
    S32 align;
} xMemBlock;

typedef struct xMemHeap_tag
{
    U32 flags;
    xMemAddr hard_base;
    U32 size;
    S16 opp_heap[XMEM_MAX_HEAPS-1]; // opposite heaps
    xHeapState state[XMEMMAXSTACK];
    U16 state_idx;
    U16 max_blks;
    xMemBlock* blk;
    xMemBlock* lastblk;
} xMemHeap;

#define XMEMHEAP_0x1 0x1
#define XMEMHEAP_0x2 0x2
#define XMEMHEAP_0x4 0x4
#define XMEMHEAP_0x8 0x8
#define XMEMHEAP_0x20 0x20
#define XMEMHEAP_0x100 0x100
#define XMEMHEAP_0x3E00(x) (((x) & 0x1F) << 9)
#define XMEMHEAP_0x4000(x) (((x) & 0x1) << 14)
#define XMEMHEAP_0x8000(x) (((x) & 0x1) << 15)
#define XMEMHEAP_0x10000 0x10000
#define XMEMHEAP_GET_0x3E00(flags) (S32)(((flags) >> 9) & 0x1F)
#define XMEMHEAP_GET_0x4000(flags) (S32)(((flags) >> 14) & 0x1)
#define XMEMHEAP_GET_0x8000(flags) (S32)(((flags) >> 15) & 0x1)

struct xMemPool;
typedef void(*xMemPoolInitCallback)(xMemPool*, void*);

struct xMemPool
{
    void* FreeList;
    U16 NextOffset;
    U16 Flags;
    void* UsedList;
    xMemPoolInitCallback InitCB;
    void* Buffer;
    U16 Size;
    U16 NumRealloc;
    U32 Total;
};

#define XMEMPOOL_0x1 0x1

typedef void(*xMemBaseNotifyFunc)();

extern xMemInfo gMemInfo;
extern U32 gActiveHeap;
extern xMemHeap gxHeap[XMEM_MAX_HEAPS];
extern xMemBaseNotifyFunc sMemBaseNotifyFunc;

void xMemDebug_SoakLog(const char*);
void xMemInit();
void xMemExit();
void xMemInitHeap(xMemHeap* heap, xMemAddr base, U32 size, U32 flags);
void* xMemGrowAlloc(U32 heapID, U32 size);
void* xMemAlloc(U32 heapID, U32 size, S32 align);
void* xMemPushTemp(U32 size);
void xMemPopTemp(void* addr);
S32 xMemPushBase(U32 heapID);
S32 xMemPushBase();
S32 xMemPopBase(U32 heapID, S32 depth);
S32 xMemPopBase(S32 depth);
S32 xMemGetBase(U32 heapID);
void xMemRegisterBaseNotifyFunc(xMemBaseNotifyFunc func);
S32 xMemGetBase();
void xMemPoolSetup(xMemPool* pool, void* buffer, U32 nextOffset, U32 flags,
                   xMemPoolInitCallback initCB, U32 size, U32 count, U32 numRealloc);
void* xMemPoolAlloc(xMemPool* pool);
void xMemPoolFree(xMemPool* pool, void* data);

#define xMALLOC(size) xMemAlloc(gActiveHeap, (size), 0)
#define xMALLOCALIGN(size, align) xMemAlloc(gActiveHeap, (size), (align))
#define xGROWALLOC(size) xMemGrowAlloc(gActiveHeap, (size))
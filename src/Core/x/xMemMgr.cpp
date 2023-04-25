#include "xMemMgr.h"

#include "xMath.h"

#include <rwcore.h>

#include <stdlib.h>
#include <string.h>

#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

typedef struct xMemBlkInfo_tag
{
    xMemBlock* header;
    U32 pre;
    U32 block;
    U32 post;
    U32 curr;
    U32 waste;
    U32 total;
} xMemBlkInfo;

xMemInfo gMemInfo;
U32 gActiveHeap;
xMemHeap gxHeap[XMEM_MAX_HEAPS];
xMemBaseNotifyFunc sMemBaseNotifyFunc;

void xMemDebug_SoakLog(const char*)
{
}

void xMemInit()
{
    iMemInit();

    xMemInitHeap(&gxHeap[XMEM_HEAP_0],
                 gMemInfo.DRAM.addr,
                 gMemInfo.DRAM.size,
                 XMEMHEAP_0x2 | XMEMHEAP_0x4 | XMEMHEAP_0x20 |
                 XMEMHEAP_0x3E00(4) | XMEMHEAP_0x8000(1));
    
    xMemInitHeap(&gxHeap[XMEM_HEAP_1],
                 gMemInfo.DRAM.addr + gMemInfo.DRAM.size,
                 gMemInfo.DRAM.size,
                 XMEMHEAP_0x1 | XMEMHEAP_0x4 | XMEMHEAP_0x20 | XMEMHEAP_0x100 |
                 XMEMHEAP_0x3E00(4) | XMEMHEAP_0x8000(1));
    
    xMemInitHeap(&gxHeap[XMEM_HEAP_2],
                 gMemInfo.DRAM.addr + gMemInfo.DRAM.size,
                 gMemInfo.DRAM.size,
                 XMEMHEAP_0x2 | XMEMHEAP_0x8 | XMEMHEAP_0x20 | XMEMHEAP_0x100 |
                 XMEMHEAP_0x3E00(4) | XMEMHEAP_0x8000(1));

    gxHeap[XMEM_HEAP_0].opp_heap[0] = XMEM_HEAP_1;
    gxHeap[XMEM_HEAP_0].opp_heap[1] = XMEM_HEAP_2;
    gxHeap[XMEM_HEAP_1].opp_heap[0] = XMEM_HEAP_0;
    gxHeap[XMEM_HEAP_1].opp_heap[1] = XMEM_HEAP_2;
    gxHeap[XMEM_HEAP_2].opp_heap[0] = XMEM_HEAP_0;
    gxHeap[XMEM_HEAP_2].opp_heap[1] = XMEM_HEAP_1;

    gActiveHeap = XMEM_HEAP_0;
}

void xMemExit()
{
    iMemExit();
}

void xMemInitHeap(xMemHeap* heap, xMemAddr base, U32 size, U32 flags)
{
    xMemAddr old_base = base;
    S32 align = 1 << XMEMHEAP_GET_0x3E00(flags);

    if (flags & XMEMHEAP_0x100) {
        base = old_base & -align;
        size -= old_base - base;
    } else {
        base = -align & (old_base + align - 1);
        size -= base - old_base;
    }

    heap->state_idx = 0;
    heap->hard_base = base;
    heap->size = size;
    heap->flags = flags;
    heap->lastblk = NULL;

    xHeapState* sp = &heap->state[heap->state_idx];
    sp->curr = base;
    sp->blk_ct = 0;

    if (flags & XMEMHEAP_0x10000) {
        heap->max_blks = XMEMHEAP_GET_0x4000(flags);
        heap->blk = (xMemBlock*)malloc(heap->max_blks * sizeof(xMemBlock));
        memset(heap->blk, 0, heap->max_blks * sizeof(xMemBlock));
    } else {
        heap->max_blks = -1;
        heap->blk = NULL;
    }

    heap->opp_heap[0] = -1;
    heap->opp_heap[1] = -1;
}

static U32 xMemGetBlockInfo(xMemHeap* heap, U32 size, S32 align, xMemBlkInfo* info) NONMATCH("https://decomp.me/scratch/euPPA")
{
    S32 header_size;
    S32 total;
    S32 hdr;
    S32 pre;
    S32 block;
    S32 post;
    S32 dir;
    xHeapState* sp = &heap->state[heap->state_idx];
    S32 r10;

    dir = (heap->flags & XMEMHEAP_0x100) ? -1 : 1;
    r10 = sizeof(xMemBlock);
    r10 &= -XMEMHEAP_GET_0x8000(heap->flags);
    header_size = r10;

    if (heap->flags & XMEMHEAP_0x100) {
        hdr = -header_size;

        S32 remainder = (align - 1) & (sp->curr - header_size - size);

        block = -(S32)(header_size + size + remainder);
        pre = block;
        post = block + size;
        total = -pre;
    } else {
        hdr = 0;

        S32 remainder = (align - 1) & (sp->curr + header_size);
        if (!remainder) remainder = align;

        block = align + header_size - remainder;
        pre = block;
        post = block + size;
        total = post;
    }

    total = ALIGN(total, 4);

    if (heap->flags & XMEMHEAP_0x10000) {
        info->header = &heap->blk[sp->blk_ct];
    } else {
        info->header = (xMemBlock*)(sp->curr + hdr);
    }

    info->pre = sp->curr + pre;
    info->block = sp->curr + block;
    info->post = sp->curr + post;
    info->curr = sp->curr + dir * total;
    info->waste = total - (header_size + size);
    info->total = total;

    return total;
}

void* xMemGrowAlloc(U32 heapID, U32 size)
{
    size = ALIGN(size, 4);

    U32 oldalignsize;
    U32 newalignsize;
    xMemHeap* heap = &gxHeap[heapID];
    xMemBlock* hdr = heap->lastblk;
    xHeapState* sp = &heap->state[heap->state_idx];

    oldalignsize = hdr->size;
    newalignsize = hdr->size + size;
    if (sp->used + (newalignsize - oldalignsize) > heap->size) {
        return NULL;
    }

    void* memptr;
    if (heap->flags & XMEMHEAP_0x100) {
        memptr = (void*)(hdr->addr - size);
        hdr->addr = (xMemAddr)memptr;
        sp->curr -= newalignsize - oldalignsize;
    } else {
        memptr = (void*)(hdr->addr + hdr->size);
        sp->curr += newalignsize - oldalignsize;
    }

    sp->used += newalignsize - oldalignsize;
    hdr->size += size;

    memset(memptr, 0, size);

    return memptr;
}

void* xMemAlloc(U32 heapID, U32 size, S32 align)
{
    xMemHeap* heap = &gxHeap[heapID];
    xMemBlock* hdr;
    xHeapState* sp = &heap->state[heap->state_idx];

    align = xmax(align, 1 << XMEMHEAP_GET_0x3E00(heap->flags));

    if (size == 0) {
        return (void*)0xDEADBEEF;
    }

    xMemBlkInfo info;
    U32 total = xMemGetBlockInfo(heap, size, align, &info);

    hdr = info.header;

    if (sp->used + total > heap->size) {
        return NULL;
    }

    sp->curr = info.curr;
    sp->blk_ct++;

    if (heap->flags & XMEMHEAP_0x10000) {
        return NULL;
    }

    hdr->addr = info.block;
    hdr->size = size;
    hdr->align = align;

    sp->used += total;
    sp->wasted += info.waste;

    memset((void*)hdr->addr, 0, size);

    heap->lastblk = hdr;

    return (void*)hdr->addr;
}

void* xMemPushTemp(U32 size)
{
#if (rwLIBRARYCURRENTVERSION >= 0x36000)
    return RwMalloc(size, rwMEMHINTDUR_NADURATION);
#else
    return RwMalloc(size);
#endif
}

void xMemPopTemp(void* addr)
{
    RwFree(addr);
}

S32 xMemPushBase(U32 heapID)
{
    xMemHeap* heap = &gxHeap[heapID];
    heap->state_idx++;

    xHeapState* sp = &heap->state[heap->state_idx];
    *sp = *(sp - 1);

    if (sMemBaseNotifyFunc) {
        sMemBaseNotifyFunc();
    }

    return heap->state_idx;
}

S32 xMemPushBase()
{
    return xMemPushBase(gActiveHeap);
}

S32 xMemPopBase(U32 heapID, S32 depth)
{
    xMemHeap* heap = &gxHeap[heapID];
    if (depth < 0) {
        depth += heap->state_idx;
    }
    heap->state_idx = depth;

    if (sMemBaseNotifyFunc) {
        sMemBaseNotifyFunc();
    }

    return heap->state_idx;
}

S32 xMemPopBase(S32 depth)
{
    return xMemPopBase(gActiveHeap, depth);
}

S32 xMemGetBase(U32 heapID)
{
    return gxHeap[heapID].state_idx;
}

void xMemRegisterBaseNotifyFunc(xMemBaseNotifyFunc func)
{
    sMemBaseNotifyFunc = func;
}

S32 xMemGetBase()
{
    return xMemGetBase(gActiveHeap);
}

static void xMemPoolAddElements(xMemPool* pool, void* buffer, U32 count)
{
    S32 i;
    void* curr = buffer;
    xMemPoolInitCallback initCB = pool->InitCB;
    U32 next = pool->NextOffset;
    U32 size = pool->Size;

    for (i = 0; i < (S32)(count - 1); i++) {
        *(void**)((char*)curr + next) = (void*)((char*)curr + size);
        curr = (void*)((char*)curr + size);
    }
    *(void**)((char*)curr + next) = pool->FreeList;

    if (initCB) {
        initCB(pool, curr);
    }

    pool->FreeList = buffer;
    pool->Total += count;
}

void xMemPoolSetup(xMemPool* pool, void* buffer, U32 nextOffset, U32 flags,
                   xMemPoolInitCallback initCB, U32 size, U32 count, U32 numRealloc)
{
    pool->FreeList = NULL;
    pool->NextOffset = nextOffset;
    pool->Flags = flags;
    pool->UsedList = NULL;
    pool->InitCB = initCB;
    pool->Buffer = buffer;
    pool->Size = size;
    pool->NumRealloc = numRealloc;
    pool->Total = 0;

    xMemPoolAddElements(pool, buffer, count);
}

void* xMemPoolAlloc(xMemPool* pool)
{
    void* retval = pool->FreeList;
    U32 next = pool->NextOffset;
    U32 flags = pool->Flags;

    if (!retval) {
        xMemPoolAddElements(pool, xMALLOC(pool->NumRealloc * pool->Size), pool->NumRealloc);
        retval = pool->FreeList;
    }

    pool->FreeList = (void*)((char*)retval + next);
    if (flags & XMEMPOOL_0x1) {
        pool->UsedList = (void*)((char*)retval + next);
    }

    return retval;
}

void xMemPoolFree(xMemPool* pool, void* data)
{
    if (!data) return;

    void* freeList = pool->FreeList;
    U32 next = pool->NextOffset;
    U32 flags = pool->Flags;

    void** prev = &pool->UsedList;
    void* curr = pool->UsedList;
    while (curr && curr != data) {
        prev = (void**)((char*)curr + next);
        curr = *(void**)((char*)curr + next);
    }
    if (curr) {
        *prev = *(void**)((char*)curr + next);
    }

    *(void**)((char*)data + next) = freeList;
    pool->FreeList = data;
}
#include "iMemMgr.h"

#include "xMemMgr.h"
#include "xDebug.h"

#include <stdlib.h>
#include <windows.h>

#define HEAP_SIZE 0x384000

static HANDLE TheHeap;

void iMemInit()
{
    ULONG_PTR StackLo;
    ULONG_PTR StackHi;
    GetCurrentThreadStackLimits(&StackLo, &StackHi);

    TheHeap = HeapCreate(0, HEAP_SIZE, 0);
    if (!TheHeap) {
        xASSERTFAILMSG("Failed to create main heap!");
        exit(-5);
    }

    xMemAddr HeapBase = (xMemAddr)HeapAlloc(TheHeap, 0, HEAP_SIZE);
    if (!HeapBase) {
        xASSERTFAILMSG("Failed to allocate main heap memory!");
        exit(-5);
    }

    gMemInfo.system.addr = 0;
    gMemInfo.system.size = 16;
    gMemInfo.system.flags = XMEMAREA_0x20;
    gMemInfo.stack.addr = (xMemAddr)StackLo;
    gMemInfo.stack.size = (U32)(StackHi - StackLo);
    gMemInfo.stack.flags = XMEMAREA_0x20 | XMEMAREA_0x800;
    gMemInfo.DRAM.addr = HeapBase;
    gMemInfo.DRAM.size = HEAP_SIZE;
    gMemInfo.DRAM.flags = XMEMAREA_0x20 | XMEMAREA_0x800;
    gMemInfo.SRAM.addr = 0;
    gMemInfo.SRAM.size = 32;
    gMemInfo.SRAM.flags = XMEMAREA_0x20 | XMEMAREA_0x40 | XMEMAREA_0x200 | XMEMAREA_0x400;
}

void iMemExit()
{
    HeapFree(TheHeap, 0, (void*)gMemInfo.DRAM.addr);
    gMemInfo.DRAM.addr = 0;

    HeapDestroy(TheHeap);
}
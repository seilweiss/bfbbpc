#include "xRMemData.h"

#include "xMemMgr.h"

#include <string.h>

void* RyzMemData::operator new(size_t amt, S32 who, RyzMemGrow* growCtxt)
{
    void* mem = NULL;
    S32 dogrow = 1;

    if (!growCtxt) {
        dogrow = 0;
    } else if (!growCtxt->IsEnabled()) {
        dogrow = 0;
    }

    if (dogrow) {
        mem = xGROWALLOC(amt);
    } else {
        mem = xMALLOC(amt);
    }

    memset(mem, 0, 4);

    return mem;
}

// not present in original game - added to fix warning C4291
void RyzMemData::operator delete(void*, S32 who, RyzMemGrow* growCtxt)
{
}

void RyzMemData::operator delete(void*)
{
}

RyzMemGrow* RyzMemGrow::Init(xBase* growuser)
{
    if (ptr) return this;

    amt_last = 0;
    ptr_last = NULL;
    user_last = NULL;
    amt = 32;
    ptr = (char*)xMALLOC(amt);
    user = growuser;
    flg_grow = 0x1;

    return this;
}

RyzMemGrow* RyzMemGrow::Resume(xBase*)
{
    amt = amt_last;
    ptr = ptr_last;
    user = user_last;
    flg_grow = 0x3;

    return this;
}

void RyzMemGrow::Done()
{
    amt_last = amt;
    ptr_last = ptr;
    user_last = user;
    amt = 0;
    ptr = NULL;
    user = NULL;
    flg_grow = 0;
}
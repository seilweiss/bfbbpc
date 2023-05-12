#pragma once

#include "types.h"

struct xBase;
struct RyzMemGrow;

struct RyzMemData
{
    void* operator new(size_t amt, S32 who, RyzMemGrow* growCtxt);
    void operator delete(void*);
};

struct RyzMemGrow
{
protected:
    S32 flg_grow;
    S32 amt;
    char* ptr;
    xBase* user;
    S32 amt_last;
    char* ptr_last;
    xBase* user_last;

public:
    RyzMemGrow* Init(xBase* growuser);
    RyzMemGrow* Resume(xBase*);
    void Done();

    S32 IsEnabled()
    {
        return flg_grow & 0x1;
    }
};
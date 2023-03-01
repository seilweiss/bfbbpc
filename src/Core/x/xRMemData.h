#pragma once

#include "types.h"

struct xBase;

struct RyzMemData
{
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
};
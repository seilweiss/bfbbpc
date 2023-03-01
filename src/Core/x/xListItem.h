#pragma once

#include "types.h"

template <class T>
struct xListItem
{
protected:
    S32 flg_travFilter;
    T* next;
    T* prev;
};
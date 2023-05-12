#pragma once

#include "types.h"

template <class T>
struct xListItem
{
protected:
    S32 flg_travFilter;
    T* next;
    T* prev;

public:
    xListItem()
    {
        flg_travFilter = 0;
        next = prev = NULL;
    }

    void Insert(T* list)
    {
        T* node = (T*)this;
        prev = list;
        next = list->next;
        if (list->next) list->next->prev = node;
        list->next = node;
    }

    void Remove()
    {
        if (next) next->prev = prev;
        if (prev) prev->next = next;
        next = NULL;
        prev = NULL;
    }

    T* RemHead(T** listhead)
    {
        if (!*listhead) return NULL;

        T* oldhead = (*listhead)->Head();
        if (!oldhead) {
            *listhead = NULL;
        } else {
            *listhead = oldhead->Next();
            oldhead->Remove();
        }
        return oldhead;
    }

    T* Head()
    {
        T* node = (T*)this;
        if (!node) return node;
        while (node->prev) node = node->prev;
        return node;
    }

    T* Next() { return next; }
    T* Prev() { return prev; }
};
#pragma once

#include "xMath3.h"
#include "xQuickCull.h"

struct xEnt;

struct xGridBound
{
    void* data;
    U16 gx;
    U16 gz;
    U8 ingrid;
    U8 oversize;
    U8 deleted;
    U8 gpad;
    xGridBound** head;
    xGridBound* next;
};

struct xGrid
{
    U8 ingrid_id;
    U8 pad[3];
    U16 nx;
    U16 nz;
    F32 minx;
    F32 minz;
    F32 maxx;
    F32 maxz;
    F32 csizex;
    F32 csizez;
    F32 inv_csizex;
    F32 inv_csizez;
    F32 maxr;
    xGridBound** cells;
    xGridBound* other;
};

struct xGridIterator
{
    xGridBound** listhead;
    xGridBound* curcell;
    U32 delfound;
};

typedef S32(*xGridCheckPositionCallback)(xEnt*, void*);

extern S32 gGridIterActive;

void xGridBoundInit(xGridBound* gridb, void* data);
void xGridInit(xGrid* grid, const xBox* bounds, U16 nx, U16 nz, U8 ingrid_id);
void xGridKill(xGrid* grid);
void xGridEmpty(xGrid* grid);
S32 xGridAddToCell(xGridBound** cell, xGridBound* gridb);
S32 xGridAdd(xGrid* grid, xEnt* ent);
S32 xGridRemove(xGridBound* gridb);
void xGridUpdate(xGrid* grid, xEnt* ent);
void xGridGetCell(xGrid* grid, F32 posx, F32 posy, F32 posz, S32& grx, S32& grz);
xGridBound* xGridIterFirstCell(xGrid* grid, F32 posx, F32 posy, F32 posz, S32& grx, S32& grz, xGridIterator& it);
S32 xGridEntIsTooBig(xGrid* grid, const xEnt* ent);
void xGridCheckPosition(xGrid* grid, xVec3* pos, xQCData* qcd, xGridCheckPositionCallback hitCB, void* cbdata);

inline xGridBound* xGridIterFirstCell(xGridBound** head, xGridIterator& it)
{
    xGridBound* curcell = *head;
    if (!curcell) return NULL;
    it.delfound = 0;
    it.listhead = head;
    it.curcell = curcell;
    gGridIterActive++;
    return curcell;
}

inline xGridBound* xGridIterFirstCell(xGrid* grid, S32 grx, S32 grz, xGridIterator& it)
{
    if (grx < 0 || grx >= grid->nx) return NULL;
    if (grz < 0 || grz >= grid->nz) return NULL;
    return xGridIterFirstCell(grid->cells + grz * grid->nx + grx, it);
}

inline void xGridIterClose(xGridIterator& it) NONMATCH("https://decomp.me/scratch/uqNnr")
{
    if (!it.listhead) return;
    gGridIterActive--;
    if (it.delfound && gGridIterActive == 0) {
        xGridBound* cur = *it.listhead;
        xGridBound** prev = it.listhead;
        while (cur) {
            if (cur->deleted) {
                *prev = cur->next;
                cur->next = NULL;
                cur->head = NULL;
                cur->ingrid = 0;
                cur->deleted = 0;
                cur->gx = -1;
                cur->gz = -1;
                cur = *prev;
            } else {
                prev = &cur->next;
                cur = cur->next;
            }
        }
    }
    it.listhead = NULL;
    it.curcell = NULL;
    it.delfound = 0;
}

inline xGridBound* xGridIterNextCell(xGridIterator& it)
{
    if (it.curcell) {
        it.curcell = it.curcell->next;
    }
    while (it.curcell) {
        if (!it.curcell->deleted) {
            return it.curcell;
        }
        it.delfound = 1;
        it.curcell = it.curcell->next;
    }
    xGridIterClose(it);
    return NULL;
}

struct grid_index
{
    U16 x;
    U16 z;
};

inline grid_index get_grid_index(const xGrid& grid, F32 x, F32 z) NONMATCH("https://decomp.me/scratch/pMa66")
{
    grid_index index = { range_limit<U16>((U16)((x - grid.minx) * grid.inv_csizex), 0, grid.nx - 1),
                         range_limit<U16>((U16)((z - grid.minz) * grid.inv_csizez), 0, grid.nz - 1) };
    return index;
}

template <class T>
inline void xGridCheckBound(xGrid& grid, const xBound& bound, const xQCData& qcd, T cb)
{
    xGridIterator it;
    
    xBox box;
    xBoundGetBox(box, bound);

    F32 ex = 0.25f * grid.csizex;
    F32 ez = 0.25f * grid.csizez;
    box.lower.x -= ex;
    box.lower.z -= ez;
    box.upper.x += ex;
    box.upper.z += ez;

    grid_index var_4C, var_50;
    var_50 = get_grid_index(grid, box.lower.x, box.lower.z);
    var_4C = get_grid_index(grid, box.upper.x, box.upper.z);

    xGridBound* cell = xGridIterFirstCell(&grid.other, it);
    while (cell) {
        if (xQuickCullIsects(&qcd, &((xBound*)(cell+1))->qcd)) {
            if (!cb(*(xEnt*)cell->data, *cell)) {
                xGridIterClose(it);
                return;
            }
        }
        cell = xGridIterNextCell(it);
    }

    for (U16 gx = var_50.x; gx <= var_4C.x; gx++) {
        for (U16 gz = var_50.z; gz <= var_4C.z; gz++) {
            xGridBound* cell = xGridIterFirstCell(&grid, gx, gz, it);
            while (cell) {
                if (xQuickCullIsects(&qcd, &((xBound*)(cell+1))->qcd)) {
                    if (!cb(*(xEnt*)cell->data, *cell)) {
                        xGridIterClose(it);
                        return;
                    }
                }
                cell = xGridIterNextCell(it);
            }
        }
    }
}
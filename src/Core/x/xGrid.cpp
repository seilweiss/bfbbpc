#include "xGrid.h"

#include "xEnt.h"
#include "xMemMgr.h"

S32 gGridIterActive;

static S32 xGridAdd(xGrid* grid, xGridBound* gridb, S32 x, S32 z);
static xGridBound** xGridGetCell(xGrid* grid, const xEnt* ent, S32& grx, S32& grz);

void xGridBoundInit(xGridBound* gridb, void* data)
{
    gridb->data = data;
    gridb->gx = -1;
    gridb->gz = -1;
    gridb->ingrid = 0;
    gridb->oversize = 0;
    gridb->head = NULL;
    gridb->next = NULL;
    gridb->gpad = 0xEA;
}

void xGridInit(xGrid* grid, const xBox* bounds, U16 nx, U16 nz, U8 ingrid_id) NONMATCH("https://decomp.me/scratch/PzqLZ")
{
    grid->ingrid_id = ingrid_id;
    grid->nx = nx;
    grid->nz = nz;
    grid->minx = bounds->lower.x;
    grid->minz = bounds->lower.z;
    grid->maxx = bounds->upper.x;
    grid->maxz = bounds->upper.z;
    
    F32 gsizex = grid->maxx - grid->minx;
    F32 gsizez = grid->maxz - grid->minz;

    grid->csizex = gsizex / nx;
    grid->csizez = gsizez / nz;

    if (xabs(gsizex) <= 0.001f) {
        grid->inv_csizex = 1.0f;
    } else {
        grid->inv_csizex = nx / gsizex;
    }

    if (xabs(gsizez) <= 0.001f) {
        grid->inv_csizez = 1.0f;
    } else {
        grid->inv_csizez = nz / gsizez;
    }

    grid->maxr = 0.25f * xmin(grid->csizex, grid->csizez);
    grid->cells = (xGridBound**)xMALLOC(nx * nz * sizeof(xGridBound*));
    memset(grid->cells, 0, nx * nz * sizeof(xGridBound*));
}

void xGridKill(xGrid* grid)
{
    xGridEmpty(grid);
    grid->cells = NULL;
}

void xGridEmpty(xGrid* grid)
{
    xGridBound** head;
    xGridBound* curr;
    
    for (S32 x = 0; x < grid->nx; x++) {
        for (S32 z = 0; z < grid->nz; z++) {
            head = grid->cells + z * grid->nx + x;
            curr = *head;
            while (curr) {
                xGridBound* currnext = curr->next;
                xGridBoundInit(curr, curr->data);
                curr = currnext;
            }
            *head = NULL;
        }
    }

    head = &grid->other;
    curr = grid->other;
    while (curr) {
        xGridBound* currnext = curr->next;
        xGridBoundInit(curr, curr->data);
        curr = currnext;
    }

    *head = NULL;
}

S32 xGridAddToCell(xGridBound** cell, xGridBound* gridb)
{
    if (gridb->head) {
        if (!gGridIterActive) {
            if (!xGridRemove(gridb)) return 0;
        } else return 0;
    }

    gridb->head = cell;
    gridb->next = *cell;
    *cell = gridb;
    return 1;
}

static S32 xGridAdd(xGrid* grid, xGridBound* gridb, S32 x, S32 z)
{
    xGridBound** cell = grid->cells + z * grid->nx + x;
    return xGridAddToCell(cell, gridb);
}

S32 xGridAdd(xGrid* grid, xEnt* ent) NONMATCH("https://decomp.me/scratch/5R7FZ")
{
    xBound* bound;
    xVec3* center;
    F32 maxr;

    bound = &ent->bound;
    maxr = grid->maxr;

    if (bound->type == k_XBOUNDTYPE_SPHERE) {
        xSphere* sph = &bound->sph;
        center = &sph->center;
        if (bound->sph.r >= maxr) {
            S32 r = xGridAddToCell(&grid->other, &ent->gridb);
            if (r) {
                ent->gridb.ingrid = grid->ingrid_id;
            }
            return r;
        }
    } else if (bound->type == k_XBOUNDTYPE_OBB) {
        xBBox* bbox = &bound->box;
        center = &bbox->center;
        F32 rx = bbox->box.upper.x - bbox->box.lower.x;
        F32 ry = bbox->box.upper.y - bbox->box.lower.y;
        F32 rz = bbox->box.upper.z - bbox->box.lower.z;
        F32 len2 = xsqr(rx) * (xsqr(bound->mat->right.x) + xsqr(bound->mat->right.y) + xsqr(bound->mat->right.z)) +
                   xsqr(ry) * (xsqr(bound->mat->up.x) + xsqr(bound->mat->up.y) + xsqr(bound->mat->up.z)) +
                   xsqr(rz) * (xsqr(bound->mat->at.x) + xsqr(bound->mat->at.y) + xsqr(bound->mat->at.z));
        if (len2 >= 4.0f * maxr * maxr) {
            S32 r = xGridAddToCell(&grid->other, &ent->gridb);
            if (r) {
                ent->gridb.ingrid = grid->ingrid_id;
            }
            return r;
        }
    } else if (bound->type == k_XBOUNDTYPE_BOX) {
        xBBox* bbox = &bound->box;
        center = &bbox->center;
        F32 rx = bound->box.box.upper.x - bound->box.box.lower.x;
        F32 rz = bound->box.box.upper.z - bound->box.box.lower.z;
        F32 len2 = xsqr(rx) + xsqr(rz);
        if (len2 >= 4.0f * maxr * maxr) {
            S32 r = xGridAddToCell(&grid->other, &ent->gridb);
            if (r) {
                ent->gridb.ingrid = grid->ingrid_id;
            }
            return r;
        }
    } else {
        return 0;
    }
    
    F32 cgridx = center->x - grid->minx;
    cgridx *= grid->inv_csizex;

    F32 cgridz = center->z - grid->minz;
    cgridz *= grid->inv_csizez;

    S32 x = (S32)xmin(grid->nx - 1, xmax(0.0f, cgridx));
    S32 z = (S32)xmin(grid->nz - 1, xmax(0.0f, cgridz));

    if (xGridAdd(grid, &ent->gridb, x, z)) {
        ent->gridb.gx = x;
        ent->gridb.gz = z;
        ent->gridb.ingrid = grid->ingrid_id;
        return 1;
    }

    return 0;
}

S32 xGridRemove(xGridBound* gridb)
{
    xGridBound* cur;
    xGridBound** prev;
    
    if (gridb->head) {
        if (gGridIterActive) {
            gridb->deleted = 1;
            return 0;
        }
        prev = gridb->head;
        cur = *prev;
        while (cur && cur != gridb) {
            prev = &cur->next;
            cur = cur->next;
        }
        *prev = cur->next;
        cur->next = NULL;
        cur->head = NULL;
        cur->ingrid = 0;
        cur->deleted = 0;
        cur->gx = -1;
        cur->gz = -1;
    }
    
    return 1;
}

void xGridUpdate(xGrid* grid, xEnt* ent)
{
    S32 dx, dz;
    xGridGetCell(grid, ent, dx, dz);
    if (dx != ent->gridb.gx || dz != ent->gridb.gz) {
        if (xGridRemove(&ent->gridb)) {
            xGridAdd(grid, &ent->gridb, dx, dz);
        }
    }
}

static xGridBound** xGridGetCell(xGrid* grid, const xEnt* ent, S32& grx, S32& grz)
{
    const xBound* bound = &ent->bound;
    const xVec3* center;
    if (bound->type == k_XBOUNDTYPE_SPHERE) {
        center = &bound->sph.center;
    } else if (bound->type == k_XBOUNDTYPE_OBB) {
        center = &bound->box.center;
    } else if (bound->type == k_XBOUNDTYPE_BOX) {
        center = &bound->box.center;
    } else {
        return NULL;
    }
    xGridGetCell(grid, center->x, center->y, center->z, grx, grz);
    return grid->cells + grz * grid->nx + grx;
}

void xGridGetCell(xGrid* grid, F32 posx, F32 posy, F32 posz, S32& grx, S32& grz) NONMATCH("https://decomp.me/scratch/l160F")
{
    F32 pgridx = posx - grid->minx;
    pgridx *= grid->inv_csizex;
    
    F32 pgridz = posz - grid->minz;
    pgridz *= grid->inv_csizez;

    grx = (S32)xmin(grid->nx - 1, xmax(0, pgridx));
    grz = (S32)xmin(grid->nz - 1, xmax(0, pgridz));
}

xGridBound* xGridIterFirstCell(xGrid* grid, F32 posx, F32 posy, F32 posz, S32& grx, S32& grz, xGridIterator& it)
{
    xGridGetCell(grid, posx, posy, posz, grx, grz);
    return xGridIterFirstCell(grid, grx, grz, it);
}

S32 xGridEntIsTooBig(xGrid* grid, const xEnt* ent)
{
    const xBound* bound = &ent->bound;
    F32 maxr = grid->maxr;

    if (bound->type == k_XBOUNDTYPE_SPHERE) {
        const xSphere* sph = &bound->sph;
        if (sph->r >= maxr) {
            return 1;
        }
    } else if (bound->type == k_XBOUNDTYPE_OBB) {
        const xBBox* bbox = &bound->box;
        F32 rx = bbox->box.upper.x - bbox->box.lower.x;
        F32 ry = bbox->box.upper.y - bbox->box.lower.y;
        F32 rz = bbox->box.upper.z - bbox->box.lower.z;
        F32 len2 = xsqr(rx) * (xsqr(bound->mat->right.x) + xsqr(bound->mat->right.y) + xsqr(bound->mat->right.z)) +
                   xsqr(ry) * (xsqr(bound->mat->up.x) + xsqr(bound->mat->up.y) + xsqr(bound->mat->up.z)) +
                   xsqr(rz) * (xsqr(bound->mat->at.x) + xsqr(bound->mat->at.y) + xsqr(bound->mat->at.z));
        if (len2 >= 4.0f * maxr * maxr) {
            return 1;
        }
    } else if (bound->type == k_XBOUNDTYPE_BOX) {
        const xBBox* bbox = &bound->box;
        F32 rx = bbox->box.upper.x - bbox->box.lower.x;
        F32 rz = bbox->box.upper.z - bbox->box.lower.z;
        F32 len2 = xsqr(rx) + xsqr(rz);
        if (len2 >= 4.0f * maxr * maxr) {
            return 1;
        }
    }

    return 0;
}

void xGridCheckPosition(xGrid* grid, xVec3* pos, xQCData* qcd, xGridCheckPositionCallback hitCB, void* cbdata)
{
    xGridIterator it;
    S32 px, pz;
    xGridBound* cell;
    
    cell = xGridIterFirstCell(grid, pos->x, pos->y, pos->z, px, pz, it);
    while (cell) {
        xBound* cellbound = (xBound*)(cell+1);
        if (xQuickCullIsects(qcd, &cellbound->qcd) && !hitCB((xEnt*)cell->data, cbdata)) {
            xGridIterClose(it);
            return;
        }
        cell = xGridIterNextCell(it);
    }

    xBox clbox;
    clbox.lower.x = grid->csizex * px;
    clbox.lower.z = grid->csizez * pz;
    clbox.lower.x += grid->minx;
    clbox.lower.z += grid->minz;
    
    F32 clcenterx = 0.5f * grid->csizex;
    clcenterx += clbox.lower.x;
    
    F32 clcenterz = 0.5f * grid->csizez;
    clcenterz += clbox.lower.z;

    static S32 offs[4][3][2] = {
        -1, 0, -1, -1,
        0, -1, 0, -1,
        1, -1, 1, 0,
        1, 0, 1, 1,
        0, 1, 0, 1,
        -1, 1, -1, 0
    };
    
    static S32 k;
    
    if (pos->x < clcenterx) {
        if (pos->z < clcenterz) {
            k = 0;
        } else {
            k = 1;
        }
    } else {
        if (pos->z < clcenterz) {
            k = 3;
        } else {
            k = 2;
        }
    }
    
    for (S32 i = 0; i < 3; i++) {
        S32 _x = px + offs[k][i][1];
        if (_x >= 0 && _x < grid->nx) {
            S32 _z = pz + offs[k][i][0];
            if (_z >= 0 && _z < grid->nz) {
                cell = xGridIterFirstCell(grid, _x, _z, it);
                while (cell) {
                    xBound* cellbound = (xBound*)(cell+1);
                    if (xQuickCullIsects(qcd, &cellbound->qcd) && !hitCB((xEnt*)cell->data, cbdata)) {
                        xGridIterClose(it);
                        return;
                    }
                    cell = xGridIterNextCell(it);
                }
            }
        }
    }

    cell = xGridIterFirstCell(&grid->other, it);
    while (cell) {
        xBound* cellbound = (xBound*)(cell+1);
        if (xQuickCullIsects(qcd, &cellbound->qcd) && !hitCB((xEnt*)cell->data, cbdata)) {
            xGridIterClose(it);
            return;
        }
        cell = xGridIterNextCell(it);
    }
}

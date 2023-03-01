#include "zGrid.h"

#include "xString.h"
#include "zBase.h"
#include "zScene.h"

xGrid colls_grid;
xGrid colls_oso_grid;
xGrid npcs_grid;
static S32 zGridInitted;

static void hack_flag_shadows(zScene* s) NONMATCH("https://decomp.me/scratch/HF4nC")
{
    static U32 special_models[] =
    {
        xStrHash("bb_arrow"),
        xStrHash("metal_sheets"),
        xStrHash("clam_poop"),
        xStrHash("beach_chair_yellow"),
        xStrHash("debris_pile_rb_smll"),
        xStrHash("debris_pile_rb"),
        xStrHash("floor_panel"),
        xStrHash("gy_woodpieceA"),
        xStrHash("quarter_note_on"),
        xStrHash("eighth_note_on"),
        xStrHash("db03_path_a"),
        xStrHash("db03_path_b"),
        xStrHash("db03_path_c"),
        xStrHash("db03_path_d"),
        xStrHash("db03_path_e"),
        xStrHash("db03_path_f"),
        xStrHash("db03_path_g"),
        xStrHash("db03_path_h"),
        xStrHash("db03_path_i"),
        xStrHash("db03_path_j"),
        xStrHash("db03_path_k"),
        xStrHash("db03_path_l"),
        xStrHash("db03_path_m"),
        xStrHash("db03_path_o"),
        xStrHash("db03_path_p"),
    };
    
    const U32* end_special_models = special_models + ARRAY_LENGTH(special_models);
    
    for (zEnt** it = s->ents, **end = it + s->num_ents; it != end; it++) {
        xEnt* ent = *it;
        if (ent && (ent->baseFlags & k_XBASE_IS_ENTITY) && ent->asset) {
            for (U32* id = special_models; id != end_special_models; id++) {
                if (ent->asset->modelInfoID == *id) {
                    ent->chkby |= 0x80;
                    ent->baseFlags |= k_XBASE_RECEIVES_SHADOWS;
                    ent->asset->baseFlags |= k_XBASE_RECEIVES_SHADOWS;
                    break;
                }
            }
        }
    }
}

void zGridReset(zScene* s)
{
    hack_flag_shadows(s);
    
    for (U32 i = 0; i < s->num_base; i++) {
        xBase* base = s->base[i];
        if (base && (base->baseFlags & k_XBASE_IS_ENTITY) &&
            base->baseType != eBaseTypeTrigger &&
            base->baseType != eBaseTypeUI &&
            base->baseType != eBaseTypeUIFont &&
            base->baseType != eBaseTypePlayer) {
            xEnt* ent = (xEnt*)base;
            if (ent->bupdate) {
                ent->bupdate(ent, (xVec3*)&ent->model->Mat->pos);
            } else {
                xEntDefaultBoundUpdate(ent, (xVec3*)&ent->model->Mat->pos);
            }
            zGridUpdateEnt(ent);
        }
    }
}

void zGridInit(zScene* s) NONMATCH("https://decomp.me/scratch/bL9tV")
{
    gGridIterActive = 0;

    const xBox* ebox = xEntGetAllEntsBox();
    F32 grsizex, grsizez, min_csize;

    grsizex = xmax(0.001f, ebox->upper.x - ebox->lower.x);
    grsizez = xmax(0.001f, ebox->upper.z - ebox->lower.z);
    min_csize = 10.0f;

    xGridInit(&colls_grid, ebox,
              (U16)xclamp(32.0f, 1.0f, std::floorf(grsizex / min_csize)),
              (U16)xclamp(32.0f, 1.0f, std::floorf(grsizez / min_csize)),
              1);

    xBox osobox = *ebox;
    osobox.upper.x += 3.4567f;
    osobox.upper.z += 3.4567f;
    osobox.lower.x -= 1.0f;
    osobox.lower.z -= 1.0f;

    grsizex = xmax(0.001f, osobox.upper.x - osobox.lower.x);
    grsizez = xmax(0.001f, osobox.upper.z - osobox.lower.z);
    min_csize *= 6.0f;

    xGridInit(&colls_oso_grid, &osobox,
              (U16)xclamp(8.0f, 1.0f, std::floorf(grsizex / min_csize)),
              (U16)xclamp(8.0f, 1.0f, std::floorf(grsizez / min_csize)),
              2);

    min_csize = 20.0f;

    xGridInit(&npcs_grid, ebox,
              (U16)xclamp(16.0f, 1.0f, std::floorf(grsizex / min_csize)),
              (U16)xclamp(16.0f, 1.0f, std::floorf(grsizez / min_csize)),
              3);

    zGridInitted = 1;
    zGridReset(s);
}

void zGridExit(zScene* s)
{
    xGridKill(&colls_grid);
    xGridKill(&colls_oso_grid);
    xGridKill(&npcs_grid);
    gGridIterActive = 0;
    zGridInitted = 0;
}

void zGridUpdateEnt(xEnt* ent)
{
    if (!zGridInitted) return;
    
    S32 oversize = 0;
    xGrid* grid = NULL;

    switch (ent->gridb.ingrid) {
    case 1:
        grid = &colls_grid;
        break;
    case 2:
        grid = &colls_oso_grid;
        oversize = (ent->gridb.oversize == 2);
        break;
    case 3:
        grid = &npcs_grid;
        oversize = (ent->gridb.oversize == 1);
        break;
    }

    if ((ent->chkby & (k_XENT_COLLTYPE_PC | k_XENT_COLLTYPE_NPC | 0x80)) ||
        ent->baseType == eBaseTypePickup) {
        if (grid) {
            if (!oversize) {
                xGridUpdate(grid, ent);
            }
        } else if (ent->collType == k_XENT_COLLTYPE_NPC) {
            if (xGridEntIsTooBig(&npcs_grid, ent)) {
                ent->gridb.oversize = 1;
            } else {
                ent->gridb.oversize = 0;
            }
            xGridAdd(&npcs_grid, ent);
        } else if (xGridEntIsTooBig(&colls_grid, ent)) {
            if (xGridEntIsTooBig(&colls_oso_grid, ent)) {
                ent->gridb.oversize = 2;
            } else {
                ent->gridb.oversize = 1;
            }
            xGridAdd(&colls_oso_grid, ent);
        } else {
            xGridAdd(&colls_grid, ent);
            ent->gridb.oversize = 0;
        }
    } else if (grid) {
        xGridRemove(&ent->gridb);
    }
}
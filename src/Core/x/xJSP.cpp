#include "xJSP.h"

#include "zGlobals.h"

#include <string.h>

struct __rwMark
{
    RwUInt32 type;
    RwUInt32 length;
    RwUInt32 libraryID;
};

static RwV3d* sCurrVert;
static U32 sAtomicStartCount;
static RwV3d** sAtomicStartVert;

static RpAtomic* CountAtomicCB(RpAtomic* atomic, void* data)
{
    sAtomicStartCount++;
    *(U32*)data += atomic->geometry->mesh->totalIndicesInMesh;
}

static RpMesh* AddMeshCB(RpMesh* mesh, RpMeshHeader*, void* pData)
{
    RwV3d** stripVert = (RwV3d**)pData;
    for (U32 i = 0; i < mesh->numIndices; i++) {
        **stripVert = sCurrVert[mesh->indices[i]];
        (*stripVert)++;
    }
    return mesh;
}

static RpAtomic* AddAtomicCB(RpAtomic* atomic, void* data) NONMATCH("https://decomp.me/scratch/GuL2B")
{
    sAtomicStartVert[--sAtomicStartCount] = *(RwV3d**)data;
    sCurrVert = atomic->geometry->morphTarget->verts;
    _rpMeshHeaderForAllMeshes(atomic->geometry->mesh, AddMeshCB, data);
    return atomic;
}

static RpAtomic* AddAtomicPrecalcedVertCB(RpAtomic* atomic, void* data) NONMATCH("https://decomp.me/scratch/AHv5H")
{
    sAtomicStartVert[--sAtomicStartCount] = *(RwV3d**)data;
    *(RwV3d**)data += atomic->geometry->mesh->totalIndicesInMesh;
    return atomic;
}

static RpAtomic* ListAtomicCB(RpAtomic* atomic, void* data)
{
    RpAtomic*** aList = (RpAtomic***)data;
    **aList = atomic;
    (*aList)++;
    return atomic;
}

void xJSP_MultiStreamRead(void* data, U32 size, xJSPHeader** jsp) NONMATCH("https://decomp.me/scratch/dkzVV")
{
    U32 i;
    RpClump* clump;
    xClumpCollBSPTree* colltree;
    
    if (!*jsp) {
        *jsp = (xJSPHeaderEx*)RwMalloc(sizeof(xJSPHeaderEx));
        memset(*jsp, 0, sizeof(xJSPHeaderEx));
    }

    xJSPHeaderEx* hdr = (xJSPHeaderEx*)*jsp;
    
    __rwMark mark;
    __rwMark* mp = (__rwMark*)data;
    memmove(&mark, mp, sizeof(__rwMark));
    RwMemNative32(&mark, sizeof(__rwMark));

    if (mark.type == 0xBEEF01) {
        data = mp + 1;
        colltree = xClumpColl_StaticBufferInit(data, mark.length);

        size -= mark.length + sizeof(__rwMark);
        data = (U8*)data + mark.length;

        mp = (__rwMark*)data;
        memmove(&mark, mp, sizeof(__rwMark));
        RwMemNative32(&mark, sizeof(__rwMark));

        data = mp + 1;

        xJSPHeader* tmphdr = (xJSPHeader*)data;
        strncpy(hdr->idtag, tmphdr->idtag, sizeof(hdr->idtag));
        hdr->version = tmphdr->version;
        hdr->jspNodeCount = tmphdr->jspNodeCount;
        hdr->colltree = colltree;
        hdr->jspNodeList = (xJSPNodeInfo*)(tmphdr + 1);

        size -= mark.length + sizeof(__rwMark);

        for (i = 0; i < colltree->numTriangles; i++) {
            colltree->triangles[i].matIndex =
                hdr->jspNodeList[hdr->jspNodeCount - 1 - colltree->triangles[i].v.i.atomIndex].originalMatIndex;
        }

#ifdef GAMECUBE
        if (size >= sizeof(__rwMark)) {
            mp = (__rwMark*)((U8*)data + mark.length);
            memmove(&mark, mp, sizeof(__rwMark));
            RwMemNative32(&mark, sizeof(__rwMark));

            data = mp + 1;

            if (mark.type == 0xBEEF03) {
                hdr->stripVecCount = *(U32*)data;
                data = (U32*)data + 1;
                hdr->stripVecList = (RwV3d*)data;

                sAtomicStartCount = RpClumpGetNumAtomics(hdr->clump);
                sAtomicStartVert = (RwV3d**)RwMalloc(sAtomicStartCount * sizeof(RwV3d*));

                RwV3d* currVec = hdr->stripVecList;
                RpClumpForAllAtomics(hdr->clump, AddAtomicPrecalcedVertCB, &currVec);
            }
        }

        if (hdr->stripVecCount == 0) {
            hdr->stripVecCount = 0;
            sAtomicStartCount = 0;

            RpClumpForAllAtomics(hdr->clump, CountAtomicCB, &hdr->stripVecCount);

            hdr->stripVecList = (RwV3d*)RwMalloc(hdr->stripVecCount * sizeof(RwV3d));
            sAtomicStartVert = (RwV3d**)RwMalloc(sAtomicStartCount * sizeof(RwV3d**));

            RwV3d* currVec = hdr->stripVecList;
            RpClumpForAllAtomics(hdr->clump, AddAtomicCB, &currVec);
        }

        for (i = 0; i < colltree->numTriangles; i++) {
            colltree->triangles[i].v.p =
                sAtomicStartVert[colltree->triangles[i].v.i.atomIndex] + colltree->triangles[i].v.i.meshVertIndex;
        }

        RwFree(sAtomicStartVert);
#endif
    } else {
        RwMemory rwmem;
        rwmem.start = (RwUInt8*)data;
        rwmem.length = size;

        RwStream* stream = RwStreamOpen(rwSTREAMMEMORY, rwSTREAMREAD, &rwmem);
        RwStreamFindChunk(stream, rwID_CLUMP, NULL, NULL);
        clump = RpClumpStreamRead(stream);
        RwStreamClose(stream, NULL);

        if (!hdr->clump) {
            hdr->clump = clump;
        } else {
            S32 i, atomCount;

            atomCount = RpClumpGetNumAtomics(clump);
            if (atomCount) {
                RpAtomic** atomList = (RpAtomic**)RwMalloc(atomCount * sizeof(RpAtomic*));
                RpAtomic** atomCurr = atomList;
                RpClumpForAllAtomics(clump, ListAtomicCB, &atomCurr);

                for (i = atomCount - 1; i >= 0; i--) {
                    RpClumpRemoveAtomic(clump, atomList[i]);
                    RpClumpAddAtomic(hdr->clump, atomList[i]);
                    RpAtomicSetFrame(atomList[i], RpClumpGetFrame(hdr->clump));
                }

                RwFree(atomList);
            }

            RpClumpDestroy(clump);
        }
    }
}

void xJSP_Destroy(xJSPHeader* jsp)
{
    if (globals.sceneCur->env->geom->jsp == jsp &&
        globals.sceneCur->env->geom->world) {
        RpWorldDestroy(globals.sceneCur->env->geom->world);
        globals.sceneCur->env->geom->world = NULL;
    }

    RpClumpDestroy(jsp->clump);
    RwFree(jsp->colltree);

#ifdef GAMECUBE
    U32* tp = (U32*)((xJSPHeaderGC*)jsp)->stripVecList;
    U32 test = *(tp - 4);
    RwMemNative32(&test, sizeof(U32));
    if (test != 0xBEEF03) {
        RwFree(((xJSPHeaderGC*)jsp)->stripVecList);
    }
#endif

    RwFree(jsp);
}
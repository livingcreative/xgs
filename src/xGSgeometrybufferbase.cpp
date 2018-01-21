/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    xGSgeometrybufferbase.cpp
        GeometryBuffer object implementation base class
*/

#include "xGSgeometrybufferbase.h"
#include "kcommon/c_util.h"


using namespace xGS;
using namespace c_util;


xGSGeometryBufferBase::xGSGeometryBufferBase() :
    p_locktype(GS_NONE),
    p_vertexcount(0),
    p_indexcount(0),
    p_currentvertex(0),
    p_currentindex(0),
    p_vertexgranularity(256),
    p_indexgranularity(256),
    p_vertexptr(nullptr),
    p_indexptr(nullptr)
{}

bool xGSGeometryBufferBase::EmitPrimitive(GSenum type, GSuint vertexcount, GSuint indexcount, GSuint flags, GSimmediateprimitive *primitive)
{
    if ((p_currentvertex + vertexcount) > p_vertexcount || (p_currentindex + indexcount) > p_indexcount) {
        return false;
    }

    primitive->stride = p_vertexdecl.buffer_size(); // TODO: optimize stride calcs
    primitive->vertexdata = getp(p_vertexptr, p_currentvertex * primitive->stride);
    primitive->vertexcount = vertexcount;
    primitive->indexdata = getp(p_indexptr, index_buffer_size(p_indexformat, p_currentindex));
    primitive->indexcount = indexcount;

    Primitive p = {
        type,
        p_currentvertex, vertexcount,
        p_currentindex, indexcount
    };
    p_primitives.push_back(p);

    p_currentvertex += vertexcount;
    p_currentindex += indexcount;

    return true;
}

GSbool xGSGeometryBufferBase::allocateGeometry(GSuint vertexcount, GSuint indexcount, GSptr &vertexmemory, GSptr &indexmemory, GSuint &basevertex)
{
    if (p_type == GS_GBTYPE_GEOMETRYHEAP) {
        vertexcount = align(vertexcount, p_vertexgranularity);
        indexcount = align(indexcount, p_indexgranularity);

        size_t commitVertexBlock = GS_UNDEFINED;
        size_t commitIndexBlock = GS_UNDEFINED;

        GSuint currentvertex = p_currentvertex;
        if (!allocateBlock(
            vertexcount, p_vertexdecl.buffer_size(), p_vertexcount, vertexmemory,
            p_currentvertex, p_freevertices, commitVertexBlock
        )) {
            return GS_FALSE;
        }

        if (!allocateBlock(
            indexcount, index_buffer_size(p_indexformat), p_indexcount, indexmemory,
            p_currentindex, p_freeindices, commitIndexBlock
        )) {
            // restore vertex top if it was allocated at top
            p_currentvertex = currentvertex;
            return GS_FALSE;
        }

        basevertex = buffercast(vertexmemory) / p_vertexdecl.buffer_size();

        commitFreeBlock(commitVertexBlock, p_freevertices);
        commitFreeBlock(commitIndexBlock, p_freeindices);
    } else {
        if ((p_currentvertex + vertexcount) > p_vertexcount) {
            return GS_FALSE;
        }
        if ((p_currentindex + indexcount) > p_indexcount) {
            return GS_FALSE;
        }

        vertexmemory = buffercast(p_vertexdecl.buffer_size(p_currentvertex));
        basevertex = p_currentvertex;
        p_currentvertex += vertexcount;
        indexmemory = buffercast(index_buffer_size(p_indexformat, p_currentindex));
        p_currentindex += indexcount;
    }

    return GS_TRUE;
}

void xGSGeometryBufferBase::freeGeometry(GSuint vertexcount, GSuint indexcount, GSptr vertexmemory, GSptr indexmemory)
{
    // freeGeometry can be issued only with heap buffers
    if (p_type != GS_GBTYPE_GEOMETRYHEAP) {
        return;
    }

    freeBlock(
        vertexmemory, align(vertexcount, p_vertexgranularity),
        p_vertexdecl.buffer_size(), p_currentvertex, p_freevertices
    );
    freeBlock(
        indexmemory, align(indexcount, p_indexgranularity),
        index_buffer_size(p_indexformat), p_currentindex, p_freeindices
    );
}

bool xGSGeometryBufferBase::allocateBlock(GSuint count, GSuint elementsize, GSuint maxcount, GSptr &memory, GSuint &current, FreeBlockList &list, size_t &block)
{
    // TODO: test option with allocating at top first, and then free block search

    // find free block of best size match
    GSuint bestmatch = GS_UNDEFINED;
    for (size_t n = 0; n < list.size(); ++n) {
        if (list[n].count >= count && (list[n].count - count) < bestmatch) {
            bestmatch = list[n].count - count;
            block = n;
            if (bestmatch == 0) {
                break;
            }
        }
    }

    if (block == GS_UNDEFINED) {
        // free block of required sized haven't been found, try to allocate at top
        if ((current + count) <= maxcount) {
            memory = buffercast(current * elementsize);
            current += count;
        } else {
            return false;
        }
    } else {
        memory = list[block].memory;

        if (count < list[block].count) {
            list[block].memory = getp(list[block].memory, count * elementsize);
            list[block].count -= count;
            block = GS_UNDEFINED;
        }
    }

    return true;
}

void xGSGeometryBufferBase::commitFreeBlock(size_t block, FreeBlockList &list)
{
    if (block != GS_UNDEFINED) {
        if (block < (list.size() - 1)) {
            list[block] = list.back();
        }
        list.pop_back();
    }
}

void xGSGeometryBufferBase::freeBlock(GSptr memory, GSuint count, GSuint elementsize, GSuint &current, FreeBlockList &list)
{
    if (count == 0) {
        return;
    }

    GSptr top = buffercast(current * elementsize);
    if (top == getp(memory, count * elementsize)) {
        // freeing data at top, just return current pointer back
        current -= count;

        // find free block ending and current top and remove it, lowering top
        GSptr newtop = buffercast(current * elementsize);
        for (size_t n = 0; n < list.size(); ++n) {
            if (newtop == getp(list[n].memory, list[n].count * elementsize)) {
                current -= list[n].count;
                commitFreeBlock(n, list);
                break;
            }
        }
    } else {
        // find neighbour blocks
        GSptr memoryend = getp(memory, count * elementsize);

        size_t mergeleft = GS_UNDEFINED;
        size_t mergeright = GS_UNDEFINED;
        for (size_t n = 0; n < list.size(); ++n) {
            // check left neighbour
            if (memory == getp(list[n].memory, list[n].count * elementsize)) {
                mergeleft = n;
                continue;
            }

            // check right neighbour
            if (memoryend == list[n].memory) {
                mergeright = n;
            }
        }

        if (mergeleft != GS_UNDEFINED && mergeright != GS_UNDEFINED) {
            list[mergeleft].count += count;
            list[mergeleft].count += list[mergeright].count;
            commitFreeBlock(mergeright, list);
        } else if (mergeleft != GS_UNDEFINED) {
            list[mergeleft].count += count;
        } else if (mergeright != GS_UNDEFINED) {
            list[mergeright].memory = memory;
            list[mergeright].count += count;
        } else {
            // freeing data from middle of allocated space, put it to FreeBlock
            FreeBlock block = {
                memory, count
            };
            list.push_back(block);
        }
    }
}

/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    dx11/xGSgeometrybuffer.cpp
        GeometryBuffer object implementation class
*/

#include "xGSgeometrybuffer.h"
#include "xGSutil.h"
#include "kcommon/c_util.h"


using namespace xGS;
using namespace c_util;


xGSGeometryBufferImpl::xGSGeometryBufferImpl(xGSImpl *owner) :
    xGSObjectImpl(owner),
    p_locktype(GS_NONE),
    p_vertexcount(0),
    p_indexcount(0),
    p_currentvertex(0),
    p_currentindex(0),
    p_vertexgranularity(256),
    p_indexgranularity(256),
    p_vertexptr(nullptr),
    p_indexptr(nullptr)
{
    p_owner->debug(DebugMessageLevel::Information, "GeometryBuffer object created\n");
}

xGSGeometryBufferImpl::~xGSGeometryBufferImpl()
{
    ReleaseRendererResources();
    p_owner->debug(DebugMessageLevel::Information, "GeometryBuffer object destroyed\n");
}

GSvalue xGSGeometryBufferImpl::GetValue(GSenum valuetype)
{
    p_owner->error(GSE_UNIMPLEMENTED);
    return 0;
}

GSbool xGSGeometryBufferImpl::allocate(const GSgeometrybufferdescription &desc)
{
    p_type = desc.type;

    p_vertexdecl = GSvertexdecl(desc.vertexdecl);
    p_indexformat = desc.indexformat;

    p_vertexcount = desc.vertexcount;
    p_indexcount = p_indexformat != GS_INDEX_NONE ? desc.indexcount : 0;

    // TODO: xGSGeometryBufferImpl::allocate

    return p_owner->error(GS_OK);
}

GSptr xGSGeometryBufferImpl::Lock(GSenum locktype, GSdword access, void *lockdata)
{
    if (p_locktype || p_type == GS_GBTYPE_IMMEDIATE) {
        p_owner->error(GSE_INVALIDOPERATION);
        return nullptr;
    }

    if (locktype != GS_LOCK_VERTEXDATA && locktype != GS_LOCK_INDEXDATA) {
        p_owner->error(GSE_INVALIDENUM);
        return nullptr;
    }

    if (locktype == GS_LOCK_INDEXDATA && p_indexformat == GS_INDEX_NONE) {
        p_owner->error(GSE_INVALIDVALUE);
        return nullptr;
    }

    p_owner->error(GS_OK);
    return lock(
        locktype, 0,
        locktype == GS_LOCK_VERTEXDATA ?
          p_vertexdecl.buffer_size(p_vertexcount) :
          index_buffer_size(p_indexformat, p_indexcount)
     );
}

GSbool xGSGeometryBufferImpl::Unlock()
{
    if (p_locktype == GS_NONE || p_locktype == LOCK_IMMEDIATE) {
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    unlock();
    p_locktype = GS_NONE;

    return p_owner->error(GS_OK);
}

GSbool xGSGeometryBufferImpl::allocateGeometry(GSuint vertexcount, GSuint indexcount, GSptr &vertexmemory, GSptr &indexmemory, GSuint &basevertex)
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

        basevertex = GSuint(vertexmemory) / p_vertexdecl.buffer_size();

        commitFreeBlock(commitVertexBlock, p_freevertices);
        commitFreeBlock(commitIndexBlock, p_freeindices);
    } else {
        if ((p_currentvertex + vertexcount) > p_vertexcount) {
            return GS_FALSE;
        }
        if ((p_currentindex + indexcount) > p_indexcount) {
            return GS_FALSE;
        }

        vertexmemory = reinterpret_cast<GSptr>(p_vertexdecl.buffer_size(p_currentvertex));
        basevertex = p_currentvertex;
        p_currentvertex += vertexcount;
        indexmemory = reinterpret_cast<GSptr>(index_buffer_size(p_indexformat, p_currentindex));
        p_currentindex += indexcount;
    }

    return GS_TRUE;
}

void xGSGeometryBufferImpl::freeGeometry(GSuint vertexcount, GSuint indexcount, GSptr vertexmemory, GSptr indexmemory)
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

GSptr xGSGeometryBufferImpl::lock(GSenum locktype, size_t offset, size_t size)
{
    p_locktype = locktype;

    // TODO: xGSGeometryBufferImpl::lock

    return nullptr;
}

void xGSGeometryBufferImpl::unlock()
{
    // TODO: xGSGeometryBufferImpl::unlock
}

void xGSGeometryBufferImpl::BeginImmediateDrawing()
{
    p_currentvertex = 0;
    p_currentindex = 0;

    p_primitives.clear();

    p_locktype = LOCK_IMMEDIATE;

    // TODO: xGSGeometryBufferImpl::BeginImmediateDrawing
}

bool xGSGeometryBufferImpl::EmitPrimitive(GSenum type, GSuint vertexcount, GSuint indexcount, GSuint flags, GSimmediateprimitive *primitive)
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
        dx11_primitive_type(type),
        p_currentvertex, vertexcount,
        p_currentindex, indexcount
    };
    p_primitives.push_back(p);

    p_currentvertex += vertexcount;
    p_currentindex += indexcount;

    return true;
}

void xGSGeometryBufferImpl::EndImmediateDrawing()
{
     // TODO: xGSGeometryBufferImpl::EndImmediateDrawing

     p_vertexptr = nullptr;
     p_indexptr = nullptr;

     // NOTE: primitives list isn't released here!!!
     //         because after calling EndImmediateDrawing
     //         implementation should issue rendering commands to
     //         render cached primitives
}

void xGSGeometryBufferImpl::ReleaseRendererResources()
{
    // TODO: xGSGeometryBufferImpl::ReleaseRendererResources
}

bool xGSGeometryBufferImpl::allocateBlock(GSuint count, GSuint elementsize, GSuint maxcount, GSptr &memory, GSuint &current, FreeBlockList &list, size_t &block)
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
            memory = reinterpret_cast<GSptr>(current * elementsize);
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

void xGSGeometryBufferImpl::commitFreeBlock(size_t block, FreeBlockList &list)
{
    if (block != GS_UNDEFINED) {
        if (block < (list.size() - 1)) {
            list[block] = list.back();
        }
        list.pop_back();
    }
}

void xGSGeometryBufferImpl::freeBlock(GSptr memory, GSuint count, GSuint elementsize, GSuint &current, FreeBlockList &list)
{
    if (count == 0) {
        return;
    }

    GSptr top = reinterpret_cast<GSptr>(current * elementsize);
    if (top == getp(memory, count * elementsize)) {
        // freeing data at top, just return current pointer back
        current -= count;

        // find free block ending and current top and remove it, lowering top
        GSptr newtop = reinterpret_cast<GSptr>(current * elementsize);
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

/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    xGSimplbase.cpp
        xGS API object common implementation class
*/

#include "xGSimplbase.h"
#include "xGSgeometrybuffer.h"
#include "xGSdatabuffer.h"
#include "xGStexture.h"
#include "xGSframebuffer.h"
#include "xGSstate.h"
#include "xGSinput.h"
#include "xGSparameters.h"
#include <cstdlib>

#ifdef _DEBUG
    #include <cstdarg>

    #ifdef WIN32
        #include <Windows.h>
    #endif

    #ifdef WIN32
        #define DEBUG_PRINT(T) OutputDebugStringA(T)
    #endif

    #ifdef __APPLE__
        #define DEBUG_PRINT(T) printf(T)
    #endif
#endif



using namespace xGS;


xGSDataBufferBase::xGSDataBufferBase() :
    p_size(0),
    p_locktype(GS_NONE)
{}



xGSFrameBufferBase::xGSFrameBufferBase() :
    p_width(0),
    p_height(0),
    p_colortargets(1),
    p_srgb(false),
    p_activecolortargets(0),
    p_depthstencilformat(GS_DEPTH_NONE),
    p_multisample(GS_MULTISAMPLE_NONE)
{
    for (int n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        p_colorformat[n] = GS_COLOR_DEFAULT;
    }
}

GSsize xGSFrameBufferBase::size() const
{
    GSsize result;
    result.width = p_width;
    result.height = p_height;
    return result;
}

static inline GSenum RTFormat(GSenum format, xGSTextureImpl *attachment)
{
    if (format != GS_DEFAULT) {
        return format;
    } else {
        return attachment ? attachment->format() : GS_NONE;
    }
}

void xGSFrameBufferBase::getformats(GSenum *colorformats, GSenum &depthstencilformat) const
{
    for (size_t n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        colorformats[n] = RTFormat(p_colorformat[n], p_colortextures[n].p_texture);
    }
    depthstencilformat = RTFormat(p_depthstencilformat, p_depthtexture.p_texture);
}


xGSFrameBufferBase::Attachment::~Attachment()
{
    if (p_texture) {
        p_texture->Release();
    }
}

void xGSFrameBufferBase::Attachment::attach(xGSTextureImpl *texture, GSuint level, GSuint slice)
{
    if (p_texture) {
        p_texture->Release();
    }

    p_texture = texture;
    p_level = level;
    p_slice = slice;

    if (p_texture) {
        p_texture->AddRef();
    }
}



IxGSGeometryImpl::IxGSGeometryImpl(xGSImplBase *owner) :
    xGSObjectImpl(owner),

    p_type(GS_NONE),
    p_indexformat(GS_NONE),
    p_vertexcount(0),
    p_indexcount(0),
    p_patch_vertices(0),
    p_restart(false),
    p_restartindex(GS_UNDEFINED),

    p_locktype(GS_NONE),
    p_lockpointer(nullptr),

    p_basevertex(0),
    p_vertexmemory(nullptr),
    p_indexmemory(nullptr),

    p_sharedgeometry(nullptr),
    p_buffer(nullptr)
{
    p_owner->debug(DebugMessageLevel::Information, "Geometry object created\n");
}

IxGSGeometryImpl::~IxGSGeometryImpl()
{
    if (p_locktype) {
        doUnlock();
    }

    if (p_sharedgeometry) {
        p_sharedgeometry->Release();
    }

    if (p_buffer) {
        p_buffer->freeGeometry(p_vertexcount, p_indexcount, p_vertexmemory, p_indexmemory);
        p_buffer->Release();
    }

    p_owner->debug(DebugMessageLevel::Information, "Geometry object destroyed\n");
}

GSvalue IxGSGeometryImpl::GetValue(GSenum valuetype)
{
    switch (valuetype) {
        case GS_GEOMETRY_TYPE:          return p_type;
        case GS_GEOMETRY_VERTEXCOUNT:   return p_vertexcount;
        case GS_GEOMETRY_INDEXCOUNT:    return p_indexcount;
        case GS_GEOMETRY_INDEXFORMAT:   return p_indexformat;
        case GS_GEOMETRY_VERTEXBYTES:   return p_buffer->vertexDecl().buffer_size(p_vertexcount);
        case GS_GEOMETRY_INDEXBYTES:    return index_buffer_size(p_indexformat, p_indexcount);
        case GS_GEOMETRY_PATCHVERTICES: return p_patch_vertices;
        case GS_GEOMETRY_RESTART:       return p_restart;
        case GS_GEOMETRY_RESTARTINDEX:  return p_restartindex;
        default:
            p_owner->error(GSE_INVALIDENUM);
            return GS_NONE;
    }
}

GSbool IxGSGeometryImpl::allocate(const GSgeometrydescription &desc)
{
    p_type = desc.type;
    p_indexformat = desc.indexformat;
    p_vertexcount = desc.vertexcount;
    p_indexcount = desc.indexcount;
    p_patch_vertices = desc.patchvertices;
    p_restart = desc.restart;
    p_restartindex = desc.restartindex;

    if (desc.sharemode == GS_NONE) {
        if (!desc.buffer) {
            return p_owner->error(GSE_INVALIDOBJECT);
        }

        xGSGeometryBufferImpl *bufferimpl = static_cast<xGSGeometryBufferImpl*>(desc.buffer);
        GSenum indexformat = bufferimpl->indexFormat();

        if (bufferimpl->type() == GS_GBTYPE_IMMEDIATE) {
            return p_owner->error(GSE_INVALIDOBJECT);
        }

        if (!checkAlloc(indexformat))
            return GS_FALSE;

        if (p_indexformat == GS_INDEX_NONE) {
            p_indexcount = 0;
        }

        if (!bufferimpl->allocateGeometry(p_vertexcount, p_indexcount, p_vertexmemory, p_indexmemory, p_basevertex)) {
            return p_owner->error(GSE_OUTOFRESOURCES);
        }

        p_buffer = bufferimpl;
        p_buffer->AddRef();
    } else {
        if (!desc.sharedgeometry) {
            return p_owner->error(GSE_INVALIDOBJECT);
        }

        if (desc.sharemode != GS_SHARE_ALL && desc.sharemode != GS_SHARE_VERTICESONLY) {
            return p_owner->error(GSE_INVALIDENUM);
        }

        IxGSGeometryImpl *geometryimpl = static_cast<IxGSGeometryImpl*>(desc.sharedgeometry);

        if (geometryimpl->p_sharedgeometry) {
            return p_owner->error(GSE_INVALIDOBJECT);
        }

        if (p_vertexcount > geometryimpl->p_vertexcount) {
            return p_owner->error(GSE_INVALIDVALUE);
        }

        if (desc.sharemode == GS_SHARE_ALL && p_indexcount > geometryimpl->p_indexcount) {
            return p_owner->error(GSE_INVALIDVALUE);
        }

        if (!checkAlloc(geometryimpl->p_indexformat)) {
            return GS_FALSE;
        }

        p_buffer = geometryimpl->p_buffer;
        p_buffer->AddRef();

        if (p_indexformat == GS_INDEX_NONE) {
            p_indexcount = 0;
        }

        p_vertexmemory = geometryimpl->p_vertexmemory;
        p_basevertex = geometryimpl->p_basevertex;

        switch (desc.sharemode) {
            case GS_SHARE_ALL:
                // indices from base geometry
                p_indexmemory = geometryimpl->p_indexmemory;
                break;

            case GS_SHARE_VERTICESONLY:
                if (p_indexcount == 0) {
                    p_indexmemory = nullptr;
                } else {
                    GSptr vertexmemory = nullptr;
                    GSuint basevertex = 0;
                    if (!geometryimpl->p_buffer->allocateGeometry(0, p_indexcount, vertexmemory, p_indexmemory, basevertex)) {
                        return p_owner->error(GSE_OUTOFRESOURCES);
                    }
                }
                break;
        }

        p_sharedgeometry = geometryimpl;
        p_sharedgeometry->AddRef();
    }

    return p_owner->error(GS_OK);
}

GSptr IxGSGeometryImpl::Lock(GSenum locktype, GSdword access, void *lockdata)
{
    if (p_locktype) {
        p_owner->error(GSE_INVALIDOPERATION);
        return nullptr;
    }

    if (locktype == GS_LOCK_INDEXDATA && p_indexformat == GS_INDEX_NONE) {
        p_owner->error(GSE_INVALIDVALUE);
        return nullptr;
    }

    switch (locktype) {
        case GS_LOCK_VERTEXDATA:
            p_locktype = locktype;
            p_lockpointer = p_buffer->LockImpl(
                locktype,
                size_t(p_vertexmemory),
                p_buffer->vertexDecl().buffer_size(p_vertexcount)
            );
            break;

        case GS_LOCK_INDEXDATA:
            p_locktype = locktype;
            p_lockpointer = p_buffer->LockImpl(
                locktype,
                size_t(p_indexmemory),
                index_buffer_size(p_indexformat, p_indexcount)
            );
            break;

        default:
            p_owner->error(GSE_INVALIDENUM);
            return nullptr;
    }

    return p_lockpointer;
}

GSbool IxGSGeometryImpl::Unlock()
{
    if (p_locktype == GS_NONE) {
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    doUnlock();

    return p_owner->error(GS_OK);
}

bool IxGSGeometryImpl::checkAlloc(GSenum indexformat)
{
    if (!p_type) {
        return p_owner->error(GSE_INVALIDOBJECT);
    }

    if (p_vertexcount == 0) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    if (p_indexformat != GS_INDEX_NONE && p_indexformat != indexformat) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    if (p_indexcount == 0 && p_indexformat != GS_INDEX_NONE) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    return p_owner->error(GS_OK);
}

void IxGSGeometryImpl::doUnlock()
{
    if (p_buffer) {
        p_buffer->UnlockImpl();
    }

    p_locktype = GS_NONE;
    p_lockpointer = nullptr;
}



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



xGSInputBase::xGSInputBase() :
    p_state(nullptr),
    p_primarybuffer(nullptr),
    p_buffers()
{}



xGSTextureBase::xGSTextureBase() :
    p_texturetype(GS_TEXTYPE_EMPTY),
    p_format(0),
    p_width(0),
    p_height(0),
    p_depth(0),
    p_layers(0),
    p_multisample(GS_MULTISAMPLE_NONE),
    p_minlevel(0),
    p_maxlevel(1000),
    p_locktype(GS_NONE)
{
#ifdef _DEBUG
    p_boundasrt = 0;
#endif
}


// xGS system object instance
IxGS xGSImplBase::gs = nullptr;


xGSImplBase::xGSImplBase() :
    p_systemstate(SYSTEM_NOTREADY),

    p_geometrylist(),
    p_geometrybufferlist(),
    p_databufferlist(),
    p_texturelist(),
    p_framebufferlist(),
    p_statelist(),
    p_inputlist(),
    p_parameterslist(),

    p_rendertarget(nullptr),
    p_state(nullptr),
    p_input(nullptr),
    p_capturebuffer(nullptr),
    p_immediatebuffer(nullptr)
{
    for (size_t n = 0; n < GS_MAX_PARAMETER_SETS; ++n) {
        p_parameters[n] = nullptr;
    }

#ifdef _DEBUG
    dbg_memory_allocs = 0;
#endif
}

xGSImplBase::~xGSImplBase()
{
    gs = nullptr;
}

GSptr xGSImplBase::allocate(GSuint size)
{
#if defined(_DEBUG) || defined(DEBUG)
    dbg_memory_allocs += size;
#endif
    return malloc(size_t(size));
}

void xGSImplBase::free(GSptr &memory)
{
#if defined(_DEBUG) || defined(DEBUG)
    dbg_memory_allocs -= GSuint(_msize(memory));
#endif
    ::free(memory);
    memory = nullptr;
}

bool xGSImplBase::error(GSerror code, DebugMessageLevel level)
{
    p_error = code;
    if (p_error != GS_OK) {
        debug(level, "Code: 0x%X\n", code);

#ifdef _MSC_VER
        //_CrtDbgBreak();
#endif
    }

    return code == GS_OK;
}

void xGSImplBase::debug(DebugMessageLevel level, const char *format, ...)
{
#ifdef _DEBUG
    va_list args;
    va_start(args, format);

    if (*format && (*format != '\n')) {
        switch (level) {
            case DebugMessageLevel::Information:
                DEBUG_PRINT("xGS INFO: ");
                break;

            case DebugMessageLevel::Warning:
                DEBUG_PRINT("xGS WARNING: ");
                break;

            case DebugMessageLevel::Error:
                DEBUG_PRINT("xGS ERROR: ");
                break;

            case DebugMessageLevel::SystemError:
                DEBUG_PRINT("xGS SYSTEM ERROR: ");
                break;
        }
    }

#ifdef WIN32
    char buf[4096];
    vsprintf_s(buf, format, args);

    OutputDebugStringA(buf);
#endif

#ifdef __APPLE__
    vprintf(format, args);
#endif

    va_end(args);
#endif
}

#define GS_ADD_REMOVE_OBJECT_IMPL(list, type)\
template <> void xGSImplBase::AddObject(type *object)\
{\
    list.insert(object);\
}\
\
template <> void xGSImplBase::RemoveObject(type *object)\
{\
    list.erase(object);\
}

GS_ADD_REMOVE_OBJECT_IMPL(p_geometrylist, IxGSGeometryImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_geometrybufferlist, IxGSGeometryBufferImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_databufferlist, IxGSDataBufferImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_texturelist, IxGSTextureImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_framebufferlist, IxGSFrameBufferImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_statelist, xGSStateImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_inputlist, IxGSInputImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_parameterslist, xGSParametersImpl)

#undef GS_ADD_REMOVE_OBJECT_IMPL

/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    xGSgeometry.cpp
        Geometry object implementation class
*/

#include "xGSgeometry.h"
#include "xGSgeometrybuffer.h"


using namespace xGS;


xGSGeometryImpl::xGSGeometryImpl(xGSImpl *owner) :
    xGSObject(owner),

    p_allocated(false),

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

xGSGeometryImpl::~xGSGeometryImpl()
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
    } else {
        if (p_vertexmemory)
            p_owner->free(p_vertexmemory);

        if (p_indexmemory)
            p_owner->free(p_indexmemory);
    }

    p_owner->debug(DebugMessageLevel::Information, "Geometry object destroyed\n");
}


GSvalue xGSGeometryImpl::GetValue(GSenum valuetype)
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

GSbool xGSGeometryImpl::allocate(const GSgeometrydescription &desc)
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

        if (!bufferimpl->allocated() || bufferimpl->type() == GS_GBTYPE_IMMEDIATE) {
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

        xGSGeometryImpl *geometryimpl = static_cast<xGSGeometryImpl*>(desc.sharedgeometry);

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

    p_allocated = true;

    return p_owner->error(GS_OK);
}

GSptr xGSGeometryImpl::Lock(GSenum locktype, GSdword access, void *lockdata)
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
            p_lockpointer = p_buffer->lock(
                locktype,
                size_t(p_vertexmemory),
                p_buffer->vertexDecl().buffer_size(p_vertexcount)
            );
            break;

        case GS_LOCK_INDEXDATA:
            p_locktype = locktype;
            p_lockpointer = p_buffer->lock(
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

GSbool xGSGeometryImpl::Unlock()
{
    if (p_locktype == GS_NONE) {
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    doUnlock();

    return p_owner->error(GS_OK);
}

void xGSGeometryImpl::setup() const
{
    // set up patch parameters
    if (p_type == GS_PRIM_PATCHES) {
        glPatchParameteri(GL_PATCH_VERTICES, p_patch_vertices);
    }

    // set up restart parameters
    if (!p_restart) {
        glDisable(GL_PRIMITIVE_RESTART);
    } else {
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(p_restartindex);
    }
}

bool xGSGeometryImpl::checkAlloc(GSenum indexformat)
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

void xGSGeometryImpl::doUnlock()
{
    if (p_buffer) {
        p_buffer->unlock();
    }

    p_locktype = GS_NONE;
    p_lockpointer = nullptr;
}

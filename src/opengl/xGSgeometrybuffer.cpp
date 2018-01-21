/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/xGSgeometrybuffer.cpp
        GeometryBuffer object implementation class
*/

#include "xGSgeometrybuffer.h"
#include "xGSutil.h"
#include "kcommon/c_util.h"


using namespace xGS;
using namespace c_util;


xGSGeometryBufferImpl::xGSGeometryBufferImpl(xGSImpl *owner) :
    p_vertexbuffer(0),
    p_indexbuffer(0)
{}

xGSGeometryBufferImpl::~xGSGeometryBufferImpl()
{}

GSbool xGSGeometryBufferImpl::allocate(const GSgeometrybufferdescription &desc)
{
    GLenum usage = 0;

#ifdef GS_CONFIG_BUFFER_STORAGE
    if (glBufferStorage) {
        switch (desc.type) {
            case GS_GBTYPE_STATIC:
                usage = GL_MAP_WRITE_BIT;
                break;

            case GS_GBTYPE_GEOMETRYHEAP:
                usage = GL_MAP_WRITE_BIT;
                break;

            case GS_GBTYPE_IMMEDIATE:
                // TODO: implement double/triple buffering and persistent mapping
                usage = GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT;
                break;

            default:
                return p_owner->error(GSE_INVALIDENUM);
        }
    } else {
        switch (desc.type) {
            case GS_GBTYPE_STATIC:
                usage = GL_STATIC_DRAW;
                break;

            case GS_GBTYPE_GEOMETRYHEAP:
                usage = GL_STATIC_DRAW;
                break;

            case GS_GBTYPE_IMMEDIATE:
                // TODO: implement double/triple buffering and persistent mapping
                usage = GL_DYNAMIC_DRAW;
                break;

            default:
                return p_owner->error(GSE_INVALIDENUM);
        }
    }
#else
    switch (desc.type) {
        case GS_GBTYPE_STATIC:
            usage = GL_STATIC_DRAW;
            break;

        case GS_GBTYPE_GEOMETRYHEAP:
            usage = GL_STATIC_DRAW;
            break;

        case GS_GBTYPE_IMMEDIATE:
            // TODO: implement double/triple buffering and persistent mapping
            usage = GL_DYNAMIC_DRAW;
            break;

        default:
            return p_owner->error(GSE_INVALIDENUM);
    }
#endif

    p_type = desc.type;

    p_vertexdecl = GSvertexdecl(desc.vertexdecl);
    p_indexformat = desc.indexformat;

    p_vertexcount = desc.vertexcount;
    p_indexcount = p_indexformat != GS_INDEX_NONE ? desc.indexcount : 0;

    glGenBuffers(1, &p_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, p_vertexbuffer);

#ifdef GS_CONFIG_BUFFER_STORAGE
    if (glBufferStorage) {
        glBufferStorage(
            GL_ARRAY_BUFFER, p_vertexdecl.buffer_size(p_vertexcount),
            nullptr, usage
        );
    } else {
        glBufferData(
            GL_ARRAY_BUFFER, p_vertexdecl.buffer_size(p_vertexcount),
            nullptr, usage
        );
    }
#else
    glBufferData(
        GL_ARRAY_BUFFER, p_vertexdecl.buffer_size(p_vertexcount),
        nullptr, usage
    );
#endif

    if (p_indexformat != GS_INDEX_NONE) {
        glGenBuffers(1, &p_indexbuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_indexbuffer);

#ifdef GS_CONFIG_BUFFER_STORAGE
        if (glBufferStorage) {
            glBufferStorage(
                GL_ELEMENT_ARRAY_BUFFER, index_buffer_size(p_indexformat, p_indexcount),
                nullptr, usage
            );
        } else {
            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER, index_buffer_size(p_indexformat, p_indexcount),
                nullptr, usage
            );
        }
#else
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, index_buffer_size(p_indexformat, p_indexcount),
            nullptr, usage
        );
#endif
    }

    return p_owner->error(GS_OK);
}

GSptr xGSGeometryBufferImpl::LockImpl(GSenum locktype, size_t offset, size_t size)
{
    // NOTE: locktype must be valid here and all checks should be passed
    p_locktype = locktype;

    switch (locktype) {
        case GS_LOCK_VERTEXDATA:
            glBindBuffer(GL_ARRAY_BUFFER, p_vertexbuffer);

#ifdef GS_CONFIG_MAP_BUFFER_RANGE
            return glMapBufferRange(GL_ARRAY_BUFFER, offset, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
#else
            return getp(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY), offset);
#endif

        case GS_LOCK_INDEXDATA:
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_indexbuffer);

#ifdef GS_CONFIG_MAP_BUFFER_RANGE
            return glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, offset, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
#else
            return getp(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY), offset);
#endif
    }

    return nullptr;
}

void xGSGeometryBufferImpl::UnlockImpl()
{
    switch (p_locktype) {
        case GS_LOCK_VERTEXDATA:
            glBindBuffer(GL_ARRAY_BUFFER, p_vertexbuffer);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            break;

        case GS_LOCK_INDEXDATA:
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_indexbuffer);
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
            break;
    }
    p_locktype = GS_NONE;
}

void xGSGeometryBufferImpl::BeginImmediateDrawingImpl()
{
    glBindBuffer(GL_ARRAY_BUFFER, p_vertexbuffer);
    p_vertexptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    if (p_indexformat != GS_INDEX_NONE) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_indexbuffer);
        p_indexptr = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    }
}

void xGSGeometryBufferImpl::EndImmediateDrawingImpl()
{
     glBindBuffer(GL_ARRAY_BUFFER, p_vertexbuffer);
     glUnmapBuffer(GL_ARRAY_BUFFER);

     if (p_indexformat != GS_INDEX_NONE) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_indexbuffer);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
     }
}

void xGSGeometryBufferImpl::ReleaseRendererResources()
{
    if (p_vertexbuffer) {
        glDeleteBuffers(1, &p_vertexbuffer);
        p_vertexbuffer = 0;
    }

    if (p_indexbuffer) {
        glDeleteBuffers(1, &p_indexbuffer);
        p_indexbuffer = 0;
    }
}

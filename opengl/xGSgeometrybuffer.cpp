/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    xGSgeometrybuffer.cpp
        GeometryBuffer object implementation class
*/

#include "xGSgeometrybuffer.h"
#include "xGSutil.h"
#include "kcommon/c_util.h"


using namespace xGS;
using namespace c_util;


xGSGeometryBufferImpl::xGSGeometryBufferImpl(xGSImpl *owner) :
    xGSObject(owner),
    p_vertexbuffer(0),
    p_indexbuffer(0),
    p_locktype(GS_NONE),
    p_vertexcount(0),
    p_indexcount(0),
    p_currentvertex(0),
    p_currentindex(0),
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
    GLenum usage = 0;

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

    p_type = desc.type;

    p_vertexdecl = GSvertexdecl(desc.vertexdecl);
    p_indexformat = desc.indexformat;

    p_vertexcount = desc.vertexcount;
    p_indexcount = p_indexformat != GS_INDEX_NONE ? desc.indexcount : 0;

    glGenBuffers(1, &p_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, p_vertexbuffer);
    glBufferData(
        GL_ARRAY_BUFFER, p_vertexdecl.buffer_size(p_vertexcount),
        nullptr, usage
    );

    if (p_indexformat != GS_INDEX_NONE) {
        glGenBuffers(1, &p_indexbuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_indexbuffer);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, index_buffer_size(p_indexformat, p_indexcount),
            nullptr, usage
        );
    }

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
    vertexmemory = reinterpret_cast<GSptr>(p_vertexdecl.buffer_size(p_currentvertex));
    basevertex = p_currentvertex;
    p_currentvertex += vertexcount;
    indexmemory = reinterpret_cast<GSptr>(index_buffer_size(p_indexformat, p_currentindex));
    p_currentindex += indexcount;

    return GS_TRUE;
}

void xGSGeometryBufferImpl::freeGeometry(GSuint vertexcount, GSuint indexcount, GSptr vertexmemory, GSptr indexmemory)
{
}

GSptr xGSGeometryBufferImpl::lock(GSenum locktype, size_t offset, size_t size)
{
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

void xGSGeometryBufferImpl::unlock()
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
}

void xGSGeometryBufferImpl::BeginImmediateDrawing()
{
    p_currentvertex = 0;
    p_currentindex = 0;

    p_primitives.clear();

    p_locktype = LOCK_IMMEDIATE;

    glBindBuffer(GL_ARRAY_BUFFER, p_vertexbuffer);
    p_vertexptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    if (p_indexformat != GS_INDEX_NONE) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_indexbuffer);
        p_indexptr = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    }
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
        gl_primitive_type(type),
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
     glBindBuffer(GL_ARRAY_BUFFER, p_vertexbuffer);
     glUnmapBuffer(GL_ARRAY_BUFFER);

     if (p_indexformat != GS_INDEX_NONE) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_indexbuffer);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
     }

     p_vertexptr = nullptr;
     p_indexptr = nullptr;

     // NOTE: primitives list isn't released here!!!
     //         because after calling EndImmediateDrawing
     //         implementation should issue rendering commands to
     //         render cached primitives
}

void xGSGeometryBufferImpl::ReleaseRendererResources()
{
    if (p_vertexbuffer) {
        glDeleteBuffers(1, &p_vertexbuffer);
        p_vertexbuffer = 0;
    }

    if (p_indexbuffer) {
        glDeleteBuffers(1, &p_indexbuffer);
    }
}

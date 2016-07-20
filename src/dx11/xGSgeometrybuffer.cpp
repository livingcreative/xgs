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
#include <d3d11.h>


using namespace xGS;
using namespace c_util;


xGSGeometryBufferImpl::xGSGeometryBufferImpl(xGSImpl *owner) :
    xGSObjectImpl(owner)
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

    // TODO: geometry buffers usage
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.Usage = D3D11_USAGE_DEFAULT;
    vertexbufferdesc.ByteWidth = p_vertexdecl.buffer_size(p_vertexcount);
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexbufferdesc.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;

    p_owner->device()->CreateBuffer(&vertexbufferdesc, nullptr, &p_vertexbuffer);

    D3D11_BUFFER_DESC indexbufferdesc = {};
    indexbufferdesc.Usage = D3D11_USAGE_DEFAULT;
    indexbufferdesc.ByteWidth = index_buffer_size(p_indexformat, p_indexcount);
    indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexbufferdesc.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;

    p_owner->device()->CreateBuffer(&indexbufferdesc, nullptr, &p_indexbuffer);

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

GSptr xGSGeometryBufferImpl::lock(GSenum locktype, size_t offset, size_t size)
{
    p_locktype = locktype;

    // TODO: xGSGeometryBufferImpl::lock

    p_lockmemory = new char[size];
    p_lockoffset = offset;
    p_locksize = size;
    return p_lockmemory;
}

void xGSGeometryBufferImpl::unlock()
{
    D3D11_BOX box = {};
    box.left = p_lockoffset;
    box.right = p_lockoffset + p_locksize;

    switch (p_locktype) {
        case GS_LOCK_VERTEXDATA:
            p_owner->context()->UpdateSubresource(p_vertexbuffer, 0, &box, p_lockmemory, 0, 0);
            break;

        case GS_LOCK_INDEXDATA:
            p_owner->context()->UpdateSubresource(p_indexbuffer, 0, &box, p_lockmemory, 0, 0);
            break;
    }

    delete[] p_lockmemory;
}

void xGSGeometryBufferImpl::BeginImmediateDrawing()
{
    p_currentvertex = 0;
    p_currentindex = 0;

    p_primitives.clear();

    p_locktype = LOCK_IMMEDIATE;

    // TODO: xGSGeometryBufferImpl::BeginImmediateDrawing
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
    ::Release(p_vertexbuffer);
    ::Release(p_indexbuffer);
}

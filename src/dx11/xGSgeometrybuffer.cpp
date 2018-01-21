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
    p_vertexbuffer(nullptr),
    p_indexbuffer(nullptr)
{}

xGSGeometryBufferImpl::~xGSGeometryBufferImpl()
{}

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

GSptr xGSGeometryBufferImpl::LockImpl(GSenum locktype, size_t offset, size_t size)
{
    p_locktype = locktype;

    // TODO: xGSGeometryBufferImpl::lock

    p_lockmemory = new char[size];
    p_lockoffset = offset;
    p_locksize = size;

    return p_lockmemory;
}

void xGSGeometryBufferImpl::UnlockImpl()
{
    D3D11_BOX box = {};
    box.left = UINT(p_lockoffset);
    box.right = UINT(p_lockoffset + p_locksize);
    box.bottom = 1;
    box.back = 1;

    switch (p_locktype) {
    case GS_LOCK_VERTEXDATA:
        p_owner->context()->UpdateSubresource(p_vertexbuffer, 0, &box, p_lockmemory, 0, 0);
        break;

    case GS_LOCK_INDEXDATA:
        p_owner->context()->UpdateSubresource(p_indexbuffer, 0, &box, p_lockmemory, 0, 0);
        break;
    }

    delete[] p_lockmemory;

    p_locktype = GS_NONE;
}

void xGSGeometryBufferImpl::BeginImmediateDrawingImpl()
{
    p_currentvertex = 0;
    p_currentindex = 0;

    p_primitives.clear();

    p_locktype = LOCK_IMMEDIATE;

    // TODO: xGSGeometryBufferImpl::BeginImmediateDrawing
}

void xGSGeometryBufferImpl::EndImmediateDrawingImpl()
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

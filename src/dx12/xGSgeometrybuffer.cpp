/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/xgs

    dx12/xGSgeometrybuffer.cpp
        GeometryBuffer object implementation class
*/

#include "xGSgeometrybuffer.h"
#include "xGSutil.h"
#include "kcommon/c_util.h"
#include <d3d12.h>


using namespace xGS;
using namespace c_util;


xGSGeometryBufferImpl::xGSGeometryBufferImpl(xGSImpl *owner) :
    xGSObjectImpl(owner),
    p_vertexbuffer(nullptr),
    p_indexbuffer(nullptr),
    p_lockbuffer(nullptr)
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

    D3D12_HEAP_PROPERTIES heapprops = {
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1, 1
    };

    D3D12_RESOURCE_DESC bufferdesc = {
        D3D12_RESOURCE_DIMENSION_BUFFER,
        0,
        p_vertexdecl.buffer_size(p_vertexcount), 1, 1, 1,
        DXGI_FORMAT_UNKNOWN 
    };
    bufferdesc.SampleDesc.Count = 1;
    bufferdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    p_owner->device()->CreateCommittedResource(
        &heapprops, D3D12_HEAP_FLAG_NONE, &bufferdesc, D3D12_RESOURCE_STATE_COMMON,
        nullptr, IID_PPV_ARGS(&p_vertexbuffer)
    );

    if (p_indexcount) {
        bufferdesc.Width = index_buffer_size(p_indexformat, p_indexcount);

        p_owner->device()->CreateCommittedResource(
            &heapprops, D3D12_HEAP_FLAG_NONE, &bufferdesc, D3D12_RESOURCE_STATE_COMMON,
            nullptr, IID_PPV_ARGS(&p_indexbuffer)
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

GSptr xGSGeometryBufferImpl::lock(GSenum locktype, size_t offset, size_t size)
{
    p_locktype = locktype;

    // TODO: xGSGeometryBufferImpl::lock

    D3D12_HEAP_PROPERTIES heapprops = {
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1, 1
    };

    D3D12_RESOURCE_DESC lockbufferdesc = {
        D3D12_RESOURCE_DIMENSION_BUFFER,
        0,
        p_locktype == GS_LOCK_VERTEXDATA ?
            p_vertexdecl.buffer_size(p_vertexcount) :
            index_buffer_size(p_indexformat, p_indexcount),
        1, 1, 1,
        DXGI_FORMAT_UNKNOWN
    };
    lockbufferdesc.SampleDesc.Count = 1;
    lockbufferdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    p_owner->device()->CreateCommittedResource(
        &heapprops, D3D12_HEAP_FLAG_NONE, &lockbufferdesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr, IID_PPV_ARGS(&p_lockbuffer)
    );

    D3D12_RANGE range = { offset, offset + size };
    p_lockbuffer->Map(0, &range, &p_lockmemory);

    p_lockoffset = offset;
    p_locksize = size;

    return p_lockmemory;
}

void xGSGeometryBufferImpl::unlock()
{
    p_lockbuffer->Unmap(0, nullptr);

    switch (p_locktype) {
        case GS_LOCK_VERTEXDATA:
            p_owner->UploadBufferData(p_lockbuffer, p_vertexbuffer, p_lockoffset, p_locksize);
            break;

        case GS_LOCK_INDEXDATA:
            p_owner->UploadBufferData(p_lockbuffer, p_indexbuffer, p_lockoffset, p_locksize);
            break;
    }

    ::Release(p_lockbuffer);
    p_lockmemory = nullptr;
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

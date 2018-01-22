/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx12/xGSdatabuffer.cpp
        DataBuffer object implementation class
*/

#include "xGSdatabuffer.h"
#include "xGSstate.h"


using namespace xGS;


xGSDataBufferImpl::xGSDataBufferImpl(xGSImpl *owner) :
    xGSObjectBase(owner),
    p_buffer(nullptr)
{}

xGSDataBufferImpl::~xGSDataBufferImpl()
{}

GSbool xGSDataBufferImpl::AllocateImpl(const GSdatabufferdescription &desc, GSuint totalsize)
{
    D3D12_HEAP_PROPERTIES heapprops = {
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1, 1
    };

    D3D12_RESOURCE_DESC lockbufferdesc = {
        D3D12_RESOURCE_DIMENSION_BUFFER,
        0, p_size, 1, 1, 1,
        DXGI_FORMAT_UNKNOWN
    };
    lockbufferdesc.SampleDesc.Count = 1;
    lockbufferdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    p_owner->device()->CreateCommittedResource(
        &heapprops, D3D12_HEAP_FLAG_NONE, &lockbufferdesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr, IID_PPV_ARGS(&p_buffer)
    );

    return GS_TRUE;
}

void xGSDataBufferImpl::UpdateImpl(GSuint offset, GSuint size, const GSptr data)
{
    // TODO: is there any "update" analogue in DX12?
    D3D12_RANGE range = { offset, offset + size };
    void *mem = nullptr;
    p_buffer->Map(0, &range, &mem);
    memcpy(mem, data, size);
    p_buffer->Unmap(0, nullptr);
}

GSptr xGSDataBufferImpl::LockImpl(GSdword access)
{
    p_locktype = GS_LOCKED;

    // TODO: lock DX12 data buffer
    void *mem = nullptr;
    D3D12_RANGE range = { 0, p_size };
    p_buffer->Map(0, &range, &mem);

    return mem;
}

void xGSDataBufferImpl::UnlockImpl()
{
    p_locktype = GS_NONE;

    p_buffer->Unmap(0, nullptr);
}

void xGSDataBufferImpl::ReleaseRendererResources()
{
    ::Release(p_buffer);
}

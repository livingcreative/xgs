/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx11/xGSdatabuffer.cpp
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
    // TODO: allocate DX11 data buffer
    D3D11_BUFFER_DESC bufferdesc = {};
    bufferdesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferdesc.ByteWidth = p_size;
    bufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    p_owner->device()->CreateBuffer(&bufferdesc, nullptr, &p_buffer);

    return GS_TRUE;
}

void xGSDataBufferImpl::UpdateImpl(GSuint offset, GSuint size, const GSptr data)
{
    // TODO: update DX11 data buffer
}

GSptr xGSDataBufferImpl::LockImpl(GSdword access)
{
    p_locktype = GS_LOCKED;

    // TODO: lock DX11 data buffer

    return nullptr;
}

void xGSDataBufferImpl::UnlockImpl()
{
    p_locktype = GS_NONE;

    // TODO: unlock DX11 data buffer
}

void xGSDataBufferImpl::ReleaseRendererResources()
{
    // TODO: release DX11 data buffer
    ::Release(p_buffer);
}

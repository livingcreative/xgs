/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx12/xGSinput.cpp
        Input object implementation class
*/

#include "xGSinput.h"


using namespace xGS;


xGSInputImpl::xGSInputImpl(xGSImpl *owner) :
    xGSObjectBase(owner)
{}

xGSInputImpl::~xGSInputImpl()
{}

GSbool xGSInputImpl::AllocateImpl(GSuint elementbuffers)
{
    // TODO: xGSInputImpl::allocate

    return GS_TRUE;
}

void xGSInputImpl::apply(const GScaps &caps)
{
    // TODO: xGSInputImpl::apply
}

void xGSInputImpl::ReleaseRendererResources()
{
    // TODO: xGSInputImpl::ReleaseRendererResources

    if (p_state) {
        p_state->Release();
    }

    for (auto b : p_buffers) {
        b->Release();
    }
}

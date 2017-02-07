/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/xgs

    dx12/xGSinput.cpp
        Input object implementation class
*/

#include "xGSinput.h"
#include "xGSstate.h"
#include "xGSgeometry.h"
#include "xGSgeometrybuffer.h"


using namespace xGS;


xGSInputImpl::xGSInputImpl(xGSImpl *owner) :
    xGSObjectImpl(owner),
    p_state(nullptr),
    p_primarybuffer(nullptr),
    p_buffers()
{
    p_owner->debug(DebugMessageLevel::Information, "Input object created\n");
}

xGSInputImpl::~xGSInputImpl()
{
    ReleaseRendererResources();
    p_owner->debug(DebugMessageLevel::Information, "Input object destroyed\n");
}

GSbool xGSInputImpl::allocate(const GSinputdescription &desc)
{
    if (!desc.bindings) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    xGSStateImpl *state = static_cast<xGSStateImpl*>(desc.state);

    if (!state) {
        return p_owner->error(GSE_INVALIDOBJECT);
    }

    GSuint elementbuffers = 0;
    p_buffers.resize(state->inputCount());
    for (auto &b : p_buffers) {
        b = nullptr;
    }

    const GSinputbinding *binding = desc.bindings;
    while (binding->slot != GSIS_END) {
        if (GSuint(binding->slot - GSIS_0) >= state->inputCount()) {
            // invalid slot index
            return p_owner->error(GSE_INVALIDVALUE);
        }

        if (state->input(binding->slot - GSIS_0).buffer) {
            // slot already bound to static buffer
            return p_owner->error(GSE_INVALIDVALUE);
        }

        xGSGeometryBufferImpl *buffer = static_cast<xGSGeometryBufferImpl*>(binding->buffer);

        // TODO: check layout match

        p_buffers[binding->slot - GSIS_0] = buffer;
        ++binding;
    }

    if (p_buffers.size() != state->inputAvailable() || elementbuffers > 1) {
        // TODO: reconsider error code
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    p_state = state;
    p_state->AddRef();

    p_primarybuffer = p_buffers[p_state->inputPrimarySlot()];

    // TODO: xGSInputImpl::allocate

    return p_owner->error(GS_OK);
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

/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    dx11/xGSparameters.cpp
        Parameters object implementation class
*/

#include "xGSparameters.h"
#include "xGSstate.h"
#include "xGSgeometrybuffer.h"
#include "xGSdatabuffer.h"
#include "xGStexture.h"


using namespace xGS;
using namespace std;


xGSParametersImpl::xGSParametersImpl(xGSImpl *owner) :
    xGSObjectImpl(owner),
    p_state(nullptr),
    p_setindex(GS_UNDEFINED)
{
    p_owner->debug(DebugMessageLevel::Information, "Parameters object created\n");
}

xGSParametersImpl::~xGSParametersImpl()
{
    ReleaseRendererResources();
    p_owner->debug(DebugMessageLevel::Information, "Parameters object destroyed\n");
}

GSbool xGSParametersImpl::allocate(const GSparametersdescription &desc)
{
    xGSStateImpl *state = static_cast<xGSStateImpl*>(desc.state);
    if (!state) {
        return p_owner->error(GSE_INVALIDOBJECT);
    }

    GSuint setindex = desc.set - GSPS_0;
    if (setindex >= state->parameterSetCount()) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    const GSParameterSet &set = state->parameterSet(setindex);
    if (set.settype != GSP_DYNAMIC) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    GSerror result = GSParametersState::allocate(
        p_owner, state, set,
        desc.uniforms, desc.textures, desc.constants
    );
    if (result == GS_OK) {
        p_state = state;
        p_state->AddRef();

        p_setindex = setindex;
    }

    return p_owner->error(result);
}

void xGSParametersImpl::apply(const GScaps &caps)
{
    GSParametersState::apply(caps, p_owner, p_state);
}

void xGSParametersImpl::ReleaseRendererResources()
{
    if (p_state) {
        p_state->Release();
        p_state = nullptr;
    }

    GSParametersState::ReleaseRendererResources(p_owner);
}

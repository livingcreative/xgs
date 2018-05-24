/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx11/xGSparameters.cpp
        Parameters object implementation class
*/

#include "xGSparameters.h"
#include "xGSstate.h"


using namespace xGS;


xGSParametersImpl::xGSParametersImpl(xGSImpl *owner) :
    xGSObjectBase(owner)
{}

xGSParametersImpl::~xGSParametersImpl()
{}

GSbool xGSParametersImpl::AllocateImpl(const GSparametersdescription &desc, const GSParameterSet &set)
{
    xGSStateImpl *state = static_cast<xGSStateImpl*>(desc.state);
    GSerror result = GSParametersState::allocate(
        p_owner, state, set,
        desc.uniforms, desc.textures, desc.constants
    );
    return result == GS_OK;
}

void xGSParametersImpl::apply(const GScaps &caps)
{
    GSParametersState::apply(caps, p_owner, p_state);
}

void xGSParametersImpl::ReleaseRendererResources()
{
    GSParametersState::ReleaseRendererResources(p_owner);
}

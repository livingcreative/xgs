/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/xgs

    dx12/xGSparameters.h
        Parameters object implementation class header
            this object stores parameter values for state object
*/

#pragma once

#include "xGSobject.h"
#include "xGSutil.h"
#include "xGSDX12util.h"
#include <vector>


namespace xGS
{

    class xGSStateImpl;
    class xGSGeometryBufferImpl;
    class xGSTextureImpl;


    // parameters object
    class xGSParametersImpl :
        public xGSObjectImpl<xGSObjectBase<xGSParameters>, xGSParametersImpl>,
        public GSParametersState
    {
    public:
        xGSParametersImpl(xGSImpl *owner);
        ~xGSParametersImpl() override;

    public:
        GSbool allocate(const GSparametersdescription &desc);

        void apply(const GScaps &caps);

        xGSStateImpl* state() const { return p_state; }
        GSuint setindex() const { return p_setindex; }

        void ReleaseRendererResources();

    private:
        xGSStateImpl *p_state;
        GSuint        p_setindex;
    };

} // namespace xGS

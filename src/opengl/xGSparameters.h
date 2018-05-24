/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/xGSparameters.h
        Parameters object implementation class header
            this object stores parameter values for state object
*/

#pragma once

#include "xGSimplbase.h"
#include "xGSutil.h"
#include "xGSGLutil.h"


namespace xGS
{

    // parameters object
    class xGSParametersImpl :
        public xGSObjectBase<xGSParametersBase, xGSImpl>,
        public GSParametersState
    {
    public:
        xGSParametersImpl(xGSImpl *owner);
        ~xGSParametersImpl() override;

    public:
        GSbool AllocateImpl(const GSparametersdescription &desc, const GSParameterSet &set);

        void apply(const GScaps &caps);

        void ReleaseRendererResources();
    };

} // namespace xGS

/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    xGSparameters.h
        Parameters object implementation class header
            this object stores parameter values for state object
*/

#pragma once

#include "xGSobject.h"
#include "xGSutil.h"
#include "glplatform.h"
#include <vector>


namespace xGS
{

    class xGSStateImpl;
    class xGSGeometryBufferImpl;
    class xGSTextureImpl;


    // parameters object
    class xGSParametersImpl :
        public xGSObject<xGSParameters, xGSParametersImpl>,
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
        GSbool allocated() const { return p_allocated; }

        void ReleaseRendererResources();

    private:
        GSbool        p_allocated;
        xGSStateImpl *p_state;
        GSuint        p_setindex;
    };

} // namespace xGS

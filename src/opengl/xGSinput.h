/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/xGSinput.h
        Input object implementation class header
            this object binds source of geometry data with state object
*/

#pragma once

#include "xGSimplbase.h"


namespace xGS
{

    // input object
    class xGSInputImpl : public xGSObjectBase<xGSInputBase, xGSImpl>
    {
    public:
        xGSInputImpl(xGSImpl *owner);
        ~xGSInputImpl() override;

    public:
        GSbool AllocateImpl(GSuint elementbuffers);

        void apply(const GScaps &caps);

        void ReleaseRendererResources();

    private:
        typedef std::vector<GLuint> GLuintList;

    private:
        GLuintList p_vertexbuffers;
        GLuintList p_strides;
        GLuintList p_slots;
        GLuint     p_indexbuffer;
        GLuint     p_vertexarray;
    };

} // namespace xGS

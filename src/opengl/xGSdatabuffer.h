/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/xGSdatabuffer.h
        DataBuffer object implementation class header
            this object wraps buffer for holding uniform data which is used
            by shaders in state object
*/

#pragma once

#include "xGSimplbase.h"


namespace xGS
{

    // data buffer object
    class xGSDataBufferImpl : public xGSObjectBase<xGSDataBufferBase, xGSImpl>
    {
    public:
        xGSDataBufferImpl(xGSImpl *owner);
        ~xGSDataBufferImpl() override;

    public:
        GLuint getID() const { return p_buffer; }
        GLenum target() const { return p_target; }

        GSbool AllocateImpl(const GSdatabufferdescription &desc, GSuint totalsize);

        void UpdateImpl(GSuint offset, GSuint size, const GSptr data);
        GSptr LockImpl(GSdword access);
        void UnlockImpl();

        void ReleaseRendererResources();

    private:
        GLuint p_buffer;
        GLenum p_target;
    };

} // namespace xGS

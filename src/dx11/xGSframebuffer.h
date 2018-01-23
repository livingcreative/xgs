/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx11/xGSframebuffer.h
        FrameBuffer object implementation class header
            this object is used for sotring render target frame buffer configuration
            and attached textures
*/

#pragma once

#include "xGSobject.h"
#include "xGSframebufferbase.h"


namespace xGS
{

    // framebuffer object
    class xGSFrameBufferImpl : public xGSObjectBase<xGSFrameBufferBase>
    {
    public:
        xGSFrameBufferImpl(xGSImpl *owner);
        ~xGSFrameBufferImpl() override;

    public:
        void AllocateImpl(int multisampled_attachments);

        void bind();
        void unbind();

        void ReleaseRendererResources();

    private:
        void attachTexture(const Attachment &texture, int attachment);

    private:
        // TODO: dx11 frame buffer internal objects
    };

} // namespace xGS

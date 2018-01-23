/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    IxGSframebufferimpl.h
        FrameBuffer object public interface implementation
*/

#pragma once

#include "xGSframebuffer.h"


namespace xGS
{

	// framebuffer object
    class IxGSFrameBufferImpl : public xGSObjectImpl<xGSFrameBufferImpl, IxGSFrameBufferImpl>
    {
    public:
        IxGSFrameBufferImpl(xGSImpl *owner);
        ~IxGSFrameBufferImpl() override;

    public:
        GSbool allocate(const GSframebufferdescription &desc);

    public:
        GSvalue xGSAPI GetValue(GSenum valuetype) override;

    private:
        void checkAttachment(GSenum format, const Attachment &att, bool &incomplete, int &attachments, int &multiampled_attachments, int &srgb_attachments);
    };

} // namespace xGS

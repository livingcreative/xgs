/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/xGSframebuffer.h
        FrameBuffer object implementation class header
            this object is used for sotring render target frame buffer configuration
            and attached textures
*/

#pragma once

#include "xGSobject.h"


namespace xGS
{

    class xGSTextureImpl;


    // framebuffer object
    class xGSFrameBufferImpl : public xGSObjectImpl<xGSObjectBase<xGSFrameBuffer>, xGSFrameBufferImpl>
    {
    public:
        xGSFrameBufferImpl(xGSImpl *owner);
        ~xGSFrameBufferImpl() override;

    public:
        GSvalue xGSAPI GetValue(GSenum valuetype) override;

    public:
        GSbool allocate(const GSframebufferdescription &desc);

        GSsize size() const;
        void bind();
        void unbind();

        void getformats(GSenum *colorformats, GSenum &depthstencilformat) const;
        bool srgb() const { return p_srgb; }

        void ReleaseRendererResources();

    private:
        struct Attachment
        {
            Attachment() :
                p_texture(nullptr),
                p_level(0),
                p_slice(0)
            {}

            ~Attachment();

            void attach(xGSTextureImpl *texture, GSuint level, GSuint slice);

            xGSTextureImpl *p_texture;
            GSuint          p_level;
            GSuint          p_slice;
        };

        void checkAttachment(GSenum format, const Attachment &att, bool &incomplete, int &attachments, int &multiampled_attachments, int &srgb_attachments);
        void createRenderBuffer(GLuint &buffer, GLenum format);
        void checkFrameBufferStatus();
        void attachTexture(const Attachment &texture, GLenum attachment);

    private:
        GSuint     p_width;
        GSuint     p_height;
        GSuint     p_colortargets;
        GSuint     p_activecolortargets;
        GSenum     p_colorformat[GS_MAX_FB_COLORTARGETS];
        GSenum     p_depthstencilformat;
        GSenum     p_multisample;
        bool       p_srgb;

        GLuint     p_framebuffer;
        GLuint     p_resolvebuffer;
        GLuint     p_colorrb[GS_MAX_FB_COLORTARGETS];
        GLuint     p_depthrb;

        Attachment p_colortextures[GS_MAX_FB_COLORTARGETS];
        Attachment p_depthtexture;
    };

} // namespace xGS

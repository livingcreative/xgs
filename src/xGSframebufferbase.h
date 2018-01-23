/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    xGSframebufferbase.h
        FrameBuffer object implementation class header
*/

#pragma once


namespace xGS
{

    class xGSTextureImpl;


    // framebuffer object
    class xGSFrameBufferBase : public xGSFrameBuffer
    {
    public:
        xGSFrameBufferBase();

    public:
        GSsize size() const;

        void getformats(GSenum *colorformats, GSenum &depthstencilformat) const;
        bool srgb() const { return p_srgb; }

    protected:
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

    protected:
        GSuint     p_width;
        GSuint     p_height;
        GSuint     p_colortargets;
        GSuint     p_activecolortargets;
        GSenum     p_colorformat[GS_MAX_FB_COLORTARGETS];
        GSenum     p_depthstencilformat;
        GSenum     p_multisample;
        bool       p_srgb;

        Attachment p_colortextures[GS_MAX_FB_COLORTARGETS];
        Attachment p_depthtexture;
    };

} // namespace xGS

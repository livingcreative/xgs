/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/xGStexture.h
        Texture object implementation class header
            this object stores texture data and its format description
*/

#pragma once

#include "xGSimplbase.h"


namespace xGS
{

    // texture object
    class xGSTextureImpl : public xGSObjectBase<xGSTextureBase, xGSImpl>
    {
    public:
        xGSTextureImpl(xGSImpl *owner);
        ~xGSTextureImpl() override;

    public:
        GSbool AllocateImpl();

        GLuint getID() const { return p_texture; }
        GLuint getBufferID() const { return p_buffer; }
        GLenum target() const { return p_target; }

        GSptr LockImpl(GSenum locktype, GSdword access, GSint level, GSint layer, void *lockdata);
        void UnlockImpl();

        static void bindNullTexture();

        void ReleaseRendererResources();

    private:
        void SetImage(GLenum gltarget, bool mipcascade);
        void SetImage1D(bool mipcascade);
        void SetImage2D(GLenum gltarget, bool mipcascade);
        void SetImage2DMultisample(bool mipcascade);
        void SetImage3D(GLenum gltarget, bool mipcascade);

        void UpdateImage(GLenum gltarget, int level, GSptr data);

    private:
        GSint   p_bpp;          // format: BYTES per pixel
        GLenum  p_target;       // format: GL texture target
        GLenum  p_GLIntFormat;  // format: GL internal format
        GLenum  p_GLFormat;     // format: GL format for TexImage/TexSubImage
        GLenum  p_GLType;       // format: GL type for TexImage/TexSubImage

        GLuint  p_texture;      // GL texture ID
        GLuint  p_buffer;       // GL buffer ID for texture

        GLuint  p_lockbuffer;   // LOCK: GL pixel buffer ID
    };

} // namespace xGS

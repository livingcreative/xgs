/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx11/xGStexture.h
        Texture object implementation class header
            this object stores texture data and its format description
*/

#pragma once

#include "xGSimplbase.h"


struct ID3D11Resource;
struct ID3D11ShaderResourceView;


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

        // TODO: this is temp. hack, see IxGSFrameBufferImpl allocate() note
        int getID() const { return int(p_texture != nullptr); }

        ID3D11Resource* texture() const { return p_texture; }
        ID3D11ShaderResourceView* view() const { return p_view; }

        GSptr LockImpl(GSenum locktype, GSdword access, GSint level, GSint layer, void *lockdata);
        void UnlockImpl();

        static void bindNullTexture();

        void ReleaseRendererResources();

    private:
        void SetImage(bool mipcascade);
        void SetImage1D(bool mipcascade);
        void SetImage2D(bool mipcascade);
        void SetImage2DMultisample(bool mipcascade);
        void SetImage3D(bool mipcascade);

        void UpdateImage(int level, GSptr data);

    private:
        GSint                     p_bpp;          // format: BYTES per pixel

        ID3D11Resource           *p_texture;
        ID3D11ShaderResourceView *p_view;

        char                     *p_lockmemory;
    };

} // namespace xGS

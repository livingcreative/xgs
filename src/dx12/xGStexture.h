/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx12/xGStexture.h
        Texture object implementation class header
            this object stores texture data and its format description
*/

#pragma once

#include "xGS/xGS.h"
#include "xGSobject.h"


namespace xGS
{

    // texture object
    class xGSTextureImpl : public xGSObjectImpl<xGSObjectBase<xGSTexture>, xGSTextureImpl>
    {
    public:
        xGSTextureImpl(xGSImpl *owner);
        ~xGSTextureImpl() override;

    public:
        GSvalue xGSAPI GetValue(GSenum valuetype) override;

        GSptr   xGSAPI Lock(GSenum locktype, GSdword access, GSint level, GSint layer, void *lockdata) override;
        GSbool  xGSAPI Unlock() override;

    public:
        GSbool allocate(const GStexturedescription &desc);

        ID3D12Resource* texture() const { return p_texture; }

        DXGI_FORMAT texformat() const { return p_texformat; }

        GSuint samples() const { return p_multisample; }
        GSenum format() const { return p_format; }
#ifdef _DEBUG
        bool boundAsRT() const { return p_boundasrt != 0; }
        void bindAsRT() { ++p_boundasrt; }
        void unbindFromRT() { --p_boundasrt; }
#endif
        static void bindNullTexture();

        void ReleaseRendererResources();

    private:
        void DoUnlock();

        void SetImage(bool mipcascade);
        void SetImage1D(bool mipcascade);
        void SetImage2D(bool mipcascade);
        void SetImage2DMultisample(bool mipcascade);
        void SetImage3D(bool mipcascade);

        void UpdateImage(int level, GSptr data);

    private:
        GSenum          p_texturetype;  // texture type: 1D, 2D, etc...
        GSenum          p_format;       // texture texel format

        DXGI_FORMAT     p_texformat;
        GSint           p_bpp;          // format: BYTES per pixel

        ID3D12Resource *p_texture;

        GSuint          p_width;        // width
        GSuint          p_height;       // height
        GSuint          p_depth;        // depth (3D texture only)
        GSuint          p_layers;       // array layers
        GSuint          p_multisample;  // multisample sample count
        GSuint          p_minlevel;     // minimum (base) defined MIP level
        GSuint          p_maxlevel;     // maximum defined MIP level

        GSenum          p_locktype;     // LOCK: locked texture part
        GSdword         p_lockaccess;   // LOCK: lock access
        GSint           p_locklayer;    // LOCK: locked texture layer (for array textures only)
        GSint           p_locklevel;    // LOCK: locked texture MIP level
        ID3D12Resource *p_locktexture;

#ifdef _DEBUG
        GSuint          p_boundasrt;    // texture currently is render target
#endif
    };

} // namespace xGS

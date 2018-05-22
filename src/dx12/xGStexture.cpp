/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx12/xGStexture.cpp
        Texture object implementation class
*/

#include "xGStexture.h"
#include "kcommon/c_util.h"


using namespace xGS;
using namespace c_util;


xGSTextureImpl::xGSTextureImpl(xGSImpl *owner) :
    xGSObjectBase(owner),
    p_texture(nullptr),
    p_locktexture(nullptr)
{}

xGSTextureImpl::~xGSTextureImpl()
{}

GSbool xGSTextureImpl::AllocateImpl()
{
    // TODO: move this to common code and texture formats
    xGSImpl::TextureFormatDescriptor texdesc;
    if (!p_owner->GetTextureFormatDescriptor(p_format, texdesc)) {
        p_owner->debug(DebugMessageLevel::Error, "Invalid texture format: %i\n", p_format);
        return p_owner->error(GSE_INVALIDENUM);
    }

    p_texformat = texdesc.format;
    p_bpp = texdesc.bpp;

    UINT bindflags =
        0;// D3D12_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

    //D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc = {};
    //viewdesc.Format = texdesc.format;

    // TODO: xGSTextureImpl::allocate

    D3D12_HEAP_PROPERTIES heapprops = {
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1, 1
    };

    D3D12_RESOURCE_DESC dxtexdesc = { };
    dxtexdesc.Height = 1;
    dxtexdesc.DepthOrArraySize = p_texturetype == GS_TEXTYPE_3D ?
        p_depth : umax(1u, p_layers);
    dxtexdesc.MipLevels = p_maxlevel + 1;
    dxtexdesc.Format = texdesc.format;
    dxtexdesc.SampleDesc.Count = 1; // TODO: ms textures

    switch (p_texturetype) {
        case GS_TEXTYPE_1D: {
            dxtexdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
            dxtexdesc.Width = p_width;

            p_owner->device()->CreateCommittedResource(
                &heapprops, D3D12_HEAP_FLAG_NONE, &dxtexdesc,
                D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&p_texture)
            );

            break;
        }

        case GS_TEXTYPE_2D: {
            //ID3D11Texture2D *tex = nullptr;

            // TODO: make this common!
            int w = p_width;
            int h = p_height;
            GSuint levels = 0;
            while (w > 0 || h > 0) {
                ++levels;
                w = w >> 1;
                h = h >> 1;
            }

            dxtexdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            dxtexdesc.Width = p_width;
            dxtexdesc.Height = p_height;
            dxtexdesc.MipLevels = umin(levels, p_maxlevel);

            p_owner->device()->CreateCommittedResource(
                &heapprops, D3D12_HEAP_FLAG_NONE, &dxtexdesc,
                D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&p_texture)
            );

            break;
        }

        case GS_TEXTYPE_3D: {
            // TODO
            break;
        }

        case GS_TEXTYPE_CUBEMAP: {
            // TODO
            break;
        }

        case GS_TEXTYPE_RECT: {
            // TODO
            break;
        }

        case GS_TEXTYPE_BUFFER: {
            // TODO
            break;
        }
    }

    return p_owner->error(GS_OK);
}

GSptr xGSTextureImpl::LockImpl(GSenum locktype, GSdword access, GSint level, GSint layer, void *lockdata)
{
    // TODO: xGSTextureImpl::Lock

    D3D12_HEAP_PROPERTIES heapprops = {
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1, 1
    };

    // this is also pasta, move to util
    size_t size = 0;
    switch (p_texturetype) {
        case GS_TEXTYPE_1D:      size = p_width >> level; break;
        case GS_TEXTYPE_2D:      size = umax(p_width >> level, 1u) * umax(p_height >> level, 1u); break;
        case GS_TEXTYPE_3D:      size = umax(p_width >> level, 1u) * umax(p_height >> level, 1u) * umax(p_depth >> level, 1u); break;
        case GS_TEXTYPE_CUBEMAP: size = umax(p_width >> level, 1u) * umax(p_height >> level, 1u); break;
    }
    size *= p_bpp;

    D3D12_RESOURCE_DESC locktexturedesc = {
        D3D12_RESOURCE_DIMENSION_BUFFER,
        0, size, 1, 1, 1
    };
    locktexturedesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    locktexturedesc.SampleDesc.Count = 1;

    p_owner->device()->CreateCommittedResource(
        &heapprops, D3D12_HEAP_FLAG_NONE, &locktexturedesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr, IID_PPV_ARGS(&p_locktexture)
    );

    void *mem = nullptr;
    p_locktexture->Map(level, nullptr, &mem);

    p_locktype = locktype;
    p_lockaccess = access;
    p_locklayer = layer;
    p_locklevel = level;

    p_owner->error(GS_OK);

    return mem;
}

void xGSTextureImpl::UnlockImpl()
{
    // TODO: xGSTextureImpl::DoUnlock
    p_locktexture->Unmap(p_locklevel, nullptr);

    D3D12_SUBRESOURCE_FOOTPRINT footprint = {
        p_texformat,
        umax(p_width >> p_locklevel, 1u),
        umax(p_height >> p_locklevel, 1u),
        umax(p_depth >> p_locklevel, 1u),
        umax(p_width >> p_locklevel, 1u) * p_bpp
    };
    p_owner->UploadTextureData(p_locktexture, p_texture, p_locklevel, footprint);

    ::Release(p_locktexture);

    p_lockaccess = 0;
    p_locklayer = 0;
    p_locktype = GS_NONE;
}

void xGSTextureImpl::bindNullTexture()
{
    // TODO: xGSTextureImpl::bindNullTexture
}

void xGSTextureImpl::ReleaseRendererResources()
{
    if (p_locktype) {
        UnlockImpl();
    }

    ::Release(p_texture);
}

void xGSTextureImpl::SetImage(bool mipcascade)
{
    // TODO: xGSTextureImpl::SetImage
}

void xGSTextureImpl::SetImage1D(bool mipcascade)
{
    // TODO: xGSTextureImpl::SetImage1D
}

void xGSTextureImpl::SetImage2D(bool mipcascade)
{
    // TODO: xGSTextureImpl::SetImage2D
}

void xGSTextureImpl::SetImage2DMultisample(bool mipcascade)
{
    // TODO: xGSTextureImpl::SetImage2DMultisample
}

void xGSTextureImpl::SetImage3D(bool mipcascade)
{
    // TODO: xGSTextureImpl::SetImage3D
}

void xGSTextureImpl::UpdateImage(int level, GSptr data)
{
    // TODO: xGSTextureImpl::UpdateImage
}

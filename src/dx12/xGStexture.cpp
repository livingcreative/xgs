/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/xgs

    dx12/xGStexture.cpp
        Texture object implementation class
*/

#include "xGStexture.h"
#include "kcommon/c_util.h"


using namespace xGS;
using namespace c_util;


xGSTextureImpl::xGSTextureImpl(xGSImpl *owner) :
    xGSObjectImpl(owner),
    p_texturetype(GS_TEXTYPE_EMPTY),
    p_texture(nullptr),
    p_width(0),
    p_height(0),
    p_depth(0),
    p_layers(0),
    p_multisample(GS_MULTISAMPLE_NONE),
    p_minlevel(0),
    p_maxlevel(1000),
    p_locktype(GS_NONE)
{
#ifdef _DEBUG
    p_boundasrt = 0;
#endif
    p_owner->debug(DebugMessageLevel::Information, "Texture object created\n");
}

xGSTextureImpl::~xGSTextureImpl()
{
    ReleaseRendererResources();
    p_owner->debug(DebugMessageLevel::Information, "Texture object destroyed\n");
}

GSvalue xGSTextureImpl::GetValue(GSenum valuetype)
{
    switch (valuetype) {
        case GS_TEX_TYPE:        return p_texturetype;
        case GS_TEX_FORMAT:      return p_format;
        case GS_TEX_WIDTH:       return p_width;
        case GS_TEX_HEIGHT:      return p_height;
        case GS_TEX_DEPTH:       return p_depth;
        case GS_TEX_LAYERS:      return p_layers;
        case GS_TEX_MIN_LEVEL:   return p_minlevel;
        case GS_TEX_MAX_LEVEL:   return p_maxlevel;
        case GS_TEX_MULTISAMPLE: return p_multisample;

        default:
            p_owner->error(GSE_INVALIDENUM);
            return 0;
    }
}

GSbool xGSTextureImpl::allocate(const GStexturedescription &desc)
{
    // TODO: check params
    p_texturetype = desc.type;
    p_format = desc.format;
    p_width = desc.width;
    p_height = desc.height;
    p_depth = desc.depth;
    p_layers = desc.layers;
    p_multisample = desc.multisample;
    if (p_texturetype == GS_TEXTYPE_RECT) {
        p_minlevel = 0;
        p_maxlevel = 0;
    } else {
        p_minlevel = desc.minlevel;
        p_maxlevel = desc.maxlevel;
    }

    xGSImpl::TextureFormatDescriptor texdesc;
    if (!p_owner->GetTextureFormatDescriptor(p_format, texdesc)) {
        p_owner->debug(DebugMessageLevel::Error, "Invalid texture format: %i\n", p_format);
        return p_owner->error(GSE_INVALIDENUM);
    }

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
        desc.depth : umax(1u, p_layers);
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
            dxtexdesc.MipLevels = umin(levels, desc.maxlevel);

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

GSptr xGSTextureImpl::Lock(GSenum locktype, GSdword access, GSint level, GSint layer, void *lockdata)
{
    if (p_locktype) {
        p_owner->error(GSE_INVALIDOPERATION);
        return nullptr;
    }

    // TODO: xGSTextureImpl::Lock
    p_lockmemory = new char[p_width * p_height * p_bpp];

    p_locktype = locktype;
    p_lockaccess = access;
    p_locklayer = layer;
    p_locklevel = level;

    p_owner->error(GS_OK);

    return p_lockmemory;
}

GSbool xGSTextureImpl::Unlock()
{
    if (!p_locktype) {
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    DoUnlock();

    return p_owner->error(GS_OK);
}

void xGSTextureImpl::bindNullTexture()
{
    // TODO: xGSTextureImpl::bindNullTexture
}

void xGSTextureImpl::ReleaseRendererResources()
{
    if (p_locktype) {
        DoUnlock();
    }

    ::Release(p_texture);
}

void xGSTextureImpl::DoUnlock()
{
    // TODO: xGSTextureImpl::DoUnlock
    //p_owner->context()->UpdateSubresource(p_texture, 0, nullptr, p_lockmemory, p_width * p_bpp, 0);

    delete[] p_lockmemory;

    p_lockaccess = 0;
    p_locklayer = 0;
    p_locktype = GS_NONE;
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

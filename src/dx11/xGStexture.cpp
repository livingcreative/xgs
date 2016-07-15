/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    dx11/xGStexture.cpp
        Texture object implementation class
*/

#include "xGStexture.h"
#include "kcommon/c_util.h"


using namespace xGS;
using namespace c_util;


xGSTextureImpl::xGSTextureImpl(xGSImpl *owner) :
    xGSObjectImpl(owner),
    p_texturetype(GS_TEXTYPE_EMPTY),
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
    // TODO: xGSTextureImpl::allocate

    return p_owner->error(GS_OK);
}

GSptr xGSTextureImpl::Lock(GSenum locktype, GSdword access, GSint level, GSint layer, void *lockdata)
{
    if (p_locktype) {
        p_owner->error(GSE_INVALIDOPERATION);
        return nullptr;
    }

    // TODO: xGSTextureImpl::Lock

    p_locktype = locktype;
    p_lockaccess = access;
    p_locklayer = layer;
    p_locklevel = level;

    p_owner->error(GS_OK);

    return nullptr;
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

    // TODO: xGSTextureImpl::ReleaseRendererResources
}

void xGSTextureImpl::DoUnlock()
{
    // TODO: xGSTextureImpl::DoUnlock

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

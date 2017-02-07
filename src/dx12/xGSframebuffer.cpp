/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/xgs

    dx12/xGSframebuffer.cpp
        FrameBuffer object implementation class
*/

#include "xGSframebuffer.h"
#include "xGStexture.h"
#include "kcommon/c_util.h"


using namespace xGS;
using namespace c_util;


xGSFrameBufferImpl::xGSFrameBufferImpl(xGSImpl *owner) :
    xGSObjectImpl(owner),
    p_width(0),
    p_height(0),
    p_colortargets(1),
    p_srgb(false),
    p_activecolortargets(0),
    p_depthstencilformat(GS_DEPTH_NONE),
    p_multisample(GS_MULTISAMPLE_NONE)
{
    for (int n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        p_colorformat[n] = GS_COLOR_DEFAULT;
    }

    p_owner->debug(DebugMessageLevel::Information, "FrameBuffer object created\n");
}

xGSFrameBufferImpl::~xGSFrameBufferImpl()
{
    ReleaseRendererResources();
    p_owner->debug(DebugMessageLevel::Information, "FrameBuffer object destroyed\n");
}

GSvalue xGSFrameBufferImpl::GetValue(GSenum valuetype)
{
    switch (valuetype) {
        case GS_FB_TYPE:          return GS_FBTYPE_OFFSCREEN;
        case GS_FB_WIDTH:         return p_width;
        case GS_FB_HEIGHT:        return p_height;
        case GS_FB_DOUBLEBUFFER:  return GS_FALSE;
        case GS_FB_COLORTARGETS:  return p_colortargets;
        case GS_FB_DEPTHFORMAT:   return p_depthstencilformat;
        case GS_FB_STENCILFORMAT: return GS_STENCIL_NONE;
        case GS_FB_MULTISAMPLE:   return p_multisample;

        case GS_FB_COLORFORMAT0:
        case GS_FB_COLORFORMAT1:
        case GS_FB_COLORFORMAT2:
        case GS_FB_COLORFORMAT3:
        case GS_FB_COLORFORMAT4:
        case GS_FB_COLORFORMAT5:
        case GS_FB_COLORFORMAT6:
        case GS_FB_COLORFORMAT7:
            return p_colorformat[valuetype - GS_FB_COLORFORMAT0];
    }

    p_owner->error(GSE_INVALIDENUM);
    return 0;
}

GSbool xGSFrameBufferImpl::allocate(const GSframebufferdescription &desc)
{
    // set description
    p_width = desc.width;
    p_height = desc.height;
    p_colortargets = umin(desc.colortargets, GSuint(GS_MAX_FB_COLORTARGETS));
    for (size_t n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        p_colorformat[n] = desc.colorformat[n];
    }
    p_depthstencilformat = desc.depthstencilformat;
    p_multisample = desc.multisample;

    if (p_width == 0 || p_height == 0) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    // initialize attachments
    if (desc.attachments) {
        const GSframebufferattachment *attachment = desc.attachments;
        while (attachment->attachment != GSA_END) {
            switch (attachment->attachment) {
                case GSA_COLOR0:
                case GSA_COLOR1:
                case GSA_COLOR2:
                case GSA_COLOR3:
                case GSA_COLOR4:
                case GSA_COLOR5:
                case GSA_COLOR6:
                case GSA_COLOR7:
                    {
                        GSuint n = attachment->attachment - GSA_COLOR0;
                        GSenum fmt = p_colorformat[n];
                        if (fmt != GS_COLOR_DEFAULT) {
                            return p_owner->error(GSE_INVALIDVALUE);
                        }

                        p_colortextures[n].attach(
                            static_cast<xGSTextureImpl*>(attachment->texture),
                            attachment->level, attachment->slice
                        );
                    }
                    break;

                case GSA_DEPTH:
                    if (p_depthstencilformat != GS_DEPTH_DEFAULT) {
                        return p_owner->error(GSE_INVALIDVALUE);
                    }

                    p_depthtexture.attach(
                        static_cast<xGSTextureImpl*>(attachment->texture),
                        attachment->level, attachment->slice
                    );

                    break;

                case GSA_STENCIL:

                default:
                    return p_owner->error(GSE_INVALIDENUM);
            }

            ++attachment;
        }
    }

    // check for multisample textures
    // texture samples value should match frame buffer's samples value, or should be 0
    bool incomplete = false;
    int attachmentcount = 0;
    int multisampled_attachments = 0;
    int srgb_attachments = 0;

    for (GSuint n = 0; n < p_colortargets; ++n) {
        checkAttachment(
            p_colorformat[n], p_colortextures[n], incomplete,
            attachmentcount, multisampled_attachments, srgb_attachments
        );
    }
    checkAttachment(
        p_depthstencilformat, p_depthtexture, incomplete,
        attachmentcount, multisampled_attachments, srgb_attachments
    );

    if (incomplete) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    if (srgb_attachments != 0 && srgb_attachments != p_colortargets) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    if (multisampled_attachments != 0 && multisampled_attachments != attachmentcount) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    p_srgb = srgb_attachments > 0;

    // TODO: xGSFrameBufferImpl::allocate

    for (GSuint n = 0; n < p_colortargets; ++n) {
        if (p_colorformat[n] == GS_COLOR_DEFAULT) {
            attachTexture(p_colortextures[n], 0);
        }
    }

    if (p_depthstencilformat == GS_DEPTH_DEFAULT) {
        attachTexture(p_depthtexture, 0);
    }

    return p_owner->error(GS_OK);
}


GSsize xGSFrameBufferImpl::size() const
{
    GSsize result;
    result.width = p_width;
    result.height = p_height;
    return result;
}

void xGSFrameBufferImpl::bind()
{
    // TODO: xGSFrameBufferImpl::bind
}

void xGSFrameBufferImpl::unbind()
{
    // TODO: xGSFrameBufferImpl::unbind
}

static inline GSenum RTFormat(GSenum format, xGSTextureImpl *attachment)
{
    if (format != GS_DEFAULT) {
        return format;
    } else {
        return attachment ? attachment->format() : GS_NONE;
    }
}

void xGSFrameBufferImpl::getformats(GSenum *colorformats, GSenum &depthstencilformat) const
{
    for (size_t n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        colorformats[n] = RTFormat(p_colorformat[n], p_colortextures[n].p_texture);
    }
    depthstencilformat = RTFormat(p_depthstencilformat, p_depthtexture.p_texture);
}

void xGSFrameBufferImpl::ReleaseRendererResources()
{
    // TODO: xGSFrameBufferImpl::ReleaseRendererResources
}


xGSFrameBufferImpl::Attachment::~Attachment()
{
    if (p_texture) {
        p_texture->Release();
    }
}

void xGSFrameBufferImpl::Attachment::attach(xGSTextureImpl *texture, GSuint level, GSuint slice)
{
    if (p_texture) {
        p_texture->Release();
    }

    p_texture = texture;
    p_level = level;
    p_slice = slice;

    if (p_texture) {
        p_texture->AddRef();
    }
}


void xGSFrameBufferImpl::checkAttachment(GSenum format, const Attachment &att, bool &incomplete, int &attachments, int &multiampled_attachments, int &srgb_attachments)
{
    switch (format) {
        case GS_DEFAULT:
            ++attachments;

            if (att.p_texture == nullptr) {
                incomplete = true;
                return;
            }

            if (att.p_texture->samples() != 0 && p_multisample == 0) {
                incomplete = true;
                return;
            }

            if (att.p_texture->samples() != 0 && att.p_texture->samples() != p_multisample) {
                incomplete = true;
                return;
            }

            if (att.p_texture->samples()) {
                ++multiampled_attachments;
            }

            if (att.p_texture->format() == GS_COLOR_S_RGBA || att.p_texture->format() == GS_COLOR_S_RGBX) {
                ++srgb_attachments;
            }

            break;

        case GS_COLOR_S_RGBA:
        case GS_COLOR_S_RGBX:
            ++srgb_attachments;
            break;
    }
}

void xGSFrameBufferImpl::attachTexture(const Attachment &texture, int attachment)
{
    // TODO: xGSFrameBufferImpl::attachTexture
}

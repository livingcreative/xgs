/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    IxGSframebufferimpl.cpp
        FrameBuffer object public interface implementation
*/

#include "IxGSframebufferimpl.h"
#include "kcommon/c_util.h"


using namespace xGS;
using namespace c_util;


IxGSFrameBufferImpl::IxGSFrameBufferImpl(xGSImpl *owner) :
    xGSObjectImpl(owner)
{
    p_owner->debug(DebugMessageLevel::Information, "FrameBuffer object created\n");
}

IxGSFrameBufferImpl::~IxGSFrameBufferImpl()
{
    ReleaseRendererResources();
    p_owner->debug(DebugMessageLevel::Information, "FrameBuffer object destroyed\n");
}

GSbool IxGSFrameBufferImpl::allocate(const GSframebufferdescription &desc)
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
            // TODO: replace getID() with common resource allocation check function
            if (attachment->texture && static_cast<xGSTextureImpl*>(attachment->texture)->getID() == 0) {
                return p_owner->error(GSE_INVALIDOBJECT);
            }

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

    AllocateImpl(multisampled_attachments);

    return p_owner->error(GS_OK);
}

GSvalue IxGSFrameBufferImpl::GetValue(GSenum valuetype)
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

void IxGSFrameBufferImpl::checkAttachment(GSenum format, const Attachment &att, bool &incomplete, int &attachments, int &multiampled_attachments, int &srgb_attachments)
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

/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    opengl/xGSframebuffer.cpp
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
    p_multisample(GS_MULTISAMPLE_NONE),
    p_framebuffer(0),
    p_resolvebuffer(0)
{
    for (int n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        p_colorformat[n] = GS_COLOR_DEFAULT;
    }

    for (int n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        p_colorrb[n] = 0;
    }

    p_depthrb = 0;

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

    glGenFramebuffers(1, &p_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, p_framebuffer);

    // create color renderbuffers
    for (GSuint n = 0; n < p_colortargets; ++n) {
        if (p_colorformat[n] != GS_COLOR_NONE) {
            ++p_activecolortargets;
        } else {
            continue;
        }

        if ((p_colorformat[n] != GS_COLOR_DEFAULT || (p_multisample && multisampled_attachments == 0)) && p_colorformat[n] != GS_COLOR_NONE) {
            // if format set not to DEFAULT nor NONE or multisampling requested, render buffer needed

            // TODO: format conversion/check (like in texture creation)
            GLenum format = 0;
            switch (p_colorformat[n] == GS_COLOR_DEFAULT ? p_colortextures[n].p_texture->format() : p_colorformat[n]) {
                case GS_COLOR_RGBA: format = GL_RGBA8; break;
                case GS_COLOR_RGBX: format = GL_RGB8; break;
                case GS_COLOR_S_RGBA: format = GL_SRGB8_ALPHA8; break;
                case GS_COLOR_S_RGBX: format = GL_SRGB8; break;
                case GS_COLOR_RGBA_HALFFLOAT: format = GL_RGBA16F; break;
                case GS_COLOR_RGBX_HALFFLOAT: format = GL_RGB16F; break;
                case GS_COLOR_RGBA_FLOAT: format = GL_RGBA32F; break;
                case GS_COLOR_RGBX_FLOAT: format = GL_RGB32F; break;
            }

            createRenderBuffer(p_colorrb[n], format);
            glFramebufferRenderbuffer(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + n,
                GL_RENDERBUFFER, p_colorrb[n]
            );
        }
    }

    // if no color attachments - disable draw/read color buffers
    if (p_activecolortargets == 0) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    } else {
        GLenum buffers[GS_MAX_FB_COLORTARGETS];
        for (GSuint n = 0; n < p_colortargets; ++n) {
            buffers[n] = p_colorformat[n] == GS_NONE ? GL_NONE : GL_COLOR_ATTACHMENT0 + n;
        }
        glDrawBuffers(p_colortargets, buffers);
    }

    // create depth/stencil renderbuffer
    if ((p_depthstencilformat != GS_DEPTH_DEFAULT || (p_multisample && multisampled_attachments == 0)) && p_depthstencilformat != GS_DEPTH_NONE) {
        createRenderBuffer(p_depthrb, GL_DEPTH_COMPONENT24);
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER, p_depthrb
        );
    }

    // resolve buffer
    if (p_multisample && multisampled_attachments == 0) {
        glGenFramebuffers(1, &p_resolvebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, p_resolvebuffer);
    }

    for (GSuint n = 0; n < p_colortargets; ++n) {
        if (p_colorformat[n] == GS_COLOR_DEFAULT) {
            attachTexture(p_colortextures[n], GL_COLOR_ATTACHMENT0 + n);
        }
    }

    if (p_depthstencilformat == GS_DEPTH_DEFAULT) {
        attachTexture(p_depthtexture, GL_DEPTH_ATTACHMENT);
    }

#ifdef _DEBUG
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

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
    glBindFramebuffer(GL_FRAMEBUFFER, p_framebuffer);
    checkFrameBufferStatus();

#ifdef _DEBUG
    for (auto &t : p_colortextures) {
        if (t.p_texture) {
            t.p_texture->bindAsRT();
        }
    }
    if (p_depthtexture.p_texture) {
        p_depthtexture.p_texture->bindAsRT();
    }
#endif
}

void xGSFrameBufferImpl::unbind()
{
    if (p_resolvebuffer) {
        // attach textures to resolve buffer and resolve AA
        glBindFramebuffer(GL_FRAMEBUFFER, p_resolvebuffer);
        checkFrameBufferStatus();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // resolve AA
        glBindFramebuffer(GL_READ_FRAMEBUFFER, p_framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, p_resolvebuffer);

        if (p_activecolortargets) {
            glBlitFramebuffer(
                0, 0, p_width, p_height, 0, 0, p_width, p_height,
                GL_COLOR_BUFFER_BIT, GL_LINEAR
            );
        }

        if (p_depthstencilformat == GS_DEPTH_DEFAULT) {
            glBlitFramebuffer(
                0, 0, p_width, p_height, 0, 0, p_width, p_height,
                GL_DEPTH_BUFFER_BIT, GL_NEAREST
            );
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

#ifdef _DEBUG
    for (auto &t : p_colortextures) {
        if (t.p_texture) {
            t.p_texture->unbindFromRT();
        }
    }
    if (p_depthtexture.p_texture) {
        p_depthtexture.p_texture->unbindFromRT();
    }
#endif
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
    if (p_framebuffer) {
        glBindFramebuffer(GL_FRAMEBUFFER, p_framebuffer);

        for (int n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
            if (p_colorrb[n]) {
                glFramebufferRenderbuffer(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + n,
                    GL_RENDERBUFFER, 0
                );
                glDeleteRenderbuffers(1, &p_colorrb[n]);
                p_colorrb[n] = 0;
            }
        }

        if (p_depthrb) {
            glFramebufferRenderbuffer(
                GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                GL_RENDERBUFFER, 0
            );
            glDeleteRenderbuffers(1, &p_depthrb);
            p_depthrb = 0;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &p_framebuffer);
        p_framebuffer = 0;
    }

    if (p_resolvebuffer) {
        glDeleteFramebuffers(1, &p_resolvebuffer);
        p_resolvebuffer = 0;
    }
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

void xGSFrameBufferImpl::createRenderBuffer(GLuint &buffer, GLenum format)
{
    glGenRenderbuffers(1, &buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, buffer);
    if (p_multisample) {
        glRenderbufferStorageMultisample(
            GL_RENDERBUFFER, p_multisample,
            format, p_width, p_height
        );
    } else {
        glRenderbufferStorage(GL_RENDERBUFFER, format, p_width, p_height);
    }
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void xGSFrameBufferImpl::checkFrameBufferStatus()
{
    switch (glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
        case GL_FRAMEBUFFER_COMPLETE:
            // all OK
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            p_owner->debug(DebugMessageLevel::SystemError, "FBO incomplete attachments\n");
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            p_owner->debug(DebugMessageLevel::SystemError, "FBO incomplete missing attachment\n");
            break;

        case GL_FRAMEBUFFER_UNSUPPORTED:
            p_owner->debug(DebugMessageLevel::SystemError, "FBO unsupported format combination\n");
            break;

#ifdef GS_CONFIG_FRAMEBUFFER_EXT
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            p_owner->debug(DebugMessageLevel::SystemError, "FBO incomplete dimentions\n");
            break;
#endif

        default:
            p_owner->debug(DebugMessageLevel::SystemError, "FBO bind status unknown\n");
    }
}

static GLenum cubemap_target_face(GSuint slice)
{
    switch (slice & 0xFFFF0000) {
        case GS_LOCK_CUBEMAPNX: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
        case GS_LOCK_CUBEMAPPX: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        case GS_LOCK_CUBEMAPNY: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
        case GS_LOCK_CUBEMAPPY: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
        case GS_LOCK_CUBEMAPNZ: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
        case GS_LOCK_CUBEMAPPZ: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
    }
    return 0;
}

void xGSFrameBufferImpl::attachTexture(const Attachment &texture, GLenum attachment)
{
    if (texture.p_texture == nullptr) {
        glFramebufferTexture(GL_FRAMEBUFFER, attachment, 0, 0);
    } else {
        switch (texture.p_texture->target()) {
            case GL_TEXTURE_1D:
            case GL_TEXTURE_2D:
            case GL_TEXTURE_RECTANGLE:
            case GL_TEXTURE_2D_MULTISAMPLE:
            case GL_TEXTURE_CUBE_MAP:
                if (texture.p_slice) {
                    glFramebufferTexture2D(
                        GL_FRAMEBUFFER, attachment, 
                        cubemap_target_face(texture.p_slice),
                        texture.p_texture->getID(), texture.p_level
                    );
                } else {
                    glFramebufferTexture(
                        GL_FRAMEBUFFER, attachment,
                        texture.p_texture->getID(), texture.p_level
                    );
                }
                break;

            case GL_TEXTURE_1D_ARRAY:
            case GL_TEXTURE_2D_ARRAY:
            case GL_TEXTURE_3D:
                glFramebufferTextureLayer(
                    GL_FRAMEBUFFER, attachment,
                    texture.p_texture->getID(), texture.p_level, texture.p_slice
                );
                break;

            case GL_TEXTURE_CUBE_MAP_ARRAY:
                p_owner->debug(DebugMessageLevel::Information, "attachment texture is cubemap array\n");
                // TODO:
                break;
        }
    }

#ifdef _DEBUG
    p_owner->debugTrackGLError("xGSFrameBufferImpl::attachTexture");
#endif
}

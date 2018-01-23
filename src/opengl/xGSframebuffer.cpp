/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

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
    xGSObjectBase(owner),
    p_framebuffer(0),
    p_resolvebuffer(0)
{
    for (int n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        p_colorrb[n] = 0;
    }

    p_depthrb = 0;
}

xGSFrameBufferImpl::~xGSFrameBufferImpl()
{}

void xGSFrameBufferImpl::AllocateImpl(int multisampled_attachments)
{
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

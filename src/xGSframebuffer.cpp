#include "xGSframebuffer.h"
#include "xGStexture.h"


xGSframebufferImpl::xGSframebufferImpl(xGSimpl *owner) :
    xGSobjectImpl(owner),
    p_fbo(0),
    p_width(0),
    p_height(0),
    p_depthrb(0)
{}

xGSframebufferImpl::~xGSframebufferImpl()
{
    if (p_depthrb) {
        glDeleteRenderbuffers(1, &p_depthrb);
    }

    if (p_fbo) {
        glDeleteFramebuffers(1, &p_fbo);
    }
}

bool xGSframebufferImpl::Allocate(const GSframebufferdesc &desc)
{
    // TODO: check params

    p_width = desc.width;
    p_height = desc.height;

    glGenFramebuffers(1, &p_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, p_fbo);


    for (unsigned int n = 0; n < desc.colortargets; ++n) {
        // TODO: check rt formats, create renderbuffers for non default attachments
    }

    if (desc.attachments) {
        const GSframebufferattachment *att = desc.attachments;
        while (att->attachment != GS_LAST_ATTACHMENT) {
            if (att->attachment == GS_ATTACHMENT_DEPTH) {
                // TODO: handle depth attachment
                continue;
            }

            unsigned int attindex = att->attachment - GS_ATTACHMENT0;

            if (desc.colorformats[attindex] == GS_DEFAULT && att->texture) {
                xGStextureImpl *texture = static_cast<xGStextureImpl*>(att->texture);
                switch (texture->target()) {
                    case GL_TEXTURE_1D:
                    case GL_TEXTURE_2D:
                    case GL_TEXTURE_RECTANGLE:
                        glFramebufferTexture(
                            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attindex,
                            texture->textureId(), att->level
                        );
                        break;

                    case GL_TEXTURE_1D_ARRAY:
                    case GL_TEXTURE_2D_ARRAY:
                        glFramebufferTextureLayer(
                            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attindex,
                            texture->textureId(), att->level, att->slice
                        );
                        break;

                    case GL_TEXTURE_CUBE_MAP:
                        glFramebufferTexture2D(
                            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attindex,
                            texture->locktarget(att->face),
                            texture->textureId(), att->level
                        );
                        break;

                    case GL_TEXTURE_CUBE_MAP_ARRAY:
                        // TODO: array cubemap fb attachment
                        break;

                    case GL_TEXTURE_3D:
                        glFramebufferTexture3D(
                            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attindex,
                            texture->target(), texture->textureId(), att->level, att->slice
                        );
                        break;
                }
            }

            ++att;
        }
    }


    if (desc.colortargets == 0) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    } else {
        GLenum buffers[GS_MAX_FB_ATTACHMENTS];
        for (unsigned int n = 0; n < desc.colortargets; ++n) {
            buffers[n] = desc.colorformats[n] == GS_NONE ? GL_NONE : GL_COLOR_ATTACHMENT0 + n;
        }
        glDrawBuffers(desc.colortargets, buffers);
    }


    if (desc.depthformat != GS_NONE && desc.depthformat != GS_DEFAULT) {
        glGenRenderbuffers(1, &p_depthrb);
        glBindRenderbuffer(GL_RENDERBUFFER, p_depthrb);

        // TODO: make format conv. proc
        GLenum format = 0;
        switch (desc.depthformat) {
            case GS_DEPTH_16: format = GL_DEPTH_COMPONENT16; break;
            case GS_DEPTH_24: format = GL_DEPTH_COMPONENT24; break;
            case GS_DEPTH_32: format = GL_DEPTH_COMPONENT32; break;
            case GS_DEPTH_32_FLOAT: format = GL_DEPTH_COMPONENT32F; break;
            case GS_DEPTH_STENCIL_D24S8: format = GL_DEPTH24_STENCIL8; break;
        }

        glRenderbufferStorage(GL_RENDERBUFFER, format, p_width, p_height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, p_depthrb);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

void xGSframebufferImpl::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, p_fbo);
}

void xGSframebufferImpl::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

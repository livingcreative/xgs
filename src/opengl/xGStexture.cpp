/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/xGStexture.cpp
        Texture object implementation class
*/

#include "xGStexture.h"
#include "kcommon/c_util.h"


using namespace xGS;
using namespace c_util;


xGSTextureImpl::xGSTextureImpl(xGSImpl *owner) :
    xGSObjectBase(owner),
    p_texture(0),
    p_buffer(0)
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

    p_bpp = texdesc.bpp;
    p_GLIntFormat = texdesc.GLIntFormat;
    p_GLFormat = texdesc.GLFormat;
    p_GLType = texdesc.GLType;

    p_target = gl_texture_bind_target(p_texturetype, p_layers > 0, p_multisample > 0);

    glGenTextures(1, &p_texture);
    glBindTexture(p_target, p_texture);

    if (p_texturetype == GS_TEXTYPE_BUFFER) {
        glGenBuffers(1, &p_buffer);
        glBindBuffer(GL_TEXTURE_BUFFER, p_buffer);
        // TODO: immutable buffer
        glBufferData(GL_TEXTURE_BUFFER, p_width * p_bpp, nullptr, GL_STATIC_DRAW);
        glTexBuffer(GL_TEXTURE_BUFFER, p_GLIntFormat, p_buffer);
    } else {
        // texture parameters
        glTexParameteri(p_target, GL_TEXTURE_BASE_LEVEL, p_minlevel);
        glTexParameteri(p_target, GL_TEXTURE_MAX_LEVEL, p_maxlevel);

        SetImage(p_target, (p_maxlevel - p_minlevel) > 0 && p_texturetype != GS_TEXTYPE_RECT);
    }

    return p_owner->error(GS_OK);
}

GSptr xGSTextureImpl::LockImpl(GSenum locktype, GSdword access, GSint level, GSint layer, void *lockdata)
{
    GLenum buffer_target = 0;
    GLenum buffer_usage = 0;

    if (p_texturetype == GS_TEXTYPE_BUFFER) {
        buffer_target = GL_TEXTURE_BUFFER;
        glBindBuffer(GL_TEXTURE_BUFFER, p_buffer);
    } else {
        switch (access) {
            case GS_WRITE:
                buffer_target = GL_PIXEL_UNPACK_BUFFER;
                buffer_usage = GL_STATIC_DRAW;
                break;

            case GS_READ:
                buffer_target = GL_PIXEL_PACK_BUFFER;
                buffer_usage = GL_STATIC_READ;
                break;
        }

        glGenBuffers(1, &p_lockbuffer);
        glBindBuffer(buffer_target, p_lockbuffer);

        GLsizeiptr size = 0;
        switch (p_texturetype) {
            case GS_TEXTYPE_1D:      size = p_width >> level; break;
            case GS_TEXTYPE_2D:      size = umax(p_width >> level, 1u) * umax(p_height >> level, 1u); break;
            case GS_TEXTYPE_3D:      size = umax(p_width >> level, 1u) * umax(p_height >> level, 1u) * umax(p_depth >> level, 1u); break;
            case GS_TEXTYPE_CUBEMAP: size = umax(p_width >> level, 1u) * umax(p_height >> level, 1u); break;
        }
        size *= p_bpp;

        // TODO: immutable buffer
        glBufferData(buffer_target, size, nullptr, buffer_usage);

        if (access == GS_READ) {
            glBindTexture(p_target, p_texture);
            glGetTexImage(p_target, level, p_GLFormat, p_GLType, nullptr);
        }
    }

    p_locktype = locktype;
    p_lockaccess = access;
    p_locklayer = layer;
    p_locklevel = level;

    p_owner->error(GS_OK);

    return glMapBuffer(buffer_target, GL_WRITE_ONLY);
}

void xGSTextureImpl::UnlockImpl()
{
    if (p_texturetype == GS_TEXTYPE_BUFFER) {
        glBindBuffer(GL_TEXTURE_BUFFER, p_buffer);
        glUnmapBuffer(GL_TEXTURE_BUFFER);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
    } else {
        switch (p_lockaccess) {
            case GS_WRITE:
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, p_lockbuffer);
                glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

                glBindTexture(p_target, p_texture);

                switch (p_locktype) {
                    case GS_LOCK_TEXTURE:   UpdateImage(p_target, p_locklevel, nullptr); break;
                    case GS_LOCK_CUBEMAPNX: UpdateImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, p_locklevel, nullptr); break;
                    case GS_LOCK_CUBEMAPPX: UpdateImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X, p_locklevel, nullptr); break;
                    case GS_LOCK_CUBEMAPNY: UpdateImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, p_locklevel, nullptr); break;
                    case GS_LOCK_CUBEMAPPY: UpdateImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, p_locklevel, nullptr); break;
                    case GS_LOCK_CUBEMAPNZ: UpdateImage(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, p_locklevel, nullptr); break;
                    case GS_LOCK_CUBEMAPPZ: UpdateImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, p_locklevel, nullptr); break;
                }

                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

                break;

            case GS_READ:
                glBindBuffer(GL_PIXEL_PACK_BUFFER, p_lockbuffer);
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
                glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                break;
        }

        glDeleteBuffers(1, &p_lockbuffer);
        p_lockbuffer = 0;
    }

    p_lockaccess = 0;
    p_locklayer = 0;
    p_locktype = GS_NONE;
}

void xGSTextureImpl::bindNullTexture()
{
    const GSuint target_count = 10;
    const GLenum targets[target_count] = {
        GL_TEXTURE_1D,
        GL_TEXTURE_2D,
        GL_TEXTURE_3D,
        GL_TEXTURE_CUBE_MAP,

        GL_TEXTURE_RECTANGLE,

        GL_TEXTURE_1D_ARRAY,
        GL_TEXTURE_2D_ARRAY,
        GL_TEXTURE_CUBE_MAP_ARRAY,

        GL_TEXTURE_2D_MULTISAMPLE,

        GL_TEXTURE_2D_MULTISAMPLE_ARRAY
    };

    for (GSuint t = 0; t < target_count; ++t) {
        glBindTexture(targets[t], 0);
    }
}

void xGSTextureImpl::ReleaseRendererResources()
{
    if (p_locktype) {
        UnlockImpl();
    }

    if (p_texture) {
        glDeleteTextures(1, &p_texture);
        p_texture = 0;
    }

    if (p_buffer) {
        glDeleteBuffers(1, &p_buffer);
    }
}

void xGSTextureImpl::SetImage(GLenum gltarget, bool mipcascade)
{
    switch (gltarget) {
        case GL_TEXTURE_1D:
            SetImage1D(mipcascade);
            break;

        case GL_TEXTURE_2D:
        case GL_TEXTURE_RECTANGLE:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            SetImage2D(gltarget, mipcascade);
            break;

        case GL_TEXTURE_2D_MULTISAMPLE:
            SetImage2DMultisample(mipcascade);
            break;

        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
            SetImage3D(gltarget, mipcascade);
            break;
    }
}

void xGSTextureImpl::SetImage1D(bool mipcascade)
{
#ifdef GS_CONFIG_TEXTURE_STORAGE
    GSuint levels = 1;

    if (mipcascade) {
        int width = p_width;
        while (width > 1 && levels <= p_maxlevel) {
            width = width >> 1;
            ++levels;
        }
    }

    glTexStorage1D(GL_TEXTURE_1D, levels - p_minlevel, p_GLIntFormat, p_width);
#else
    glTexImage1D(
        GL_TEXTURE_1D, p_minlevel, p_GLIntFormat,
        p_width >> p_minlevel, 0, p_GLFormat, p_GLType, nullptr
    );

    if (mipcascade) {
        int width = p_width >> p_minlevel;
        GSuint level = p_minlevel;
        do {
            width = width >> 1;
            ++level;
            glTexImage1D(
                GL_TEXTURE_1D, level, p_GLIntFormat,
                width, 0, p_GLFormat, p_GLType, nullptr
            );
        } while (width > 0 && level < p_maxlevel);
    }
#endif
}

void xGSTextureImpl::SetImage2D(GLenum gltarget, bool mipcascade)
{
#ifdef GS_CONFIG_TEXTURE_STORAGE
    int levels = 1;

    if (mipcascade) {
        int width = p_width;
        int height = p_height;
        while (width > 1 || height > 1) {
            width = width >> 1;
            height = height >> 1;
            ++levels;
        }
    }

    glTexStorage2D(gltarget, levels, p_GLIntFormat, p_width, p_height);
#else
    if (gltarget == GL_TEXTURE_CUBE_MAP) {
        SetImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, mipcascade);
        SetImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, mipcascade);
        SetImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, mipcascade);
        SetImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, mipcascade);
        SetImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, mipcascade);
        SetImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, mipcascade);
    } else {
        glTexImage2D(
            gltarget, 0, p_GLIntFormat,
            p_width, p_height, 0, p_GLFormat, p_GLType, nullptr
        );

        if (mipcascade) {
            int width = p_width;
            int height = p_height;
            int level = 0;
            do {
                width = width >> 1;
                height = height >> 1;
                ++level;
                glTexImage2D(
                    gltarget, level, p_GLIntFormat,
                    c_util::umax(width, 1), c_util::umax(height, 1),
                    0, p_GLFormat, p_GLType, nullptr
                );
            } while (width > 1 && height > 1);
        }
    }
#endif
}

void xGSTextureImpl::SetImage2DMultisample(bool mipcascade)
{
#ifdef GS_CONFIG_TEXTURE_STORAGE_MULTISAMPLE
    glTexStorage2DMultisample(
        GL_TEXTURE_2D_MULTISAMPLE, p_multisample, p_GLIntFormat,
        p_width, p_height, GL_TRUE
    );
#else
    glTexImage2DMultisample(
        GL_TEXTURE_2D_MULTISAMPLE, p_multisample, p_GLIntFormat,
        p_width, p_height, GL_TRUE
    );
#endif
}

void xGSTextureImpl::SetImage3D(GLenum gltarget, bool mipcascade)
{
    int depth = gltarget == GL_TEXTURE_2D_ARRAY ? p_layers : p_depth;

#ifdef GS_CONFIG_TEXTURE_STORAGE
    int levels = 1;

    if (mipcascade) {
        int width = p_width;
        int height = p_height;
        while (width > 1 || height > 1 || depth > 1) {
            width = width >> 1;
            height = height >> 1;

            if (gltarget == GL_TEXTURE_3D) {
                depth = depth >> 1;
            } else if (width <= 0 && height <= 0) {
                break;
            }

            ++levels;
        }
    }

    glTexStorage3D(gltarget, levels, p_GLIntFormat, p_width, p_height, depth);
#else
    glTexImage3D(
        gltarget, 0, p_GLIntFormat,
        p_width, p_height, depth, 0, p_GLFormat, p_GLType, nullptr
    );

    if (mipcascade) {
        int width = p_width;
        int height = p_height;
        int level = 0;
        do {
            width = width >> 1;
            height = height >> 1;

            if (gltarget == GL_TEXTURE_3D) {
                depth = depth >> 1;
            } else if (width <= 0 && height <= 0) {
                break;
            }

            ++level;
            glTexImage3D(
                gltarget, level, p_GLIntFormat,
                umax(width, 1), umax(height, 1), umax(depth, 1),
                0, p_GLFormat, p_GLType, nullptr
            );
        } while (width > 0 || height > 0 || depth > 0);
    }
#endif
}

void xGSTextureImpl::UpdateImage(GLenum gltarget, int level, GSptr data)
{
    switch (gltarget) {
        case GL_TEXTURE_1D:
            glTexSubImage1D(gltarget, level, 0, p_width >> level, p_GLFormat, p_GLType, data);
            break;

        case GL_TEXTURE_2D:
        case GL_TEXTURE_RECTANGLE:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            glTexSubImage2D(
                gltarget, level, 0, 0,
                umax(p_width >> level, 1u), umax(p_height >> level, 1u),
                p_GLFormat, p_GLType, data
            );
            break;

        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
            glTexSubImage3D(
                gltarget, level, 0, 0, gltarget == GL_TEXTURE_2D_ARRAY ? p_locklayer : 0,
                umax(p_width >> level, 1u),
                umax(p_height >> level, 1u),
                gltarget == GL_TEXTURE_2D_ARRAY ? 1 : umax(p_depth >> level, 1u),
                p_GLFormat, p_GLType, data
            );
            break;
    }
}

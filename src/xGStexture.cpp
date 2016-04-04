#include "xGStexture.h"
#include "xGSutil.h"


xGStextureImpl::xGStextureImpl(xGSimpl *owner) :
    xGSobjectImpl(owner),
    p_texture(0),
    p_target(0),
    p_intformat(0),
    p_format(0),
    p_type(0),

    p_width(0),
    p_height(0),
    p_depth(0),

    p_lockbuffer(0)
{}

xGStextureImpl::~xGStextureImpl()
{
    if (p_lockbuffer) {
        DoUnlock();
    }

    if (p_texture) {
        glDeleteTextures(1, &p_texture);
    }
}

void *xGStextureImpl::Lock(GSlocktexture lock, unsigned int level, unsigned int layer, unsigned int flags)
{
    if (p_lockbuffer) {
        // TODO: handle error
        return nullptr;
    }

    GLuint access = 0;

    switch (flags) {
        case GS_READ:
            // TODO: lock texture for reading
            break;

        case GS_WRITE:
            access = GL_WRITE_ONLY;
            p_locktarget = GL_PIXEL_UNPACK_BUFFER;
            break;

        default:
            // TODO: handle error
            return nullptr;
    }

    glGenBuffers(1, &p_lockbuffer);
    glBindBuffer(p_locktarget, p_lockbuffer);
    p_locklevel = level;
    p_locklayer = layer;
    p_lock = lock;


    GLuint w, h, d;
    ComputeLevelSize(level, w, h, d);

    // TODO: calc correct compsize
    GLuint compsize = 4;

    // TODO: storage for PBO's, usage
    glBufferData(p_locktarget, w * h * d * compsize, nullptr, GL_STATIC_DRAW);

    return glMapBuffer(p_locktarget, access);
}

bool xGStextureImpl::Unlock()
{
    if (!p_lockbuffer) {
        // TODO: handle error
        return false;
    }

    DoUnlock();

    return true;
}

bool xGStextureImpl::Allocate(const GStexturedesc &desc)
{
    switch (desc.type) {
        case GS_TEXTURE_1D:
            p_target = desc.layers ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;
            break;

        case GS_TEXTURE_2D:
            if (desc.layers) {
                p_target = desc.multisample ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_ARRAY;
            } else {
                p_target = desc.multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
            }
            break;

        case GS_TEXTURE_3D:
            p_target = GL_TEXTURE_3D;
            break;

        case GS_TEXTURE_RECTANGLE:
            p_target = GL_TEXTURE_RECTANGLE;
            break;

        case GS_TEXTURE_CUBEMAP:
            p_target = desc.layers ? GL_TEXTURE_CUBE_MAP_ARRAY : GL_TEXTURE_CUBE_MAP;
            break;

        case GS_TEXTURE_BUFFER:
            p_target = GL_TEXTURE_BUFFER;
            break;

        default:
            // TODO: handle error
            return false;
    }

    // I will replace this shit with more practical solution later ;)
    switch (desc.format) {
        case GS_COLOR_R:
            p_intformat = GL_R8;
            p_format = GL_RED;
            p_type = GL_UNSIGNED_BYTE;
            break;

        case GS_COLOR_RG:
            p_intformat = GL_RG8;
            p_format = GL_RG;
            p_type = GL_UNSIGNED_BYTE;
            break;

        case GS_COLOR_RGBX:
            p_intformat = desc.sRGB ? GL_SRGB8 : GL_RGB8;
            p_format = GL_BGR;
            p_type = GL_UNSIGNED_BYTE;
            break;

        case GS_COLOR_RGBA:
            p_intformat = desc.sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
            p_format = GL_BGRA;
            p_type = GL_UNSIGNED_BYTE;
            break;

        case GS_COLOR_R_HALF_FLOAT:
            p_intformat = GL_R16F;
            p_format = GL_RED;
            p_type = GL_HALF_FLOAT;
            break;

        case GS_COLOR_RG_HALF_FLOAT:
            p_intformat = GL_RG16F;
            p_format = GL_RG;
            p_type = GL_HALF_FLOAT;
            break;

        case GS_COLOR_RGBX_HALF_FLOAT:
            p_intformat = GL_RGB16F;
            p_format = GL_RGB;
            p_type = GL_HALF_FLOAT;
            break;

        case GS_COLOR_RGBA_HALF_FLOAT:
            p_intformat = GL_RGBA16F;
            p_format = GL_RGBA;
            p_type = GL_HALF_FLOAT;
            break;

        case GS_COLOR_R_FLOAT:
            p_intformat = GL_R32F;
            p_format = GL_RED;
            p_type = GL_FLOAT;
            break;

        case GS_COLOR_RG_FLOAT:
            p_intformat = GL_RG32F;
            p_format = GL_RG;
            p_type = GL_FLOAT;
            break;

        case GS_COLOR_RGBX_FLOAT:
            p_intformat = GL_RGB32F;
            p_format = GL_RGB;
            p_type = GL_FLOAT;
            break;

        case GS_COLOR_RGBA_FLOAT:
            p_intformat = GL_RGBA32F;
            p_format = GL_RGBA;
            p_type = GL_FLOAT;
            break;

        case GS_DEPTH_16:
            p_intformat = GL_DEPTH_COMPONENT16;
            p_format = GL_DEPTH_COMPONENT;
            p_type = GL_UNSIGNED_SHORT;
            break;

        case GS_DEPTH_24:
            p_intformat = GL_DEPTH_COMPONENT24;
            p_format = GL_DEPTH_COMPONENT;
            p_type = GL_UNSIGNED_SHORT;
            break;

        case GS_DEPTH_32:
            p_intformat = GL_DEPTH_COMPONENT32;
            p_format = GL_DEPTH_COMPONENT;
            p_type = GL_UNSIGNED_INT;
            break;

        case GS_DEPTH_32_FLOAT:
            p_intformat = GL_DEPTH_COMPONENT32F;
            p_format = GL_DEPTH_COMPONENT;
            p_type = GL_FLOAT;
            break;


        case GS_DEPTH_STENCIL_D24S8:
            p_intformat = GL_DEPTH24_STENCIL8;
            p_format = GL_DEPTH_STENCIL;
            p_type = GL_UNSIGNED_INT;
            break;
    }

    glGenTextures(1, &p_texture);
    glBindTexture(p_target, p_texture);

    glTexParameteri(p_target, GL_TEXTURE_MAX_LEVEL, desc.levels);

    p_width = desc.width;
    p_height = desc.type == GS_TEXTURE_1D ? 1 : desc.height;
    p_depth = desc.type == GS_TEXTURE_3D ? desc.depth : 1;

    if (desc.type == GS_TEXTURE_BUFFER) {
        // TODO: buffer textures
    } else {
        AllocateTextureImage(desc);
    }

    return true;
}

void xGStextureImpl::AllocateTextureImage(const GStexturedesc &desc)
{
    switch (p_target) {
        case GL_TEXTURE_1D:
            glTexStorage1D(p_target, desc.levels, p_intformat, desc.width);
            break;

        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_RECTANGLE:
        case GL_TEXTURE_CUBE_MAP:
            glTexStorage2D(
                p_target, desc.levels, p_intformat, desc.width,
                p_target == GL_TEXTURE_1D_ARRAY ? desc.layers : desc.height
            );
            break;

        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_CUBE_MAP_ARRAY:
        case GL_TEXTURE_3D:
            glTexStorage3D(
                p_target, desc.levels, p_intformat, desc.width, desc.height,
                p_target == GL_TEXTURE_3D ? desc.depth : desc.layers
            );
            break;

        case GL_TEXTURE_2D_MULTISAMPLE:
            glTexStorage2DMultisample(p_target, desc.multisample, p_intformat, desc.width, desc.height, GL_TRUE);
            break;

        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
            glTexStorage3DMultisample(p_target, desc.multisample, p_intformat, desc.width, desc.height, desc.layers, GL_TRUE);
            break;
    }
}

void xGStextureImpl::ComputeLevelSize(GLuint level, GLuint &w, GLuint &h, GLuint &d)
{
    w = umax(1u, p_width >> level);
    h = umax(1u, p_height >> level);
    d = umax(1u, p_depth >> level);
}

void xGStextureImpl::DoUnlock()
{
    if (p_locktarget == GL_PIXEL_UNPACK_BUFFER) {
        glBindBuffer(p_locktarget, p_lockbuffer);

        GLuint w, h, d;
        ComputeLevelSize(p_locklevel, w, h, d);

        // update texture image data from buffer
        switch (p_target) {
            case GL_TEXTURE_1D:
                glTexSubImage1D(p_target, p_locklevel, 0, w, p_format, p_type, nullptr);
                break;

            case GL_TEXTURE_1D_ARRAY:
                glTexSubImage2D(p_target, p_locklevel, 0, p_locklevel, w, 1, p_format, p_type, nullptr);
                break;

            case GL_TEXTURE_2D:
            case GL_TEXTURE_RECTANGLE:
            case GL_TEXTURE_CUBE_MAP: {
                GLenum target = p_target;
                if (p_target == GL_TEXTURE_CUBE_MAP) {
                    switch (p_lock) {
                        case GS_LOCK_POSX: target = GL_TEXTURE_CUBE_MAP_POSITIVE_X; break;
                        case GS_LOCK_NEGX: target = GL_TEXTURE_CUBE_MAP_NEGATIVE_X; break;
                        case GS_LOCK_POSY: target = GL_TEXTURE_CUBE_MAP_POSITIVE_Y; break;
                        case GS_LOCK_NEGY: target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y; break;
                        case GS_LOCK_POSZ: target = GL_TEXTURE_CUBE_MAP_POSITIVE_Z; break;
                        case GS_LOCK_NEGZ: target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; break;
                    }
                }
                glTexSubImage2D(target, p_locklevel, 0, 0, w, h, p_format, p_type, nullptr);
                break;
            }

            case GL_TEXTURE_2D_ARRAY:
            case GL_TEXTURE_3D:
                glTexSubImage3D(
                    p_target, p_locklevel, 0, 0,
                    p_target == GL_TEXTURE_2D_ARRAY ? p_locklayer : 0, w, h, d,
                    p_format, p_type, nullptr
                );
                break;
        }
    }

    glDeleteBuffers(1, &p_lockbuffer);
    p_lockbuffer = 0;
}

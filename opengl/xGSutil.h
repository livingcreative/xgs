/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    xGSutil.h
        Utility functions and classes
*/

#pragma once

#include "xGS/xGS.h"
#include "GL/glew.h"
#include <vector>


namespace xGS
{

    // forwards
    class xGSImpl;
    class xGSStateImpl;
    class xGSUniformBufferImpl;
    class xGSTextureImpl;


    static const GSuint GS_UNDEFINED = 0xFFFFFFFF;

    enum class DebugMessageLevel
    {
        Information,
        Warning,
        Error,
        SystemError
    };



    enum GSbits
    {
        b2   = 0x01,
        b4   = 0x02,
        b8   = 0x04,
        b16  = 0x08,
        b24  = 0x10,
        b32  = 0x20,
        b64  = 0x40,
        b128 = 0x80
    };

    enum GSsamples
    {
        s2  = 0x01,
        s4  = 0x02,
        s8  = 0x04,
        s16 = 0x08
    };

    struct GSpixelformat     // compact pixel format structure
    {
        GSint  pfColorBits;   // bits for color
        GSint  pfAlphaBits;   // bits for alpha channel
        GSint  pfDepthBits;   // bits for depth
        GSint  pfStencilBits; // bits for stencil
        GSint  pfMultisample; // AA type (samples count)
        GSbool pfSRGB;
    };


    struct GScaps
    {
        GLint  max_active_attribs;
        GLint  max_texture_units;
        GLint  max_ubo_size;
        GLint  ubo_alignment;
        GSbool multi_bind;
        GSbool multi_blend;
        GSbool vertex_format;
        GSbool texture_srgb;
        GSbool texture_float;
        GSbool texture_depth;
        GSbool texture_depthstencil;
    };


    inline GSptr ptr_offset(GSptr ptr, GSint offset)
    {
        return reinterpret_cast<char*>(ptr) + offset;
    }


    inline GSenum ColorFormatFromPixelFormat(const GSpixelformat &fmt)
    {
        switch (fmt.pfColorBits) {
            case 32: return fmt.pfAlphaBits ? GS_COLOR_RGBA : GS_COLOR_RGBX;
        }

        return GS_NONE;
    }

    inline GSenum DepthFormatFromPixelFormat(const GSpixelformat &fmt)
    {
        if (fmt.pfDepthBits == 24 && fmt.pfStencilBits == 8) {
            return GS_DEPTHSTENCIL_D24S8;
        } else {
            switch (fmt.pfDepthBits) {
                case 16: return GS_DEPTH_16;
                case 24: return GS_DEPTH_24;
                case 32: return GS_DEPTH_32;
            }
            return GS_NONE;
        }
    }

    inline GLenum gl_index_type(GSenum type)
    {
        switch (type) {
            case GS_INDEX_16: return GL_UNSIGNED_SHORT;
            case GS_INDEX_32: return GL_UNSIGNED_INT;
        }

        return 0;
    }

    inline GLenum gl_primitive_type(GSenum type)
    {
        switch (type) {
            case GS_PRIM_POINTS:        return GL_POINTS;
            case GS_PRIM_LINES:         return GL_LINES;
            case GS_PRIM_LINESTRIP:     return GL_LINE_STRIP;
            case GS_PRIM_TRIANGLES:     return GL_TRIANGLES;
            case GS_PRIM_TRIANGLESTRIP: return GL_TRIANGLE_STRIP;
            case GS_PRIM_TRIANGLEFAN:   return GL_TRIANGLE_FAN;
            case GS_PRIM_PATCHES:       return GL_PATCHES;
        }

        return 0;
    }

    inline GLenum gl_texture_bind_target(GSenum type, bool isarray, bool multisample)
    {
        switch (type) {
            case GS_TEXTYPE_1D:
                return isarray ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;

            case GS_TEXTYPE_2D:
                if (multisample) {
                    return isarray ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_MULTISAMPLE;
                } else {
                    return isarray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
                }

            case GS_TEXTYPE_RECT:
                return isarray ? 0 : GL_TEXTURE_RECTANGLE;

            case GS_TEXTYPE_3D:
                return isarray ? 0 : GL_TEXTURE_3D;

            case GS_TEXTYPE_CUBEMAP:
                return isarray ? GL_TEXTURE_CUBE_MAP_ARRAY : GL_TEXTURE_CUBE_MAP;

            case GS_TEXTYPE_BUFFER:
                return GL_TEXTURE_BUFFER;
        }

        return 0;
    }

    inline GLenum gl_texture_wrap(GSenum wrap)
    {
        switch (wrap) {
            case GS_WRAP_CLAMP:  return GL_CLAMP_TO_EDGE;
            case GS_WRAP_REPEAT: return GL_REPEAT;
        }

        return 0;
    }

    inline GLenum gl_compare_func(GSenum func)
    {
        switch (func) {
            case GS_DEPTHTEST_NEVER:    return GL_NEVER;
            case GS_DEPTHTEST_LESS:     return GL_LESS;
            case GS_DEPTHTEST_LEQUAL:   return GL_LEQUAL;
            case GS_DEPTHTEST_EQUAL:    return GL_EQUAL;
            case GS_DEPTHTEST_NOTEQUAL: return GL_NOTEQUAL;
            case GS_DEPTHTEST_GREATER:  return GL_GREATER;
            case GS_DEPTHTEST_GEQUAL:   return GL_GEQUAL;
            case GS_DEPTHTEST_ALWAYS:   return GL_ALWAYS;
        }

        return 0;
    }

    inline GLenum gl_fill_mode(GSenum mode)
    {
        switch (mode) {
            case GS_FILL_POINT:   return GL_POINT;
            case GS_FILL_LINE:    return GL_LINE;
            case GS_FILL_POLYGON: return GL_FILL;
        }

        return 0;
    }

    inline GLenum gl_cull_face(GSenum mode)
    {
        return mode == GS_CULL_CW ? GL_FRONT : GL_BACK;
    }

    inline GLenum gl_blend_eq(GSenum eq)
    {
        switch (eq) {
            case GS_BLEND_ADD:      return GL_FUNC_ADD;
            case GS_BLEND_SUBTRACT: return GL_FUNC_SUBTRACT;
            case GS_BLEND_MIN:      return GL_MIN;
            case GS_BLEND_MAX:      return GL_MAX;
        }

        return 0;
    }

    inline GLenum gl_blend_factor(GSenum f)
    {
        switch (f) {
            case GSBF_ZERO:             return GL_ZERO;
            case GSBF_ONE:              return GL_ONE;
            case GSBF_SRCCOLOR:         return GL_SRC_COLOR;
            case GSBF_INVSRCCOLOR:      return GL_ONE_MINUS_SRC_COLOR;
            case GSBF_DSTCOLOR:         return GL_DST_COLOR;
            case GSBF_INVDSTCOLOR:      return GL_ONE_MINUS_DST_COLOR;
            case GSBF_SRCALPHA:         return GL_SRC_ALPHA;
            case GSBF_INVSRCALPHA:      return GL_ONE_MINUS_SRC_ALPHA;
            case GSBF_DSTALPHA:         return GL_DST_ALPHA;
            case GSBF_INVDSTALPHA:      return GL_ONE_MINUS_DST_ALPHA;
            case GSBF_CONSTANTCOLOR:    return GL_CONSTANT_COLOR;
            case GSBF_INVCONSTANTCOLOR: return GL_ONE_MINUS_CONSTANT_COLOR;
            case GSBF_CONSTANTALPHA:    return GL_CONSTANT_ALPHA;
            case GSBF_INVCONSTANTALPHA: return GL_ONE_MINUS_CONSTANT_ALPHA;
            case GSBF_SRCALPHA_SAT:     return GL_SRC_ALPHA_SATURATE;
            case GSBF_SRC1COLOR:        return GL_SRC1_COLOR;
            case GSBF_INVSRC1COLOR:     return GL_ONE_MINUS_SRC1_COLOR;
            case GSBF_SRC1ALPHA:        return GL_SRC1_ALPHA;
            case GSBF_INVSRC1ALPHA:     return GL_ONE_MINUS_SRC1_ALPHA;
        }

        return 0;
    }

    inline GSint vertex_component_count(const GSvertexcomponent &c)
    {
        switch (c.type) {
            case GSVD_POSITION:  return 3;
            case GSVD_POSITIONW: return 4;
            case GSVD_VECTOR1:   return 1;
            case GSVD_VECTOR2:   return 2;
            case GSVD_VECTOR3:   return 3;
            case GSVD_VECTOR4:   return 4;

            default:
                return 0;
        }
    }

    inline GSint vertex_component_size(const GSvertexcomponent &c)
    {
        switch (c.type) {
            case GSVD_POSITION:  return sizeof(float) * 3;
            case GSVD_POSITIONW: return sizeof(float) * 4;
            case GSVD_VECTOR1:   return sizeof(float) * 1;
            case GSVD_VECTOR2:   return sizeof(float) * 2;
            case GSVD_VECTOR3:   return sizeof(float) * 3;
            case GSVD_VECTOR4:   return sizeof(float) * 4;

            default:
                return 0;
        }
    }

    inline GSint index_buffer_size(GSenum format, int count = 1)
    {
        int result = 0;

        switch (format) {
            case GS_INDEX_16: result = 2; break;
            case GS_INDEX_32: result = 4; break;
        }

        return result * count;
    }

    inline GSuint align(GSuint value, GSuint align)
    {
        GSuint result = value / align;
        if (value % align) {
            ++result;
        }
        return result * align;
    }


#ifdef _DEBUG
    const char* uniform_type(GLenum type);
#endif


    class GSvertexdecl
    {
    public:
        GSvertexdecl();
        GSvertexdecl(const GSvertexcomponent *decl);

        GSint buffer_size(int count = 1) const;
        GSbool dynamic() const { return p_dynamic; }
        const std::vector<GSvertexcomponent>& declaration() const { return p_decl; }

        void initialize(const GSvertexcomponent *decl);

    protected:
        std::vector<GSvertexcomponent> p_decl;
        bool                           p_dynamic;
    };


    struct GSParameterSet
    {
        GSenum settype;
        GSuint first;
        GSuint onepastlast;
        GSuint firstsampler;
        GSuint onepastlastsampler;
        GSuint constantcount;
    };

    // internal parameters object implementation
    // which is used by state object to hold static resource bindings
    class GSParametersState
    {
    public:
        GSParametersState();

        GSerror allocate(xGSImpl *impl, xGSStateImpl *state, const GSParameterSet &set, const GSuniformbinding *uniforms, const GStexturebinding *textures, const GSconstantvalue *constants);
        void apply(const GScaps &caps, xGSImpl *impl, xGSStateImpl *state);

        void ReleaseRendererResources(xGSImpl *impl);

    protected:
        struct UniformBlockData
        {
            GLuint                index;
            GSuint                offset;
            GSuint                size;
            xGSUniformBufferImpl *buffer;
        };

        struct TextureSlot
        {
            xGSTextureImpl *texture;
            GSuint          sampler;
        };

        struct ConstantValue
        {
            GSenum type;
            GSenum location;
            size_t offset;
        };

        typedef std::vector<UniformBlockData> UniformBlockDataSet;
        typedef std::vector<TextureSlot> TextureSet;
        typedef std::vector<GLuint> TextureBinding;
        typedef std::vector<ConstantValue> ConstantValues;
        typedef std::vector<char> ConstantMemory;

    protected:
        UniformBlockDataSet p_uniformblockdata;
        TextureSet          p_textures;
        GLuint              p_firstslot;
        TextureBinding      p_texture_targets;
        TextureBinding      p_texture_binding;
        TextureBinding      p_texture_samplers;
        ConstantValues      p_constants;
        ConstantMemory      p_constantmemory;
    };

} // namespace xGS

/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    opengl/xGSGLutil.h
        OpenGL specific utility functions and classes
*/

#pragma once

#include "xGS/xGS.h"
#include "glplatform.h"
#include <vector>


namespace xGS
{

    // forwards
    class xGSImpl;
    class xGSStateImpl;
    class xGSDataBufferImpl;
    class xGSTextureImpl;

    struct GScaps;
    struct GSParameterSet;


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

#ifdef _DEBUG
    const char* uniform_type(GLenum type);
#endif

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
            GLuint             index;
            GSuint             offset;
            GSuint             size;
            xGSDataBufferImpl *buffer;
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

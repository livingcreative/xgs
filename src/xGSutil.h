#pragma once

#include "xGS/xGS.h"
#include "GL/glew.h"
#include <vector>


template <typename T>
inline T umax(T a, T b)
{
    return a > b ? a : b;
}


inline bool uniformissampler(GLenum type)
{
    return
        type == GL_SAMPLER_1D ||
        type == GL_SAMPLER_1D_ARRAY ||
        type == GL_SAMPLER_2D ||
        type == GL_SAMPLER_2D_ARRAY ||
        type == GL_SAMPLER_2D_MULTISAMPLE ||
        type == GL_SAMPLER_2D_MULTISAMPLE_ARRAY ||
        type == GL_SAMPLER_2D_RECT ||
        type == GL_SAMPLER_CUBE ||
        type == GL_SAMPLER_CUBE_MAP_ARRAY ||
        type == GL_SAMPLER_3D ||
        type == GL_SAMPLER_2D_SHADOW ||
        type == GL_SAMPLER_2D_ARRAY_SHADOW;
}


inline GLenum glindextype(GSindexformat format)
{
    switch (format) {
        case GS_INDEX_WORD: return GL_UNSIGNED_SHORT;
        case GS_INDEX_DWORD: return GL_UNSIGNED_INT;
    }
    return 0;
}

inline GLuint indexsize(GSindexformat format, GLuint count = 1)
{
    GLuint result = 0;

    switch (format) {
        case GS_INDEX_WORD: result = sizeof(GLushort); break;
        case GS_INDEX_DWORD: result = sizeof(GLuint); break;
    }

    return result * count;
}

inline GLuint vertexcomponentcount(GSvertexcomponenttype type)
{
    switch (type) {
        case GS_FLOAT: return 1;
        case GS_VEC2: return 2;
        case GS_VEC3: return 3;
        case GS_VEC4: return 4;
    }
    return 0;
}

inline GLuint vertexcomponentsize(GSvertexcomponenttype type)
{
    switch (type) {
        case GS_FLOAT: return 1 * sizeof(float);
        case GS_VEC2: return 2 * sizeof(float);
        case GS_VEC3: return 3 * sizeof(float);
        case GS_VEC4: return 4 * sizeof(float);
    }
    return 0;
}

inline GLenum glwrapmode(GSwrapmode mode)
{
    switch (mode) {
        case GS_WRAP_CLAMP:
            return GL_CLAMP_TO_EDGE;

        case GS_WRAP_REPEAT:
            return GL_REPEAT;
    }

    return 0;
}

class VertexDecl
{
public:
    VertexDecl() :
        p_size(0)
    {}

    VertexDecl(const GSvertexcomponent *decl)
    {
        reset(decl);
    }

    void reset(const GSvertexcomponent *decl)
    {
        p_size = 0;
        while (decl->type != GS_LAST_COMPONENT) {
            p_size += vertexcomponentsize(decl->type);
            p_decl.emplace_back(*decl);
            ++decl;
        }
    }

    GLuint size(GLuint count = 1) const { return p_size * count; }

private:
    std::vector<GSvertexcomponent> p_decl;
    GLuint                         p_size;
};


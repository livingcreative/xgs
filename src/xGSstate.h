#pragma once

#include "xGS/xGS.h"
#include "GL/glew.h"
#include <vector>
#include <map>
#include <string>


class xGStextureImpl;


class xGSstateImpl : public xGSstate
{
public:
    xGSstateImpl();
    ~xGSstateImpl();

    bool Allocate(const GSstatedesc &desc);

    void Apply(const GLuint *samplers);

private:
    void EnumProgramInputs();
    void EnumProgramParameters();

private:
    struct Attrib
    {
        char    name[256];
        GLsizei len;
        GLint   size;
        GLenum  type;
        GLint   location;
    };

    struct Sampler
    {
        GLuint          slot;
        xGStextureImpl *texture;
        GLuint          sampler;
    };

    GLuint p_program;
    GLuint p_vao;

    std::vector<Attrib> p_attribs;
    std::vector<Sampler> p_samplers;
    std::map<std::string, size_t> p_samplermap; // TODO: temprorary solution
};

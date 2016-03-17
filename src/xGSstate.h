#pragma once

#include "xGS/xGS.h"
#include "GL/glew.h"
#include <vector>


class xGSstateImpl : public xGSstate
{
public:
    xGSstateImpl();
    ~xGSstateImpl();

    bool Allocate(const GSstatedesc &desc);

    void Apply();

private:
    void EnumProgramInputs();

private:
    struct Attrib
    {
        char    name[256];
        GLsizei len;
        GLint   size;
        GLenum  type;
        GLint   location;
    };

    GLuint p_program;
    GLuint p_vao;

    std::vector<Attrib> p_attribs;
};

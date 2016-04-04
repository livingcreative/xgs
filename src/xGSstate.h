#pragma once

#include "xGS/xGS.h"
#include "GL/glew.h"
#include "xGSobject.h"
#include <vector>
#include <map>
#include <string>


class xGSdatabufferImpl;
class xGStextureImpl;


class xGSstateImpl : public xGSobjectImpl<xGSstate>
{
public:
    xGSstateImpl(xGSimpl *owner);
    ~xGSstateImpl() override;

    bool Allocate(const GSstatedesc &desc);

    void Apply();

private:
    void EnumProgramInputs();
    void EnumProgramUniforms();
    void EnumProgramUniformBlocks();

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

    struct Block
    {
        char               name[256];
        GLint              size;
        xGSdatabufferImpl *buffer;
        unsigned int       offset;
    };

    GLuint p_program;
    GLuint p_vao;

    std::vector<Attrib> p_attribs;
    std::vector<Sampler> p_samplers;
    std::vector<Block> p_blocks;
    std::map<std::string, size_t> p_samplermap; // TODO: temprorary solution
    std::map<std::string, size_t> p_blockmap; // TODO: temprorary solution
};

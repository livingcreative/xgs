#pragma once

#include "xGS/xGS.h"
#include "GL/glew.h"


class xGStextureImpl : public xGStexture
{
public:
    xGStextureImpl();
    ~xGStextureImpl();

    void *Lock(GSlocktexture lock, unsigned int level, unsigned int layer, unsigned int flags) override;
    bool Unlock() override;

    bool Allocate(const GStexturedesc &desc);

private:
    void AllocateTextureImage(const GStexturedesc &desc);
    void ComputeLevelSize(GLuint level, GLuint &w, GLuint &h, GLuint &d);
    void DoUnlock();

private:
    GLuint        p_texture;
    GLenum        p_target;
    GLenum        p_intformat;
    GLenum        p_format;
    GLenum        p_type;
    GLuint        p_width;
    GLuint        p_height;
    GLuint        p_depth;

    GLuint        p_lockbuffer;
    GLuint        p_locktarget;
    GLuint        p_locklevel;
    GLuint        p_locklayer;
    GSlocktexture p_lock;
};

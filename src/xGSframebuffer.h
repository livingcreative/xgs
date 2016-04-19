#pragma once

#include "xGS/xGS.h"
#include "GL/glew.h"
#include "xGSobject.h"


class xGSframebufferImpl : public xGSobjectImpl<xGSframebuffer>
{
public:    xGSframebufferImpl(xGSimpl *owner);
    ~xGSframebufferImpl() override;

    bool Allocate(const GSframebufferdesc &desc);

public:
    unsigned int width() const { return p_width; }
    unsigned int height() const { return p_height; }

    void Bind();
    void Unbind();

private:
    GLuint       p_fbo;
    unsigned int p_width;
    unsigned int p_height;

    GLuint       p_depthrb;
};
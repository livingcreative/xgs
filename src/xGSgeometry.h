#pragma once

#include "xGS/xGS.h"
#include "GL/glew.h"
#include "xGSobject.h"


class xGSgeometrybufferImpl;


class xGSgeometryImpl : public xGSobjectImpl<xGSgeometry>
{
public:
    xGSgeometryImpl(xGSimpl *owner);
    ~xGSgeometryImpl() override;

    bool Allocate(const GSgeometrydesc &desc);

public:
    xGSgeometrybufferImpl *buffer() const { return p_buffer; }
    GLenum primtype() const { return p_primtype; }
    GLuint vertexcount() const { return p_vertexcount; }
    GLuint indexcount() const { return p_indexcount; }
    GLuint basevertex() const { return p_basevertex; }
    GLuint baseindex() const { return p_baseindex; }
    GLuint patchvertices() const { return p_patchvertices; }

    void Setup();

private:
    xGSgeometrybufferImpl *p_buffer;
    GLenum                 p_primtype;
    GLuint                 p_vertexcount;
    GLuint                 p_indexcount;
    GLuint                 p_basevertex;
    GLuint                 p_baseindex;
    GLuint                 p_patchvertices;
};

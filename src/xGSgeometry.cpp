#include "xGSgeometry.h"
#include "xGSgeometrybuffer.h"


xGSgeometryImpl::xGSgeometryImpl() :
    p_buffer(nullptr),
    p_primtype(0),
    p_vertexcount(0),
    p_indexcount(0),
    p_basevertex(0),
    p_baseindex(0)
{}

xGSgeometryImpl::~xGSgeometryImpl()
{}

bool xGSgeometryImpl::Allocate(const GSgeometrydesc &desc)
{
    // TODO: check for correct values passed in desc

    p_buffer = static_cast<xGSgeometrybufferImpl*>(desc.buffer);

    switch (desc.type) {
        case GS_PRIM_LINES: p_primtype = GL_LINES; break;
        case GS_PRIM_LINESTRIP: p_primtype = GL_LINE_STRIP; break;
        case GS_PRIM_TRIANGLES: p_primtype = GL_TRIANGLES; break;
        case GS_PRIM_TRIANGLESTRIP: p_primtype = GL_TRIANGLE_STRIP; break;
        case GS_PRIM_TRIANGLEFAN: p_primtype = GL_TRIANGLE_FAN; break;
    }

    // TODO: check for errors here
    p_buffer->AllocateGeometry(desc.vertexcount, desc.indexcount, p_basevertex, p_baseindex);

    p_vertexcount = desc.vertexcount;
    p_indexcount = desc.indexcount;

    return true;
}

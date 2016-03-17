#pragma once

#include "xGS/xGS.h"
#include "GL/glew.h"


class xGSgeometrybufferImpl : public xGSgeometrybuffer
{
public:
    xGSgeometrybufferImpl();
    ~xGSgeometrybufferImpl();

    // xGSgeometrybuffer interface implementation
    void *Lock(GSlockbuffer buffer, unsigned int flags) override;
    bool Unlock() override;

public:
    bool Allocate(const GSgeometrybufferdesc &desc);
    bool AllocateGeometry(unsigned int vertexcount, unsigned int indexcount, /* out */ unsigned int &basevertex, /* out */ unsigned int &baseindex);
    void DoUnlock();

    GLuint vertexBufferId() const { return p_vertexbuffer; }
    GLuint indexBufferId() const { return p_indexbuffer; }

    GSindexformat indexformat() const { return p_indexformat; }

private:
    enum LockedBuffer
    {
        NONE,
        VERTEX,
        INDEX
    };

    GLuint        p_vertexbuffer;
    GLuint        p_indexbuffer;
    LockedBuffer  p_locked;

    GSindexformat p_indexformat;
    unsigned int  p_vertexcount;
    unsigned int  p_indexcount;
    unsigned int  p_currentvertex;
    unsigned int  p_currentindex;
};

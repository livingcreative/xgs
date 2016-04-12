#pragma once

#include "xGS/xGS.h"
#include "xGSobject.h"
#include "GL/glew.h"
#include <vector>


class xGSdatabufferImpl : public xGSobjectImpl<xGSdatabuffer>
{
public:
    xGSdatabufferImpl(xGSimpl *owner);
    ~xGSdatabufferImpl() override;

    bool Update(unsigned int slot, unsigned int parameter, unsigned int count, const void *data) override;

    void *Lock(unsigned int offset, unsigned int size, unsigned int flags) override;
    bool Unlock() override;

public:
    bool Allocate(const GSdatabufferdesc &desc);

    GLuint bufferId() const { return p_buffer; }
    GLuint blockOffset(unsigned int index) const { return p_blocks[index].offset; }

private:
    struct Block
    {
        unsigned int offset;
        unsigned int size;
    };

    GLuint             p_buffer;
    GLuint             p_size;
    std::vector<Block> p_blocks;
};

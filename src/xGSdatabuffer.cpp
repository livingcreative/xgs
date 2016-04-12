#include "xGSdatabuffer.h"
#include "xGSutil.h"


xGSdatabufferImpl::xGSdatabufferImpl(xGSimpl *owner) :
    xGSobjectImpl(owner),
    p_buffer(0)
{}

xGSdatabufferImpl::~xGSdatabufferImpl()
{
    if (p_buffer) {
        glDeleteBuffers(1, &p_buffer);
    }
}

bool xGSdatabufferImpl::Update(unsigned int slot, unsigned int parameter, unsigned int count, const void *data)
{
    // TODO: implement update

    if (slot >= p_blocks.size()) {
        // TODO: report error
        return false;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, p_buffer);
    glBufferSubData(GL_UNIFORM_BUFFER, p_blocks[slot].offset, p_blocks[slot].size, data);

    return true;
}

void *xGSdatabufferImpl::Lock(unsigned int offset, unsigned int size, unsigned int flags)
{
    GLbitfield access = 0;

    if (flags & GS_READ) {
        access |= GL_MAP_READ_BIT;
    }
    if (flags & GS_WRITE) {
        access |= GL_MAP_WRITE_BIT;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, p_buffer);
    return glMapBufferRange(GL_UNIFORM_BUFFER, offset, size, access);
}

bool xGSdatabufferImpl::Unlock()
{
    // TODO: add locked state
    glBindBuffer(GL_UNIFORM_BUFFER, p_buffer);
    glUnmapBuffer(GL_UNIFORM_BUFFER);

    return true;
}

bool xGSdatabufferImpl::Allocate(const GSdatabufferdesc &desc)
{
    p_size = 0;

    const GSuniformblock *block = desc.blocks;
    while (block->uniforms) {
        const GSuniform *uniform = block->uniforms;
        GLuint blocksize = 0;
        while (uniform->type != GS_LAST_COMPONENT) {
            blocksize += vertexcomponentsize(uniform->type) * uniform->count;
            ++uniform;
        }

        Block b = {
            p_size,
            blocksize * block->count
        };
        p_blocks.emplace_back(b);
        p_size += align(GLint(blocksize * block->count), p_owner->caps().uboalignment);

        ++block;
    }

    glGenBuffers(1, &p_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, p_buffer);

    // TODO: use buffer storage if possible
    // TODO: account for flags and buffer storage type
    glBufferData(GL_UNIFORM_BUFFER, p_size, nullptr, GL_DYNAMIC_DRAW);

    return true;
}

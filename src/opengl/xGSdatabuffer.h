/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    opengl/xGSdatabuffer.h
        DataBuffer object implementation class header
            this object wraps buffer for holding uniform data which is used
            by shaders in state object
*/

#pragma once

#include "xGSobject.h"
#include <vector>


namespace xGS
{

    // data buffer object
    class xGSDataBufferImpl : public xGSObjectImpl<xGSDataBuffer, xGSDataBufferImpl>
    {
    public:
        xGSDataBufferImpl(xGSImpl *owner);
        ~xGSDataBufferImpl() override;

    public:
        GSvalue xGSAPI GetValue(GSenum valuetype) override;

        GSbool xGSAPI Update(GSuint offset, GSuint size, const GSptr data) override;
        GSbool xGSAPI UpdateBlock(GSuint block, GSuint index, const GSptr data) override;
        GSbool xGSAPI UpdateValue(GSuint block, GSuint index, GSuint uniform, GSuint uniformindex, GSuint count, const GSptr data) override;

        GSptr  xGSAPI Lock(GSdword access, void *lockdata) override;
        GSbool xGSAPI Unlock() override;

    public:
        struct UniformBlock
        {
            GSuint offset;             // offset of the 1st block inside buffer
            GSuint size;               // aligned size of one block
            GSuint actualsize;         // actual block size
            GSuint count;              // number of consecutive blocks of this type, starting from offset
            GSuint firstuniform;       // first uniform index in uniforms array
            GSuint onepastlastuniform; // one past last uniform index in uniforms array
        };

    public:
        GSbool allocate(const GSdatabufferdescription &desc);

        GLuint getID() const { return p_buffer; }
        GLenum target() const { return p_target; }

        GSuint blockCount() const { return GSuint(p_blocks.size()); }
        const UniformBlock& block(size_t index) const { return p_blocks[index]; }

        void ReleaseRendererResources();

    private:
        struct Uniform
        {
            GSenum type;      // element type
            GSuint offset;    // offset inside block
            GSuint size;      // actual element size (one element size, single field or one array element size)
            GSuint stride;    // array stride, if count > 1
            GSuint totalsize; // total array size, accounting for strides
            GSuint count;     // array count
        };

        typedef std::vector<UniformBlock> UniformBlockList;
        typedef std::vector<Uniform> UniformList;

    private:
        GLuint           p_buffer;
        GLenum           p_target;
        GSuint           p_size;

        UniformBlockList p_blocks;
        UniformList      p_uniforms;

        GSenum           p_locktype;
    };

} // namespace xGS

/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    xGSdatabufferbase.h
        DataBuffer object implementation base class header
*/

#pragma once

#include <vector>


namespace xGS
{

    // data buffer object base class
    class xGSDataBufferBase : public xGSDataBuffer
    {
    public:
        xGSDataBufferBase();

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

        GSuint blockCount() const { return GSuint(p_blocks.size()); }
        const UniformBlock& block(size_t index) const { return p_blocks[index]; }

    protected:
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

    protected:
        GSuint           p_size;

        UniformBlockList p_blocks;
        UniformList      p_uniforms;

        GSenum           p_locktype;
    };

} // namespace xGS

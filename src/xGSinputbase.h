/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    xGSinputbase.h
        Input object implementation base class header
*/

#pragma once

#include <vector>


namespace xGS
{

    class xGSStateImpl;
    class xGSGeometryImpl;
    class xGSGeometryBufferImpl;


    // input object
    class xGSInputBase : public xGSInput
    {
    public:
        xGSInputBase();

    public:
        xGSStateImpl* state() const { return p_state; }
        GSuint bufferCount() const { return GSuint(p_buffers.size()); }
        xGSGeometryBufferImpl* buffer(GSuint index) const { return p_buffers[index]; }

        xGSGeometryBufferImpl* primaryBuffer() const { return p_primarybuffer; }

    protected:
        typedef std::vector<xGSGeometryBufferImpl*> GeometryBufferList;

    protected:
        xGSStateImpl          *p_state;
        xGSGeometryBufferImpl *p_primarybuffer;
        GeometryBufferList     p_buffers;
    };

} // namespace xGS

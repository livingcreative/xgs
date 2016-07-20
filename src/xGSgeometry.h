/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    xGSgeometry.h
        Geometry object implementation class header
            this object defines parameters of static chunk of geometry (mesh)
            primitive type, vertex and index count and source buffer

            it is common for any implementation - xGS API object
*/

#pragma once

#include "xGSobject.h"


namespace xGS
{

    // geometry object
    class xGSGeometryImpl : public xGSObjectImpl<xGSGeometry, xGSGeometryImpl>
    {
    public:
        xGSGeometryImpl(xGSImpl *owner);
        ~xGSGeometryImpl() override;

    // IxGSGeometry interface
    public:
        GSvalue xGSAPI GetValue(GSenum valuetype) override;

        GSptr   xGSAPI Lock(GSenum locktype, GSdword access, void *lockdata) override;
        GSbool  xGSAPI Unlock() override;

    // internal public interface
    public:
        GSbool allocate(const GSgeometrydescription &desc);

        GSenum                 type() const { return p_type; }
        GSuint                 patchvertices() const { return p_patch_vertices; }
        GSbool                 restart() const { return p_restart; }
        GSuint                 restartindex() const { return p_restartindex; }
        GSenum                 indexFormat() const { return p_indexformat; }
        GSint                  vertexCount() const { return p_vertexcount; }
        GSint                  indexCount() const { return p_indexcount; }
        GSuint                 patchVertices() const { return p_patch_vertices; }
        const GSptr            vertexPtr() const { return p_vertexmemory; }
        const GSptr            indexPtr() const { return p_indexmemory; }
        xGSGeometryBufferImpl* buffer() const { return p_buffer; }
        GSuint                 baseVertex() const { return p_basevertex; }

        void ReleaseRendererResources()
        {
            // nothing to release
            // TODO: check for shared geometry refcount here, does it need to be decremented?
        }

    // internal functions
    protected:
        bool checkAlloc(GSenum indexformat/*, GSenum sharemode*/);
        void doUnlock();

    private:
        GSenum                 p_type;         // primitive type
        GSenum                 p_indexformat;  // index format
        GSint                  p_vertexcount;
        GSint                  p_indexcount;
        GSuint                 p_patch_vertices;
        GSbool                 p_restart;
        GSuint                 p_restartindex;

        GSenum                 p_locktype;     // lock type (none, vertices, indices)
        GSptr                  p_lockpointer;  // current lock ptr (nullptr if there's no lock)

        GSuint                 p_basevertex;   // base vertex in buffer
        GSptr                  p_vertexmemory; // starting vertex pointer (offset inside buffer or own memory)
        GSptr                  p_indexmemory;  // starting index pointer (offset inside buffer or own memory)

        xGSGeometryImpl       *p_sharedgeometry;
        xGSGeometryBufferImpl *p_buffer;       // buffer in which geometry is allocated
    };

} // namespace xGS

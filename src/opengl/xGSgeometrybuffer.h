/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    opengl/xGSgeometrybuffer.h
        GeometryBuffer object implementation class header
            this object wraps buffer objects for storing geometry (mesh) data
            and format for vertex and index data
*/

#pragma once

#include "xGSobject.h"


namespace xGS
{

    // geometry buffer object
    class xGSGeometryBufferImpl : public xGSObjectImpl<xGSGeometryBuffer, xGSGeometryBufferImpl>
    {
    public:
        xGSGeometryBufferImpl(xGSImpl *owner);
        ~xGSGeometryBufferImpl() override;

    public:
        GSvalue xGSAPI GetValue(GSenum valuetype) override;

        GSptr   xGSAPI Lock(GSenum locktype, GSdword access, void *lockdata) override;
        GSbool  xGSAPI Unlock() override;

    public:
        struct Primitive
        {
            GLenum type;
            GSuint firstvertex;
            GSuint vertexcount;
            GSuint firstindex;
            GSuint indexcount;
        };

    public:
        GSbool allocate(const GSgeometrybufferdescription &desc);

        GSenum type() const { return p_type; }

        GLuint getVertexBufferID() const { return p_vertexbuffer; }
        GLuint getIndexBufferID() const { return p_indexbuffer; }
        const GSvertexdecl& vertexDecl() const { return p_vertexdecl; }
        GSenum indexFormat() const { return p_indexformat; }

        GSbool allocateGeometry(GSuint vertexcount, GSuint indexcount, GSptr &vertexmemory, GSptr &indexmemory, GSuint &basevertex);
        void freeGeometry(GSuint vertexcount, GSuint indexcount, GSptr vertexmemory, GSptr indexmemory);

        GSptr lock(GSenum locktype, size_t offset, size_t size);
        void unlock();

        void BeginImmediateDrawing();
        bool EmitPrimitive(GSenum type, GSuint vertexcount, GSuint indexcount, GSuint flags, GSimmediateprimitive *primitive);
        void EndImmediateDrawing();

        size_t immediateCount() const { return p_primitives.size(); }
        const Primitive& immediatePrimitive(size_t index) const { return p_primitives[index]; }

        void ReleaseRendererResources();

    private:
        enum
        {
            LOCK_IMMEDIATE = 0x1000
        };

        typedef std::vector<Primitive> PrimitiveList;

        struct FreeBlock
        {
            GSptr  memory;
            GSuint count;
        };

        typedef std::vector<FreeBlock> FreeBlockList;

        bool allocateBlock(GSuint count, GSuint elementsize, GSuint maxcount, GSptr &memory, GSuint &current, FreeBlockList &list, size_t &block);
        void commitFreeBlock(size_t block, FreeBlockList &list);
        void freeBlock(GSptr memory, GSuint count, GSuint elementsize, GSuint &current, FreeBlockList &list);

    private:
        GSenum        p_type;
        GSvertexdecl  p_vertexdecl;
        GSenum        p_indexformat;
        GLuint        p_vertexbuffer;
        GLuint        p_indexbuffer;
        GSuint        p_vertexcount;
        GSuint        p_indexcount;
        GSuint        p_currentvertex;
        GSuint        p_currentindex;

        GSuint        p_vertexgranularity;
        GSuint        p_indexgranularity;
        FreeBlockList p_freevertices;
        FreeBlockList p_freeindices;

        GSenum        p_locktype;

        // immediate cache data
        PrimitiveList p_primitives;
        GSptr         p_vertexptr;
        GSptr         p_indexptr;
    };

} // namespace xGS

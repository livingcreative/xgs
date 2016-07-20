/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    xGSgeometrybufferbase.h
        GeometryBuffer object implementation base class header
*/

#pragma once

#include "xGSutil.h"


namespace xGS
{

    class xGSGeometryBufferBase : public xGSGeometryBuffer
    {
    public:
        xGSGeometryBufferBase();

    public:
        struct Primitive
        {
            GSenum type;
            GSuint firstvertex;
            GSuint vertexcount;
            GSuint firstindex;
            GSuint indexcount;
        };

        GSenum type() const { return p_type; }

        const GSvertexdecl& vertexDecl() const { return p_vertexdecl; }
        GSenum indexFormat() const { return p_indexformat; }

        size_t immediateCount() const { return p_primitives.size(); }
        const Primitive& immediatePrimitive(size_t index) const { return p_primitives[index]; }

        bool EmitPrimitive(GSenum type, GSuint vertexcount, GSuint indexcount, GSuint flags, GSimmediateprimitive *primitive);

        GSbool allocateGeometry(GSuint vertexcount, GSuint indexcount, GSptr &vertexmemory, GSptr &indexmemory, GSuint &basevertex);
        void freeGeometry(GSuint vertexcount, GSuint indexcount, GSptr vertexmemory, GSptr indexmemory);

    protected:
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

    protected:
        GSenum        p_type;
        GSvertexdecl  p_vertexdecl;
        GSenum        p_indexformat;
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

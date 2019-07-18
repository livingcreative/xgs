/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    xGSimplbase.h
        xGS API object common implementation class header
            defines main API (system) object common structure
*/

#pragma once

#include "xGS/xGS.h"
#include "xGSutil.h"
#include "IUnknownImpl.h"
#include <vector>
#include <unordered_set>


namespace xGS
{

    class xGSGeometryBufferImpl;
    class xGSFrameBufferImpl;
    class xGSInputImpl;
    class xGSTextureImpl;
    class xGSStateImpl;
    class xGSParametersImpl;

    class IxGSGeometryImpl;
    class IxGSGeometryBufferImpl;
    class IxGSDataBufferImpl;
    class IxGSTextureImpl;
    class IxGSFrameBufferImpl;
    class IxGSStateImpl;
    class IxGSInputImpl;
    class IxGSParametersImpl;


    class xGSImplBase : public xGSIUnknownImpl<xGSSystem>
    {
    public:
        xGSImplBase();
        ~xGSImplBase() override;

    public:
        // TODO: make GScaps common for all platforms
        //          split caps into two parts - xGS common caps, exposed by API
        //          and implementation specific caps
        // TODO: think of samplers and how to make common interface for all platforms
        //          if it's actually needed (it might be platform specific though)

        // memory management
        GSptr allocate(GSuint size);
        void free(GSptr &memory);

        // error set-up
        bool error(GSerror code, DebugMessageLevel level = DebugMessageLevel::Error);

        // debug logging
        void debug(DebugMessageLevel level, const char *format, ...);

        // children objects add/remove
        template <typename T> void AddObject(T *object);
        template <typename T> void RemoveObject(T *object);

    protected:
        enum SystemState
        {
            SYSTEM_NOTREADY,
            SYSTEM_READY,
            RENDERER_READY,
            CAPTURE
        };

        typedef std::unordered_set<IxGSGeometryImpl*>       GeometryList;
        typedef std::unordered_set<IxGSGeometryBufferImpl*> GeometryBufferList;
        typedef std::unordered_set<IxGSDataBufferImpl*>     DataBufferList;
        typedef std::unordered_set<IxGSTextureImpl*>        TextureList;
        typedef std::unordered_set<IxGSFrameBufferImpl*>    FrameBufferList;
        typedef std::unordered_set<IxGSStateImpl*>          StateList;
        typedef std::unordered_set<IxGSInputImpl*>          InputList;
        typedef std::unordered_set<IxGSParametersImpl*>     ParametersList;

        static IxGS            gs;

        SystemState            p_systemstate;
        GSerror                p_error;

        GeometryList           p_geometrylist;
        GeometryBufferList     p_geometrybufferlist;
        DataBufferList         p_databufferlist;
        TextureList            p_texturelist;
        FrameBufferList        p_framebufferlist;
        StateList              p_statelist;
        InputList              p_inputlist;
        ParametersList         p_parameterslist;

        xGSFrameBufferImpl    *p_rendertarget;
        GSenum                 p_colorformats[GS_MAX_FB_COLORTARGETS];
        GSenum                 p_depthstencilformat;

        xGSStateImpl          *p_state;
        xGSInputImpl          *p_input;
        xGSParametersImpl     *p_parameters[GS_MAX_PARAMETER_SETS];

        xGSGeometryBufferImpl *p_capturebuffer;

        xGSGeometryBufferImpl *p_immediatebuffer;

#ifdef _DEBUG
        GSuint                 dbg_memory_allocs;
#endif
    };

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

    // framebuffer object
    class xGSFrameBufferBase : public xGSFrameBuffer
    {
    public:
        xGSFrameBufferBase();

    public:
        GSsize size() const;

        void getformats(GSenum *colorformats, GSenum &depthstencilformat) const;
        bool srgb() const { return p_srgb; }

    protected:
        struct Attachment
        {
            Attachment() :
                p_texture(nullptr),
                p_level(0),
                p_slice(0)
            {}

            ~Attachment();

            void attach(xGSTextureImpl *texture, GSuint level, GSuint slice);

            xGSTextureImpl *p_texture;
            GSuint          p_level;
            GSuint          p_slice;
        };

    protected:
        GSuint     p_width;
        GSuint     p_height;
        GSuint     p_colortargets;
        GSuint     p_activecolortargets;
        GSenum     p_colorformat[GS_MAX_FB_COLORTARGETS];
        GSenum     p_depthstencilformat;
        GSenum     p_multisample;
        bool       p_srgb;

        Attachment p_colortextures[GS_MAX_FB_COLORTARGETS];
        Attachment p_depthtexture;
    };

    // geometry buffer object
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

    // texture object
    class xGSTextureBase : public xGSTexture
    {
    public:
        xGSTextureBase();

    public:
        GSuint samples() const { return p_multisample; }
        GSenum format() const { return p_format; }
#ifdef _DEBUG
        bool boundAsRT() const { return p_boundasrt != 0; }
        void bindAsRT() { ++p_boundasrt; }
        void unbindFromRT() { --p_boundasrt; }
#endif

    protected:
        GSenum  p_texturetype;  // texture type: 1D, 2D, etc...
        GSenum  p_format;       // texture texel format

        GSuint  p_width;        // width
        GSuint  p_height;       // height
        GSuint  p_depth;        // depth (3D texture only)
        GSuint  p_layers;       // array layers
        GSuint  p_multisample;  // multisample sample count
        GSuint  p_minlevel;     // minimum (base) defined MIP level
        GSuint  p_maxlevel;     // maximum defined MIP level

        GSenum  p_locktype;     // LOCK: locked texture part
        GSdword p_lockaccess;   // LOCK: lock access
        GSint   p_locklayer;    // LOCK: locked texture layer (for array textures only)
        GSint   p_locklevel;    // LOCK: locked texture MIP level

#ifdef _DEBUG
        GSuint  p_boundasrt;    // texture currently is render target
#endif
    };

    // state object
    class xGSStateBase : public xGSState
    {
    public:
        xGSStateBase();

    public:
        struct InputSlot
        {
            InputSlot()
            {}

            InputSlot(const GSvertexcomponent *_decl, GSuint _divisor) :
                decl(_decl),
                buffer(nullptr),
                divisor(_divisor)
            {}

            GSvertexdecl           decl;
            xGSGeometryBufferImpl *buffer;
            GSuint                 divisor;
        };

        struct ParameterSlot
        {
            GSenum type;     // texture slot, uniform constant, uniform block
            GSint  location; // block index or location
            GSuint index;    // array index for array uniforms
        };

    public:
        GSuint inputCount() const { return GSuint(p_input.size()); }
        GSuint inputAvailable() const { return p_inputavail; }
        size_t inputPrimarySlot() const { return p_primaryslot; }
        const InputSlot& input(size_t index) const { return p_input[index]; }

        GSuint parameterSetCount() const { return GSuint(p_parametersets.size()); }
        const GSParameterSet& parameterSet(GSuint index) const { return p_parametersets[index]; }
        const ParameterSlot& parameterSlot(GSuint index) const { return p_parameterslots[index]; }

        bool validate(const GSenum *colorformats, GSenum depthstencilformat);

    protected:
        typedef std::vector<InputSlot> InputSlotList;
        typedef std::vector<GSParameterSet> ParamSetList;
        typedef std::vector<ParameterSlot> ParamSlotList;

    protected:
        InputSlotList     p_input;
        size_t            p_primaryslot;
        GSuint            p_inputavail;

        ParamSetList      p_parametersets;
        ParamSlotList     p_parameterslots;

        // output RT formats
        GSenum            p_colorformats[GS_MAX_FB_COLORTARGETS];
        GSenum            p_depthstencilformat;

        // some fixed state common params
        GSbool            p_rasterizerdiscard;
    };

    // parameters object
    class xGSParametersBase : public xGSParameters
    {
    public:
        xGSParametersBase();

    public:
        xGSStateImpl* state() const { return p_state; }
        GSuint setindex() const { return p_setindex; }

    protected:
        xGSStateImpl *p_state;
        GSuint        p_setindex;
    };


    // generic object base
    template <typename T, typename implT>
    class xGSObjectBase : public xGSIUnknownImpl<T>
    {
    public:
        typedef implT impl_t;

        xGSObjectBase() :
            p_objecttype(GS_NONE),
            p_owner(nullptr)
        {}

        xGSObjectBase(implT *owner) :
            p_objecttype(GS_NONE),
            p_owner(owner)
        {}

        ~xGSObjectBase() override
        {}

        void DetachFromRenderer()
        {
            p_owner = nullptr;
        }

        GSenum objecttype() const { return p_objecttype; }

    protected:
        GSenum  p_objecttype;
        implT  *p_owner;
    };

    // generic object implementation
    template <typename baseT, typename selfT>
    class xGSObjectImpl : public baseT
    {
    public:
        xGSObjectImpl(typename baseT::impl_t *owner) :
            baseT(owner)
        {
            p_owner = owner;
            p_owner->AddObject(static_cast<selfT*>(this));
        }

        ~xGSObjectImpl() override
        {
            if (p_owner) {
                p_owner->RemoveObject(static_cast<selfT*>(this));
            }
        }

        // TODO: is it possible to tie type as template static parameter?
        static selfT* create(typename baseT::impl_t *owner, GSenum type)
        {
            selfT *result = new selfT(owner);
            result->p_objecttype = type;
            result->AddRef();
            return result;
        }
    };


    // base (unknown) object implementation
    class xGSUnknownObjectImpl : public xGSObjectImpl<xGSObjectBase<xGSObject, xGSBase>, xGSUnknownObjectImpl>
    {};

    // geometry object implementation
    class IxGSGeometryImpl : public xGSObjectImpl<xGSObjectBase<xGSGeometry, xGSBase>, IxGSGeometryImpl>
    {
    public:
        IxGSGeometryImpl(xGSBase *owner);
        ~IxGSGeometryImpl() override;

        // IxGSGeometry interface
    public:
        GSvalue xGSAPI GetValue(GSenum valuetype) override;

        GSptr   xGSAPI Lock(GSenum locktype, GSdword access, void *lockdata) override;
        GSbool  xGSAPI Unlock() override;

        // internal public interface
    public:
        GSbool allocate(const GSgeometrydescription &desc);

        GSenum                  type() const { return p_type; }
        GSuint                  patchvertices() const { return p_patch_vertices; }
        GSbool                  restart() const { return p_restart; }
        GSuint                  restartindex() const { return p_restartindex; }
        GSenum                  indexFormat() const { return p_indexformat; }
        GSint                   vertexCount() const { return p_vertexcount; }
        GSint                   indexCount() const { return p_indexcount; }
        GSuint                  patchVertices() const { return p_patch_vertices; }
        const GSptr             vertexPtr() const { return p_vertexmemory; }
        const GSptr             indexPtr() const { return p_indexmemory; }
        xGSGeometryBufferImpl*  buffer() const { return p_buffer; }
        GSuint                  baseVertex() const { return p_basevertex; }

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
        GSenum                  p_type;         // primitive type
        GSenum                  p_indexformat;  // index format
        GSint                   p_vertexcount;
        GSint                   p_indexcount;
        GSuint                  p_patch_vertices;
        GSbool                  p_restart;
        GSuint                  p_restartindex;

        GSenum                  p_locktype;     // lock type (none, vertices, indices)
        GSptr                   p_lockpointer;  // current lock ptr (nullptr if there's no lock)

        GSuint                  p_basevertex;   // base vertex in buffer
        GSptr                   p_vertexmemory; // starting vertex pointer (offset inside buffer or own memory)
        GSptr                   p_indexmemory;  // starting index pointer (offset inside buffer or own memory)

        IxGSGeometryImpl       *p_sharedgeometry;
        xGSGeometryBufferImpl  *p_buffer;       // buffer in which geometry is allocated
    };

} // namespace xGS

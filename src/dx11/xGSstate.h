/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    dx11/xGSstate.h
        State object implementation class header
            state object holds most of the 3D rendering pipeline parameters
            and shaders for each pipeline stage
*/

#pragma once

#include "xGS/xGS.h"
#include "xGSobject.h"
#include "xGSparameters.h"
#include "xGSutil.h"
#include <string>
#include <vector>
#include <unordered_map>


struct ID3D11InputLayout;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11ShaderReflection;
struct _D3D11_SHADER_DESC;


namespace xGS
{

    // program object
    class xGSStateImpl : public xGSObjectImpl<xGSObjectBase<xGSState>, xGSStateImpl>
    {
    public:
        xGSStateImpl(xGSImpl *owner);
        ~xGSStateImpl() override;

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
        GSbool allocate(const GSstatedescription &desc);

        // TODO: move vertex array declarations from GSvertexdecl here
        int attribLocation(const char *name) const;

        GSuint inputCount() const { return GSuint(p_input.size()); }
        GSuint inputAvailable() const { return p_inputavail; }
        size_t inputPrimarySlot() const { return p_primaryslot; }
        const InputSlot& input(size_t index) const { return p_input[index]; }

        GSuint parameterSetCount() const { return GSuint(p_parametersets.size()); }
        const GSParameterSet& parameterSet(GSuint index) const { return p_parametersets[index]; }
        const ParameterSlot& parameterSlot(GSuint index) const { return p_parameterslots[index]; }

        bool depthMask() const { return p_depthmask; }

        bool validate(const GSenum *colorformats, GSenum depthstencilformat);

        void apply(const GScaps &caps);

        static void bindNullProgram();

        void ReleaseRendererResources();

    private:
        typedef std::unordered_map<std::string, GSint> ElementIndexMap;

        GSbool uniformIsSampler(int type) const;

        void EnumAttributes(const _D3D11_SHADER_DESC &desc, ID3D11ShaderReflection *vsr);
        void EnumUniforms(const _D3D11_SHADER_DESC &desc, ID3D11ShaderReflection *sr);
        void EnumUniformBlocks(const _D3D11_SHADER_DESC &desc, ID3D11ShaderReflection *sr);

        template<typename T>
        void CreateElementIndex(ElementIndexMap &map, const T &elementlist) const;

    private:
        struct Attribute
        {
            GSuint      size;
            std::string name;
        };

        struct Uniform
        {
            GSuint      size;         // array size - for array uniforms
            GSuint      offset;       // offet inside block
            GSuint      arraystride;
            GSuint      matrixstride;
            GSuint      blockindex;
            GSbool      sampler;
            std::string name;
        };

        struct UniformBlock
        {
            GSuint      datasize; // block data size
            std::string name;
        };

        typedef std::vector<Attribute> AttributeList;
        typedef std::vector<Uniform> UniformList;
        typedef std::vector<UniformBlock> UniformBlockList;
        typedef std::vector<std::string> StringList;

        typedef std::vector<InputSlot> InputSlotList;
        typedef std::vector<GSParameterSet> ParamSetList;
        typedef std::vector<ParameterSlot> ParamSlotList;

    private:
        ID3D11InputLayout  *p_inputlayout;

        ID3D11VertexShader *p_vs;
        ID3D11PixelShader  *p_ps;

        AttributeList       p_attributes;
        UniformList         p_uniforms;
        UniformBlockList    p_uniformblocks;

        InputSlotList       p_input;
        size_t              p_primaryslot;
        GSuint              p_inputavail;

        ParamSetList        p_parametersets;
        ParamSlotList       p_parameterslots;

        StringList          p_feedback;

        // static params set
        GSParametersState   p_staticstate;

        // fixed state params
        GSbool              p_rasterizerdiscard;
        GSbool              p_sampleshading;
        int                 p_fill;
        bool                p_cull;
        float               p_pointsize;
        bool                p_programpointsize;
        int                 p_cullface;
        bool                p_colormask;
        bool                p_depthmask;
        bool                p_depthtest;
        int                 p_depthfunc;
        bool                p_blendseparate;
        bool                p_blend[GS_MAX_FB_COLORTARGETS];
        int                 p_blendeq[GS_MAX_FB_COLORTARGETS];
        int                 p_blendsrc[GS_MAX_FB_COLORTARGETS];
        int                 p_blenddst[GS_MAX_FB_COLORTARGETS];
        int                 p_blendeqalpha[GS_MAX_FB_COLORTARGETS];
        int                 p_blendsrcalpha[GS_MAX_FB_COLORTARGETS];
        int                 p_blenddstalpha[GS_MAX_FB_COLORTARGETS];
        GSuint              p_polygonoffset;
        bool                p_multisample;

        // output RT formats
        GSenum              p_colorformats[GS_MAX_FB_COLORTARGETS];
        GSenum              p_depthstencilformat;
    };

} // namespace xGS

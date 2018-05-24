/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/xGSstate.h
        State object implementation class header
            state object holds most of the 3D rendering pipeline parameters
            and shaders for each pipeline stage
*/

#pragma once

#include "xGSimplbase.h"
#include "xGSparameters.h"
#include "xGSutil.h"
#include "glplatform.h"
#include <string>


namespace xGS
{

    // program object
    class xGSStateImpl : public xGSObjectBase<xGSStateBase, xGSImpl>
    {
    public:
        xGSStateImpl(xGSImpl *owner);
        ~xGSStateImpl() override;

    public:
        GSbool AllocateImpl(const GSstatedescription &desc, GSuint staticinputslots, const GSparameterlayout *staticparams, GSuint staticset);

        // TODO: move vertex array declarations from GSvertexdecl here
        GLint attribLocation(const char *name) const;

        bool depthMask() const { return p_depthmask; }

        void setarrays(const GSvertexdecl &decl, GSuint divisor, GSptr vertexptr) const;
        void setformat(const GSvertexdecl &decl, GSuint binding, GSuint divisor) const;

        void apply(const GScaps &caps);

        static void bindNullProgram();

        void ReleaseRendererResources();

    private:
        typedef std::unordered_map<std::string, GSint> ElementIndexMap;

        GSbool uniformIsSampler(GLenum type) const;

        void AttachShaders(GLenum type, const char **source, std::vector<GLuint> &shaders);

        void EnumAttributes();
        void EnumUniforms();
        void EnumUniformBlocks();

        template<typename T>
        void CreateElementIndex(ElementIndexMap &map, const T &elementlist) const;

    private:
        struct Attribute
        {
            GLenum      type;
            GLint       location;
            GSuint      size;
            std::string name;
        };

        struct Uniform
        {
            GLenum      type;         // uniform type (GL) 
            GLint       location;     // location (index to pass into glUniform!)
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
            GLint       index;    // location
            GSuint      datasize; // block data size
            std::string name;
        };

        typedef std::vector<Attribute> AttributeList;
        typedef std::vector<GLuint> GLuintList;
        typedef std::vector<Uniform> UniformList;
        typedef std::vector<UniformBlock> UniformBlockList;
        typedef std::vector<std::string> StringList;

    private:
        GLuint            p_program;
        GLuint            p_vao;

        AttributeList     p_attributes;
        UniformList       p_uniforms;
        UniformBlockList  p_uniformblocks;

        StringList        p_feedback;

        // static params set
        GSParametersState p_staticstate;

        // fixed state params
        GSbool            p_sampleshading;
        GLenum            p_fill;
        bool              p_cull;
        float             p_pointsize;
        bool              p_programpointsize;
        GLenum            p_cullface;
        bool              p_colormask;
        bool              p_depthmask;
        bool              p_depthtest;
        GLenum            p_depthfunc;
        bool              p_blendseparate;
        bool              p_blend[GS_MAX_FB_COLORTARGETS];
        GLenum            p_blendeq[GS_MAX_FB_COLORTARGETS];
        GLenum            p_blendsrc[GS_MAX_FB_COLORTARGETS];
        GLenum            p_blenddst[GS_MAX_FB_COLORTARGETS];
        GLenum            p_blendeqalpha[GS_MAX_FB_COLORTARGETS];
        GLenum            p_blendsrcalpha[GS_MAX_FB_COLORTARGETS];
        GLenum            p_blenddstalpha[GS_MAX_FB_COLORTARGETS];
        GSuint            p_polygonoffset;
        bool              p_multisample;
    };

} // namespace xGS

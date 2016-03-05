/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    xGSstate.cpp
        State object implementation class
*/

#include "xGSstate.h"
#include "xGSgeometrybuffer.h"
#include "xGSdatabuffer.h"
#include "xGStexture.h"


using namespace xGS;
using namespace std;


xGSStateImpl::xGSStateImpl(xGSImpl *owner) :
    xGSObject(owner),
    p_program(0),
    p_vao(0),
    p_allocated(false),
    p_primaryslot(GS_UNDEFINED)
{
    p_owner->debug(DebugMessageLevel::Information, "State object created\n");
}

xGSStateImpl::~xGSStateImpl()
{
    ReleaseRendererResources();
    p_owner->debug(DebugMessageLevel::Information, "State object destroyed\n");
}

GSbool xGSStateImpl::allocate(const GSstatedescription &desc)
{
    if (!desc.inputlayout) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    if (!desc.parameterlayout) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    p_program = glCreateProgram();

    // bind input attribute locations & allocate stream slots
    GSuint staticinputslots = 0;
    p_input.clear();
    p_inputavail = 0;

    const GSinputlayout *inputlayout = desc.inputlayout;
    while (inputlayout->slottype != GSI_END) {
        // bind
        const GSvertexcomponent *comp = inputlayout->decl;
        while (comp->type != GSVD_END) {
            if (comp->name && comp->index != GS_DEFAULT) {
                glBindAttribLocation(p_program, comp->index, comp->name);
            }
            ++comp;
        }

        // allocate slot
        InputSlot slot(inputlayout->decl, inputlayout->divisor);
        if (inputlayout->divisor == 0) {
            // TODO: check for only one primary slot
            p_primaryslot = p_input.size();
        }

        switch (inputlayout->slottype) {
            case GSI_DYNAMIC:
                ++p_inputavail;
                break;

            case GSI_STATIC: {
                // assign static geometry buffer source
                if (inputlayout->buffer == nullptr) {
                    ReleaseRendererResources();
                    return p_owner->error(GSE_INVALIDOBJECT);
                }

                xGSGeometryBufferImpl *buffer =
                    static_cast<xGSGeometryBufferImpl*>(inputlayout->buffer);
                if (!buffer->allocated()) {
                    ReleaseRendererResources();
                    return p_owner->error(GSE_INVALIDOBJECT);
                }

                slot.buffer = buffer;
                buffer->AddRef();

                ++staticinputslots;

                break;
            }

            default:
                ReleaseRendererResources();
                return p_owner->error(GSE_INVALIDENUM);
        }

        p_input.push_back(slot);

        ++inputlayout;
    }

    // bind output locations and feedback
    if (desc.output) {
        const GSoutputlayout *outputlayout = desc.output;
        while (outputlayout->destination != GS_NONE) {
            switch (outputlayout->destination) {
                case GSD_FRAMEBUFFER:
                case GSD_FRAMEBUFFER_INDEXED:
                    if (outputlayout->name && outputlayout->location != GS_DEFAULT) {
                        glBindFragDataLocation(
                            p_program, outputlayout->location,
                            outputlayout->name
                        );
                    }
                    break;

                case GSD_FEEDBACK:
                    if (outputlayout->name) {
                        p_feedback.push_back(string(outputlayout->name));
                    }
                    break;
            }
            ++outputlayout;
        }
    }

    if (p_feedback.size()) {
        const char **feedback = new const char*[p_feedback.size()];
        size_t index = 0;
        for (auto &s : p_feedback) {
            feedback[index++] = s.c_str();
        }

        glTransformFeedbackVaryings(p_program, GLsizei(p_feedback.size()), feedback, GL_INTERLEAVED_ATTRIBS);

        delete[] feedback;
    }

    const GSpixelformat &fmt = p_owner->DefaultRenderTargetFormat();
    for (size_t n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        p_colorformats[n] =
            desc.colorformats[n] == GS_COLOR_DEFAULT ?
                ColorFormatFromPixelFormat(fmt) :
                desc.colorformats[n];
    }
    p_depthstencilformat = desc.depthstencilformat == GS_DEPTH_DEFAULT ?
        DepthFormatFromPixelFormat(fmt) :
        desc.depthstencilformat;

    // attach shader sources
    std::vector<GLuint> shaders;

    AttachShaders(GL_VERTEX_SHADER, desc.vs, shaders);
    AttachShaders(GL_FRAGMENT_SHADER, desc.ps, shaders);
    AttachShaders(GL_TESS_CONTROL_SHADER, desc.cs, shaders);
    AttachShaders(GL_TESS_EVALUATION_SHADER, desc.es, shaders);
    AttachShaders(GL_GEOMETRY_SHADER, desc.gs, shaders);

    for (auto s : shaders) {
        glAttachShader(p_program, s);
    }

    glLinkProgram(p_program);

    for (auto s : shaders) {
        glDetachShader(p_program, s);
        glDeleteShader(s);
    }

#ifdef _DEBUG
    {
        GLsizei loglen = 0;
        char infoLog[1024];
        glGetProgramInfoLog(p_program, 1024, &loglen, infoLog);
        infoLog[loglen] = 0;
        p_owner->debug(DebugMessageLevel::Information, "Program info log:\n%s\n\n", infoLog);
    }

    p_owner->debugTrackGLError("xGSStateImpl::Allocate");
#endif

    // TODO: check status
    EnumAttributes();
    EnumUniforms();
    EnumUniformBlocks();


    // gather parameters info
    p_parametersets.clear();
    p_parameterslots.clear();

    GSuint currenttextureslot = 0;

    const GSparameterlayout *paramset = desc.parameterlayout;
    while (paramset->settype != GSP_END) {
        GSParameterSet set = {
            paramset->settype,
            GSuint(p_parameterslots.size()),
            GSuint(p_parameterslots.size()),
            currenttextureslot,
            currenttextureslot,
            0
        };

        const GSparameterdecl *param = paramset->parameters;
        while (param->type != GSPD_END) {
            ParameterSlot slot = {
                param->type,
                GS_DEFAULT,
                param->index
            };

            switch (param->type) {
                case GSPD_CONSTANT:
                    slot.location = glGetUniformLocation(p_program, param->name);
                    if (slot.location == GS_DEFAULT) {
                        p_owner->debug(DebugMessageLevel::Warning, "Requested parameter \"%s\" not found in program parameters\n", param->name);
                    }
                    ++set.constantcount;
                    break;

                case GSPD_BLOCK:
                    slot.location = glGetUniformBlockIndex(p_program, param->name);
                    if (slot.location == GS_DEFAULT) {
                        p_owner->debug(DebugMessageLevel::Warning, "Requested parameter \"%s\" not found in program parameters\n", param->name);
                    }
                    break;

                case GSPD_TEXTURE: {
                    GSint location = glGetUniformLocation(p_program, param->name);
                    if (location == GS_DEFAULT) {
                        p_owner->debug(DebugMessageLevel::Warning, "Requested parameter \"%s\" not found in program parameters\n", param->name);
                    } else {
                        slot.location = currenttextureslot - set.firstsampler;
                        // TODO: be sure that glProgramUniform1i available
                        glProgramUniform1i(p_program, location + param->index, currenttextureslot);

                        p_owner->debug(
                            DebugMessageLevel::Information,
                            "Program texture slot \"%s\" with location %i got texture slot #%i\n",
                            param->name, location + param->index, currenttextureslot
                        );

                        ++currenttextureslot;
                        ++set.onepastlastsampler;
                    }
                    break;
                }

                default:
                    ReleaseRendererResources();
                    return p_owner->error(GSE_INVALIDENUM);
            }

            p_parameterslots.push_back(slot);

            ++set.onepastlast;

            ++param;
        }

        p_parametersets.push_back(set);

        if (paramset->settype == GSP_STATIC) {
            // bind static parameters
            p_staticstate.allocate(
                p_owner, this, set,
                paramset->uniforms, paramset->textures, nullptr
            );
        }

        ++paramset;
    }

    // allocate input streams
    bool need_vao = p_owner->caps().vertex_format || staticinputslots == p_input.size();
    if (need_vao) {
        glGenVertexArrays(1, &p_vao);
        glBindVertexArray(p_vao);

        if (p_owner->caps().vertex_format) {
#ifdef GS_CONFIG_SEPARATE_VERTEX_FORMAT
            // If separate vertex format is available, one VAO used
            // per state. Static input buffers bound inside VAO, dynamic buffers
            // are not bound and left for Input object

            // If no separate vertex format available - static buffer binding goes
            // into every Input object created for this state
            GLuint binding = 0;
            for (auto &is : p_input) {
                setformat(is.decl, binding, is.divisor);
                if (is.buffer) {
                    glBindVertexBuffer(
                        binding, is.buffer->getVertexBufferID(), 0,
                        is.buffer->vertexDecl().buffer_size()
                    );
                }
                ++binding;
            }
#endif
        } else {
            // but if all input slots are static, VAO built here to attach input buffers
            for (auto &is : p_input) {
                glBindBuffer(GL_ARRAY_BUFFER, is.buffer->getVertexBufferID());
                setarrays(is.decl, is.divisor, nullptr);
            }
        }

        if (p_primaryslot != GS_UNDEFINED && p_input[p_primaryslot].buffer) {
            glBindBuffer(
                GL_ELEMENT_ARRAY_BUFFER,
                p_input[p_primaryslot].buffer->getIndexBufferID()
            );
        }

#ifdef _DEBUG
        glBindVertexArray(0);
#endif
    }

    // fixed state
    p_rasterizerdiscard = desc.rasterizer.discard;
    p_sampleshading = desc.rasterizer.sampleshading;
    p_fill = gl_fill_mode(desc.rasterizer.fill);
    p_cull = desc.rasterizer.cull != GS_CULL_NONE;
    p_cullface = gl_cull_face(desc.rasterizer.cull);
    p_colormask = desc.blend.writemask != 0;
    p_depthmask = desc.depthstencil.depthmask;
    p_depthtest = desc.depthstencil.depthtest != GS_DEPTHTEST_NONE;
    p_depthfunc = gl_compare_func(desc.depthstencil.depthtest);
    p_blendseparate = desc.blend.separate;
    for (size_t n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        p_blend[n] = desc.blend.parameters[n].colorop != GS_BLEND_NONE && desc.blend.parameters[n].alphaop != GS_BLEND_NONE;
        p_blendeq[n] = gl_blend_eq(desc.blend.parameters[n].colorop);
        p_blendsrc[n] = gl_blend_factor(desc.blend.parameters[n].src);
        p_blenddst[n] = gl_blend_factor(desc.blend.parameters[n].dst);
        p_blendeqalpha[n] = gl_blend_eq(desc.blend.parameters[n].alphaop);
        p_blendsrcalpha[n] = gl_blend_factor(desc.blend.parameters[n].srcalpha);
        p_blenddstalpha[n] = gl_blend_factor(desc.blend.parameters[n].dstalpha);
    }
    p_polygonoffset = desc.rasterizer.polygonoffset;
    p_multisample = desc.rasterizer.multisample;

    p_allocated = true;

    return p_owner->error(GS_OK);
 }

GLint xGSStateImpl::attribLocation(const char *name) const
{
    return glGetAttribLocation(p_program, name);
}

bool xGSStateImpl::validate(const GSenum *colorformats, GSenum depthstencilformat)
{
    // TODO: check possibility to NULL rendering destination, if so
    // null targets should be skipped
    // also review RT formats matching

    if (!p_rasterizerdiscard) {
        for (size_t n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
            if (p_colorformats[n] != colorformats[n] && colorformats[n] != GS_COLOR_NONE) {
                return false;
            }
        }

        if (p_depthstencilformat != depthstencilformat && depthstencilformat != GS_DEPTH_NONE && p_depthstencilformat != GS_DEPTH_NONE) {
            return false;
        }
    }

    return true;
}

void xGSStateImpl::setarrays(const GSvertexdecl &decl, GSuint divisor, GSptr vertexptr) const
{
    GSint stride = decl.buffer_size();
    GSint offset = 0;

    for (auto const &i : decl.declaration())
    {
        GLint attrib = attribLocation(i.name);
        if (attrib != -1) {
            glEnableVertexAttribArray(attrib);
            glVertexAttribPointer(
                attrib, vertex_component_count(i), GL_FLOAT, GL_FALSE,
                stride, ptr_offset(vertexptr, offset)
            );
            glVertexAttribDivisor(attrib, divisor);
        }

        offset += vertex_component_size(i);
    }
}

void xGSStateImpl::setformat(const GSvertexdecl &decl, GSuint binding, GSuint divisor) const
{
#ifdef GS_CONFIG_SEPARATE_VERTEX_FORMAT
    GSint offset = 0;

    for (auto const &i : decl.declaration())
    {
        GLint attrib = attribLocation(i.name);
        if (attrib != -1) {
            glEnableVertexAttribArray(attrib);
            glVertexAttribFormat(attrib, vertex_component_count(i), GL_FLOAT, GL_FALSE, offset);
            glVertexAttribBinding(attrib, binding);
        }

        offset += vertex_component_size(i);
    }

    glVertexBindingDivisor(binding, divisor);
#endif
}

void xGSStateImpl::apply(const GScaps &caps)
{
    glUseProgram(p_program);

    if (p_vao) {
        glBindVertexArray(p_vao);
    }

    p_staticstate.apply(caps, p_owner, this);

    if (p_rasterizerdiscard) {
        glEnable(GL_RASTERIZER_DISCARD);
    } else {
        glDisable(GL_RASTERIZER_DISCARD);

        if (p_sampleshading) {
            glEnable(GL_SAMPLE_SHADING);
            glMinSampleShading(0.0f);
        } else {
            glDisable(GL_SAMPLE_SHADING);
        }

        glPolygonMode(GL_FRONT_AND_BACK, p_fill);

        if (!p_cull) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
            glCullFace(p_cullface);
        }

        glColorMask(p_colormask, p_colormask, p_colormask, p_colormask);

        glDepthMask(p_depthmask);

        if (!p_depthtest) {
            glDisable(GL_DEPTH_TEST);
        } else {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(p_depthfunc);
        }

        if (p_blendseparate && caps.multi_blend) {
            for (GLuint n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
                if (!p_blend[n]) {
                    glDisablei(GL_BLEND, n);
                } else {
                    glEnablei(GL_BLEND, n);
                    glBlendEquationSeparatei(n, p_blendeq[n], p_blendeqalpha[n]);
                    glBlendFuncSeparatei(
                        n,
                        p_blendsrc[n], p_blenddst[n],
                        p_blendsrcalpha[n], p_blenddstalpha[n]
                    );
                }
            }
        } else {
            if (!p_blend[0]) {
                glDisable(GL_BLEND);
            } else {
                glEnable(GL_BLEND);
                glBlendEquationSeparate(p_blendeq[0], p_blendeqalpha[0]);
                glBlendFuncSeparate(
                    p_blendsrc[0], p_blenddst[0],
                    p_blendsrcalpha[0], p_blenddstalpha[0]
                );
            }
        }

        if (p_polygonoffset == 0) {
            glPolygonOffset(1, 1);
        } else {
            glPolygonOffset(-GLfloat(p_polygonoffset) * 0.5f, 1.0f);
        }

        p_multisample ? glEnable(GL_MULTISAMPLE) : glDisable(GL_MULTISAMPLE);
    }
}

void xGSStateImpl::bindNullProgram()
{
    glUseProgram(0);
}

void xGSStateImpl::ReleaseRendererResources()
{
    if (p_program) {
        glDeleteProgram(p_program);
        p_program = 0;
    }

    for (auto &is : p_input) {
        if (is.buffer) {
            is.buffer->Release();
        }
    }
    p_input.clear();

    if (p_vao) {
        glDeleteVertexArrays(1, &p_vao);
        p_vao = 0;
    }

    p_staticstate.ReleaseRendererResources(p_owner);

    p_allocated = false;
}

GSbool xGSStateImpl::uniformIsSampler(GLenum type) const
{
    return (type == GL_SAMPLER_1D) || (type == GL_SAMPLER_1D_SHADOW) ||
           (type == GL_SAMPLER_2D) || (type == GL_SAMPLER_2D_SHADOW) ||
           (type == GL_SAMPLER_2D_RECT) || (type == GL_SAMPLER_2D_RECT_SHADOW) ||
           (type == GL_SAMPLER_2D_MULTISAMPLE) ||
           (type == GL_SAMPLER_3D) ||
           (type == GL_SAMPLER_CUBE) || (type == GL_SAMPLER_CUBE_SHADOW) ||
           (type == GL_SAMPLER_1D_ARRAY) || (type == GL_SAMPLER_1D_ARRAY_SHADOW) ||
           (type == GL_SAMPLER_2D_ARRAY) || (type == GL_SAMPLER_2D_ARRAY_SHADOW) ||
           (type == GL_SAMPLER_CUBE_MAP_ARRAY) || (type == GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW) ||
           (type == GL_SAMPLER_BUFFER);
}

void xGSStateImpl::AttachShaders(GLenum type, const char **source, vector<GLuint> &shaders)
{
    if (source == nullptr) {
        return;
    }

    while (*source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, source, nullptr);
        glCompileShader(shader);

#ifdef _DEBUG
        GLsizei loglen = 0;
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, &loglen, infoLog);
        infoLog[loglen] = 0;
        if (loglen) {
            p_owner->debug(DebugMessageLevel::Information, "Shader info log:\n%s\n", infoLog);
        }
#endif

        GLint status = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) {
            p_owner->debug(DebugMessageLevel::Error, "Shader failed to compile!\n");
            glDeleteShader(shader);
            p_owner->error(GSE_INVALIDOPERATION);
        }

        shaders.push_back(shader);

        ++source;
    }
}

void xGSStateImpl::EnumAttributes()
{
    GLint attribcount = 0;
    glGetProgramiv(p_program, GL_ACTIVE_ATTRIBUTES, &attribcount);
#ifdef _DEBUG
    p_owner->debug(DebugMessageLevel::Information, "Program active attributes:\n");
#endif
    p_attributes.clear();
    for (int n = 0; n < attribcount; ++n) {
        char name[256];
        GLsizei len = 0;
        GLsizei size = 0;
        GLenum type = 0;
        glGetActiveAttrib(p_program, n, 255, &len, &size, &type, name);
        name[len] = 0;

        Attribute a;
        a.location = glGetAttribLocation(p_program, name);
        a.type = type;
        a.name = name;
        a.size = size;

        p_attributes.push_back(a);
#ifdef _DEBUG
        p_owner->debug(
            DebugMessageLevel::Information,
            "    Attribute #%i: %s, location: %i, size: %i, type: %s\n",
            n, name, a.location, size, uniform_type(type)
        );
#endif
    }
}

void xGSStateImpl::EnumUniforms()
{
    GLint uniformcount = 0;
    glGetProgramiv(p_program, GL_ACTIVE_UNIFORMS, &uniformcount);
#ifdef _DEBUG
    p_owner->debug(DebugMessageLevel::Information, "Program active uniforms:\n");
#endif
    p_uniforms.clear();
    for (int n = 0; n < uniformcount; ++n) {
        char name[256];
        GLsizei len = 0;
        GLsizei size = 0;
        GLenum type = 0;
        glGetActiveUniform(p_program, n, 255, &len, &size, &type, name);
        name[len] = 0;

        // location != active uniform index !!!
        Uniform u;
        u.location = glGetUniformLocation(p_program, name);
        u.type = type;
        u.name = name;
        u.size = size;

        GLint value = 0;
        glGetActiveUniformsiv(
            p_program, 1, reinterpret_cast<GLuint*>(&n),
            GL_UNIFORM_OFFSET, &value
        );
        u.offset = value;

        value = 0;
        glGetActiveUniformsiv(
            p_program, 1, reinterpret_cast<GLuint*>(&n),
            GL_UNIFORM_ARRAY_STRIDE, &value
        );
        u.arraystride = value;

        value = 0;
        glGetActiveUniformsiv(
            p_program, 1, reinterpret_cast<GLuint*>(&n),
            GL_UNIFORM_MATRIX_STRIDE, &value
        );
        u.matrixstride = value;

        value = 0;
        glGetActiveUniformsiv(
            p_program, 1, reinterpret_cast<GLuint*>(&n),
            GL_UNIFORM_BLOCK_INDEX, &value
        );
        u.blockindex = value;
        u.sampler = uniformIsSampler(type);

        p_uniforms.push_back(u);
#ifdef _DEBUG
        p_owner->debug(
            DebugMessageLevel::Information,
            "    Uniform #%i: %s, location: %i, size: %i, type: %s, block index: %i\n",
            n, name, u.location, size, uniform_type(type), u.blockindex
        );
#endif
    }
}

void xGSStateImpl::EnumUniformBlocks()
{
    GLint uniformblockcount = 0;
    glGetProgramiv(p_program, GL_ACTIVE_UNIFORM_BLOCKS, &uniformblockcount);
#ifdef _DEBUG
    p_owner->debug(DebugMessageLevel::Information, "Program active uniform blocks:\n");
#endif
    p_uniformblocks.clear();
    for (int n = 0; n < uniformblockcount; ++n) {
        char name[256];
        GLsizei len = 0;
        glGetActiveUniformBlockName(p_program, n, 255, &len, name);
        name[len] = 0;

        UniformBlock u;
        u.index = n;
        u.name = name;

        GLint datasize = 0;
        glGetActiveUniformBlockiv(p_program, n, GL_UNIFORM_BLOCK_DATA_SIZE, &datasize);
        u.datasize = datasize;

        p_uniformblocks.push_back(u);

        // 1:1 index to binding point mapping
        glUniformBlockBinding(p_program, n, n);

#ifdef _DEBUG
        p_owner->debug(
            DebugMessageLevel::Information,
            "    Uniform block #%i: %s, index: %i, datasize: %i\n",
            n, name, glGetUniformBlockIndex(p_program, name), datasize
        );

        GLint activeuniforms = 0;
        glGetActiveUniformBlockiv(
            p_program, n,
            GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeuniforms
        );

        GLint *indices = new GLint[activeuniforms];

        glGetActiveUniformBlockiv(
            p_program, n,
            GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices
        );
        for (int u = 0; u < activeuniforms; ++u) {
            p_owner->debug(
                DebugMessageLevel::Information,
                "        Active uniform #%i: %s, offset: %i, array stride: %i, matrix stride: %i\n",
                indices[u],
                p_uniforms[indices[u]].name.c_str(),
                p_uniforms[indices[u]].offset,
                p_uniforms[indices[u]].arraystride,
                p_uniforms[indices[u]].matrixstride
            );
        }

        delete[] indices;
#endif
    }
}

template<typename T>
void xGSStateImpl::CreateElementIndex(ElementIndexMap &map, const T &elementlist) const
{
    map.clear();
    for (size_t n = 0; n < elementlist.size(); ++n) {
        map.insert(std::make_pair(elementlist[n].name, GSint(n)));
    }
}

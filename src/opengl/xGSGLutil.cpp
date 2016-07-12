/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    opengl/xGSGLutil.cpp
        OpenGL specific utility functions and classes
*/

#include "xGSGLutil.h"
#include "xGSimpl.h"
#include "xGSstate.h"
#include "xGStexture.h"
#include "xGSdatabuffer.h"


using namespace xGS;


#ifdef _DEBUG
const char* xGS::uniform_type(GLenum type)
{
    switch (type) {
        case GL_SAMPLER_1D: return "SAMPLER_1D";
        case GL_SAMPLER_1D_ARRAY: return "SAMPLER_1D_ARRAY";
        case GL_SAMPLER_2D: return "SAMPLER_2D";
        case GL_SAMPLER_2D_MULTISAMPLE: return "SAMPLER_2D_MULTISAMPLE";
        case GL_SAMPLER_2D_ARRAY: return "SAMPLER_2D_ARRAY";
        case GL_SAMPLER_3D: return "SAMPLER_3D";
        case GL_SAMPLER_CUBE: return "SAMPLER_CUBE";
        case GL_SAMPLER_CUBE_MAP_ARRAY: return "SAMPLER_CUBE_MAP_ARRAY";

        case GL_SAMPLER_1D_SHADOW: return "SAMPLER_1D_SHADOW";
        case GL_SAMPLER_1D_ARRAY_SHADOW: return "SAMPLER_1D_ARRAY_SHADOW";
        case GL_SAMPLER_2D_SHADOW: return "SAMPLER_2D_SHADOW";
        case GL_SAMPLER_2D_ARRAY_SHADOW: return "SAMPLER_2D_ARRAY_SHADOW";
        case GL_SAMPLER_CUBE_SHADOW: return "SAMPLER_CUBE_SHADOW";
        case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW: return "SAMPLER_CUBE_MAP_ARRAY_SHADOW";

        case GL_SAMPLER_2D_RECT: return "GL_SAMPLER_2D_RECT";
        case GL_SAMPLER_2D_RECT_SHADOW: return "GL_SAMPLER_2D_RECT_SHADOW";

        case GL_SAMPLER_BUFFER: return "SAMPLER_BUFFER";

        case GL_FLOAT: return "FLOAT";
        case GL_FLOAT_VEC2: return "FLOAT_VEC2";
        case GL_FLOAT_VEC3: return "FLOAT_VEC3";
        case GL_FLOAT_VEC4: return "FLOAT_VEC4";

        case GL_FLOAT_MAT2: return "FLOAT_MAT2";
        case GL_FLOAT_MAT3: return "FLOAT_MAT3";
        case GL_FLOAT_MAT4: return "FLOAT_MAT4";

        case GL_INT: return "INT";
        case GL_INT_VEC2: return "INT_VEC2";
        case GL_INT_VEC3: return "INT_VEC3";
        case GL_INT_VEC4: return "INT_VEC4";

        case GL_BOOL: return "BOOL";
        case GL_BOOL_VEC2: return "BOOL_VEC2";
        case GL_BOOL_VEC3: return "BOOL_VEC3";
        case GL_BOOL_VEC4: return "BOOL_VEC4";
    }

    return "? uniform";
}
#endif


GSParametersState::GSParametersState() :
    p_uniformblockdata(),
    p_textures(),
    p_firstslot(0),
    p_texture_targets(),
    p_texture_binding(),
    p_texture_samplers()
{}

GSerror GSParametersState::allocate(xGSImpl *impl, xGSStateImpl *state, const GSParameterSet &set, const GSuniformbinding *uniforms, const GStexturebinding *textures, const GSconstantvalue *constants)
{
    // TODO: think about what to do with slots which weren't present in state program
    // so their samplers weren't allocated

    GSuint slotcount = set.onepastlast - set.first;
    GSuint samplercount = set.onepastlastsampler - set.firstsampler;
    GSuint blockscount = slotcount - samplercount - set.constantcount;

    p_firstslot = set.firstsampler;

    p_uniformblockdata.clear();
    p_uniformblockdata.reserve(blockscount);

    // bind uniform blocks
    if (uniforms) {
        const GSuniformbinding *binding = uniforms;
        while (binding->slot != GSPS_END) {
            GSuint slotindex = binding->slot - GSPS_0;
            if (slotindex >= slotcount) {
                return GSE_INVALIDVALUE;
            }

            const xGSStateImpl::ParameterSlot &slot = state->parameterSlot(slotindex + set.first);
            if (slot.type != GSPD_BLOCK) {
                return GSE_INVALIDENUM;
            }

            if (slot.location != GS_DEFAULT) {
                xGSDataBufferImpl *buffer = static_cast<xGSDataBufferImpl*>(binding->buffer);
                if (!buffer->allocated()) {
                    return GSE_INVALIDOBJECT;
                }

                if (binding->block >= buffer->blockCount()) {
                    return GSE_INVALIDVALUE;
                }

                const xGSDataBufferImpl::UniformBlock &block = buffer->block(binding->block);

                UniformBlockData ub = {
                    GLuint(slot.location),
                    block.offset + block.size * binding->index, block.size,
                    buffer
                };
                p_uniformblockdata.push_back(ub);
            }
            ++binding;
        }
    }

    p_textures.clear();
    p_texture_targets.clear();
    p_texture_binding.clear();
    p_texture_samplers.clear();

    p_textures.reserve(samplercount);
    p_texture_targets.resize(samplercount);
    p_texture_binding.resize(samplercount);
    p_texture_samplers.resize(samplercount);

    // bind textures & samplers
    if (textures) {
        const GStexturebinding *binding = textures;
        while (binding->slot != GSPS_END) {
            GSuint slotindex = binding->slot - GSPS_0;
            if (slotindex >= slotcount) {
                return GSE_INVALIDVALUE;
            }

            const xGSStateImpl::ParameterSlot &slot = state->parameterSlot(slotindex + set.first);
            if (slot.type != GSPD_TEXTURE) {
                return GSE_INVALIDENUM;
            }

            if (slot.location != GS_DEFAULT) {
                xGSTextureImpl *texture = static_cast<xGSTextureImpl*>(binding->texture);
                GSuint sampler = binding->sampler - GSS_0;

                if (sampler >= impl->samplerCount()) {
                    return GSE_INVALIDVALUE;
                }

                TextureSlot t = { texture, sampler };
                p_textures.push_back(t);

                p_texture_targets[slot.location] = texture->target();
                p_texture_binding[slot.location] = texture->getID();
                p_texture_samplers[slot.location] = impl->sampler(sampler);
            }

            ++binding;
        }
    }

    // copy constant values
    if (constants) {
        p_constants.reserve(set.constantcount);
        size_t memoffset = 0;

        const GSconstantvalue *constval = constants;
        while (constval->slot != GSPS_END) {
            GSuint slotindex = constval->slot - GSPS_0;
            if (slotindex >= slotcount) {
                return GSE_INVALIDVALUE;
            }

            const xGSStateImpl::ParameterSlot &slot = state->parameterSlot(slotindex + set.first);
            if (slot.type != GSPD_CONSTANT) {
                return GSE_INVALIDENUM;
            }

            if (slot.location != GS_DEFAULT) {
                ConstantValue cv = {
                    constval->type,
                    slot.location,
                    memoffset
                };

                size_t size = 0;
                switch (constval->type) {
                    case GSU_SCALAR: size = sizeof(float) * 1; break;
                    case GSU_VEC2: size = sizeof(float) * 2; break;
                    case GSU_VEC3: size = sizeof(float) * 3; break;
                    case GSU_VEC4: size = sizeof(float) * 4; break;
                    case GSU_MAT2: size = sizeof(float) * 4; break;
                    case GSU_MAT3: size = sizeof(float) * 9; break;
                    case GSU_MAT4: size = sizeof(float) * 16; break;

                    default:
                        return GSE_INVALIDENUM;
                }

                // TODO: optimize constant memory, change for more robust container
                p_constantmemory.resize(p_constantmemory.size() + size);
                memcpy(p_constantmemory.data() + cv.offset, constval->value, size);
                memoffset += size;

                p_constants.push_back(cv);
            }

            ++constval;
        }
    }

    for (auto &b : p_uniformblockdata) {
        b.buffer->AddRef();
    }

    for (auto &t : p_textures) {
        t.texture->AddRef();
        impl->referenceSampler(t.sampler);
    }

    return GS_OK;
}

void GSParametersState::apply(const GScaps &caps, xGSImpl *impl, xGSStateImpl *state)
{
    // set up uniform blocks
    for (auto u : p_uniformblockdata) {
        glBindBufferRange(GL_UNIFORM_BUFFER, u.index, u.buffer->getID(), u.offset, u.size);
    }

    if (p_textures.size()) {
        if (caps.multi_bind) {
            GLuint count = GLuint(p_texture_binding.size());
            glBindTextures(p_firstslot, count, p_texture_binding.data());
            glBindSamplers(p_firstslot, count, p_texture_samplers.data());
        } else {
            // set up textures
            for (size_t n = 0; n < p_texture_binding.size(); ++n) {
                glActiveTexture(GL_TEXTURE0 + p_firstslot + GLenum(n));
                if (p_texture_binding[n]) {
                    glBindTexture(
                        p_texture_targets[n],
                        p_texture_binding[n]
                    );
                    glBindSampler(GLuint(n) + p_firstslot, p_texture_samplers[n]);
#ifdef _DEBUG
                } else {
                    xGSTextureImpl::bindNullTexture();
                    glBindSampler(GLuint(n) + p_firstslot, 0);
#endif
                }
            }
        }
    }

    for (auto c : p_constants) {
        // TODO: copypasta - similar code in Impl::SetUniformValue
        const GLfloat *value = reinterpret_cast<const GLfloat*>(p_constantmemory.data() + c.offset);
        switch (c.type) {
            case GSU_SCALAR:
                glUniform1fv(c.location, 1, value);
                break;

            case GSU_VEC2:
                glUniform2fv(c.location, 1, value);
                break;

            case GSU_VEC3:
                glUniform3fv(c.location, 1, value);
                break;

            case GSU_VEC4:
                glUniform4fv(c.location, 1, value);
                break;

            case GSU_MAT2:
                glUniformMatrix2fv(c.location, 1, GL_FALSE, value);
                break;

            case GSU_MAT3:
                glUniformMatrix3fv(c.location, 1, GL_FALSE, value);
                break;

            case GSU_MAT4:
                glUniformMatrix4fv(c.location, 1, GL_FALSE, value);
                break;
        }
    }

#ifdef _DEBUG
    for (size_t n = 0; n < p_textures.size(); ++n) {
        xGSTextureImpl *texture = p_textures[n].texture;

        bool depthtexture =
            texture->format() == GS_DEPTH_16 ||
            texture->format() == GS_DEPTH_24 ||
            texture->format() == GS_DEPTH_32 ||
            texture->format() == GS_DEPTH_32_FLOAT ||
            texture->format() == GS_DEPTHSTENCIL_D24S8;

        if (texture->boundAsRT() && (!depthtexture || state->depthMask())) {
            //impl->debug(DebugMessageLevel::Warning, "Texture bound as RT is being used as source [id=%i]\n", p_textures[n].texture->getID());
        }
    }
#endif
}

void GSParametersState::ReleaseRendererResources(xGSImpl *impl)
{
    for (auto &u : p_uniformblockdata) {
        if (u.buffer) {
            u.buffer->Release();
        }
    }

    for (auto &t : p_textures) {
        if (t.texture) {
            t.texture->Release();
        }
        impl->dereferenceSampler(t.sampler);
    }
}


/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/xgs

    dx12/xGSDX12util.cpp
        DirectX 12 specific utility functions and classes
*/

#include "xGSDX12util.h"
#include "xGSimpl.h"
#include "xGSstate.h"
#include "xGStexture.h"
#include "xGSdatabuffer.h"


using namespace xGS;


#ifdef _DEBUG
const char* xGS::uniform_type(int type)
{
    // TODO

    return "? uniform";
}
#endif


GSParametersState::GSParametersState() :
    p_uniformblockdata(),
    p_textures(),
    p_firstslot(0)
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
                //if (!buffer->allocated()) {
                //    return GSE_INVALIDOBJECT;
                //}

                if (binding->block >= buffer->blockCount()) {
                    return GSE_INVALIDVALUE;
                }

                const xGSDataBufferImpl::UniformBlock &block = buffer->block(binding->block);

                UniformBlockData ub = {
                    GSuint(slot.location),
                    block.offset + block.size * binding->index, block.size,
                    buffer
                };
                p_uniformblockdata.push_back(ub);
            }
            ++binding;
        }
    }

    p_textures.clear();

    p_textures.reserve(samplercount);

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

                // TODO
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
    // TODO: GSParametersState::apply

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


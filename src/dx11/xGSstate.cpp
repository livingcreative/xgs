/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx11/xGSstate.cpp
        State object implementation class
*/

#include "xGSstate.h"
#include "xGSgeometrybuffer.h"
#include "xGSdatabuffer.h"
#include "xGStexture.h"
#include <d3d11.h>

// temp. stuff for compiling HLSL
#include <d3dcompiler.h>


using namespace xGS;
using namespace std;


xGSStateImpl::xGSStateImpl(xGSImpl *owner) :
    xGSObjectBase(owner),
    p_inputlayout(nullptr),
    p_vs(nullptr),
    p_ps(nullptr)
{}

xGSStateImpl::~xGSStateImpl()
{}

GSbool xGSStateImpl::AllocateImpl(const GSstatedescription &desc, GSuint staticinputslots, const GSparameterlayout *staticparams, GSuint staticset)
{
    D3D11_INPUT_ELEMENT_DESC inputelements[32];
    UINT inputelementscount = 0;

    const GSinputlayout *inputlayout = desc.inputlayout;
    while (inputlayout->slottype != GSI_END) {
        switch (inputlayout->slottype) {
            case GSI_DYNAMIC:
                ++p_inputavail;
                break;

            case GSI_STATIC: {
                const GSvertexcomponent *comp = inputlayout->decl;
                while (comp->type != GSVD_END) {
                    D3D11_INPUT_ELEMENT_DESC &el = inputelements[inputelementscount];

                    el.AlignedByteOffset =
                        inputelementscount == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;

                    switch (comp->type) {
                        case GSVD_POS:
                        case GSVD_VEC3:
                            el.Format = DXGI_FORMAT_R32G32B32_FLOAT;
                            break;

                        case GSVD_FLOAT:
                            el.Format = DXGI_FORMAT_R32_FLOAT;
                            break;

                        case GSVD_VEC2:
                            el.Format = DXGI_FORMAT_R32G32_FLOAT;
                            break;

                        case GSVD_POSW:
                        case GSVD_VEC4:
                            el.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                            break;

                        default:
                            // TODO: handle error
                            break;
                    }

                    el.InputSlot = 0;
                    el.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                    el.InstanceDataStepRate = 0;
                    el.SemanticIndex = comp->index;
                    el.SemanticName = "INPUT";

                    ++inputelementscount;

                    ++comp;
                }
                break;
            }
        }
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
                        // TODO
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

        // TODO

        delete[] feedback;
    }

    // TODO: shader code

    ID3DBlob *err = nullptr;

    ID3D11ShaderReflection *vsreflection = nullptr;
    ID3D11ShaderReflection *psreflection = nullptr;

    D3D11_SHADER_DESC vsdesc;
    D3D11_SHADER_DESC psdesc;

    ID3DBlob *vsblob = nullptr;
    HRESULT res = D3DCompile(
        desc.vs[0], strlen(desc.vs[0]), nullptr, nullptr, nullptr,
        "VSmain", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &vsblob, &err
    );
    if (res != S_OK && err != nullptr) {
        const char *text = reinterpret_cast<const char*>(err->GetBufferPointer());
        string errtext(text, text + err->GetBufferSize());
        p_owner->debug(
            DebugMessageLevel::Error,
            "Error compiling vertex shader: %s\n",
            errtext.c_str()
        );
        ::Release(err);
    }
    if (vsblob) {
        p_owner->device()->CreateVertexShader(
            vsblob->GetBufferPointer(), vsblob->GetBufferSize(), nullptr,
            &p_vs
        );

        D3DReflect(
            vsblob->GetBufferPointer(), vsblob->GetBufferSize(),
            IID_ID3D11ShaderReflection, reinterpret_cast<void**>(&vsreflection)
        );

        vsreflection->GetDesc(&vsdesc);
    }

    ID3DBlob *psblob = nullptr;
    res = D3DCompile(
        desc.ps[0], strlen(desc.ps[0]), nullptr, nullptr, nullptr,
        "PSmain", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &psblob, &err
    );
    if (res != S_OK && err != nullptr) {
        const char *text = reinterpret_cast<const char*>(err->GetBufferPointer());
        string errtext(text, text + err->GetBufferSize());
        p_owner->debug(
            DebugMessageLevel::Error,
            "Error compiling pixel shader: %s\n",
            errtext.c_str()
        );
        ::Release(err);
    }
    if (psblob) {
        p_owner->device()->CreatePixelShader(
            psblob->GetBufferPointer(), psblob->GetBufferSize(), nullptr,
            &p_ps
        );

        D3DReflect(
            psblob->GetBufferPointer(), psblob->GetBufferSize(),
            IID_ID3D11ShaderReflection, reinterpret_cast<void**>(&psreflection)
        );

        psreflection->GetDesc(&psdesc);
    }

    // TODO: check status
    EnumAttributes(vsdesc, vsreflection);

    EnumUniforms(vsdesc, vsreflection);
    EnumUniforms(psdesc, psreflection);

    EnumUniformBlocks(vsdesc, vsreflection);
    EnumUniformBlocks(psdesc, psreflection);


    GSuint currenttextureslot = 0;

    // TODO: bind parameters to shaders
    const GSparameterlayout *paramset = desc.parameterlayout;
    while (paramset->settype != GSP_END) {
        const GSparameterdecl *param = paramset->parameters;
        while (param->type != GSPD_END) {
            switch (param->type) {
                case GSPD_CONSTANT:
                    if (param->location == GS_DEFAULT) {
                        p_owner->debug(DebugMessageLevel::Warning, "Requested parameter \"%s\" not found in program parameters\n", param->name);
                    }
                    break;

                case GSPD_BLOCK:
                    if (param->location == GS_DEFAULT) {
                        p_owner->debug(DebugMessageLevel::Warning, "Requested parameter \"%s\" not found in program parameters\n", param->name);
                    }
                    break;

                case GSPD_TEXTURE: {
                    GSint location = param->location;
                    if (location == GS_DEFAULT) {
                        p_owner->debug(DebugMessageLevel::Warning, "Requested parameter \"%s\" not found in program parameters\n", param->name);
                    } else {
                        // TODO

                        p_owner->debug(
                            DebugMessageLevel::Information,
                            "Program texture slot \"%s\" with location %i got texture slot #%i\n",
                            param->name, location + param->index, currenttextureslot
                        );

                        ++currenttextureslot;
                    }
                    break;
                }
            }

            ++param;
        }

        ++paramset;
    }

    if (staticparams) {
        // bind static parameters
        // TODO: failure handling
        p_staticstate.allocate(
            p_owner, this, p_parametersets[staticset],
            staticparams->uniforms, staticparams->textures, nullptr
        );
    }


    // TODO: allocate input streams
    if (p_inputavail == 0 && staticinputslots) {
        res = p_owner->device()->CreateInputLayout(
            inputelements, inputelementscount,
            vsblob->GetBufferPointer(), vsblob->GetBufferSize(), &p_inputlayout
        );
    }

    ::Release(vsblob);
    ::Release(psblob);

    ::Release(vsreflection);
    ::Release(psreflection);

    // fixed state
    p_rasterizerdiscard = desc.rasterizer.discard;
    p_sampleshading = desc.rasterizer.sampleshading;
    p_fill = dx11_fill_mode(desc.rasterizer.fill);
    p_cull = desc.rasterizer.cull != GS_CULL_NONE;
    p_cullface = dx11_cull_face(desc.rasterizer.cull);
    p_pointsize = desc.rasterizer.pointsize;
    p_programpointsize = desc.rasterizer.programpointsize != 0;
    p_colormask = desc.blend.writemask != 0;
    p_depthmask = desc.depthstencil.depthmask != 0;
    p_depthtest = desc.depthstencil.depthtest != GS_DEPTHTEST_NONE;
    p_depthfunc = dx11_compare_func(desc.depthstencil.depthtest);
    p_blendseparate = desc.blend.separate != 0;
    for (size_t n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        p_blend[n] = desc.blend.parameters[n].colorop != GS_BLEND_NONE && desc.blend.parameters[n].alphaop != GS_BLEND_NONE;
        p_blendeq[n] = dx11_blend_eq(desc.blend.parameters[n].colorop);
        p_blendsrc[n] = dx11_blend_factor(desc.blend.parameters[n].src);
        p_blenddst[n] = dx11_blend_factor(desc.blend.parameters[n].dst);
        p_blendeqalpha[n] = dx11_blend_eq(desc.blend.parameters[n].alphaop);
        p_blendsrcalpha[n] = dx11_blend_factor(desc.blend.parameters[n].srcalpha);
        p_blenddstalpha[n] = dx11_blend_factor(desc.blend.parameters[n].dstalpha);
    }
    p_polygonoffset = desc.rasterizer.polygonoffset;
    p_multisample = desc.rasterizer.multisample != 0;

    return p_owner->error(GS_OK);
 }

int xGSStateImpl::attribLocation(const char *name) const
{
    // TODO
    return 0;
}

void xGSStateImpl::apply(const GScaps &caps)
{
    // TODO: xGSStateImpl::apply

    ID3D11DeviceContext *context = p_owner->context();

    if (p_inputlayout) {
        context->IASetInputLayout(p_inputlayout);

        auto &slot = p_input[p_primaryslot];

        auto buffer = slot.buffer->vertexbuffer();
        UINT stride = slot.buffer->vertexDecl().buffer_size();
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);

        if (slot.buffer->indexbuffer()) {
            context->IASetIndexBuffer(
                slot.buffer->indexbuffer(),
                dx11_index_type(slot.buffer->indexFormat()), 0
            );
        }
    }

    context->VSSetShader(p_vs, nullptr, 0);
    context->PSSetShader(p_ps, nullptr, 0);

    //context->VSSetShaderResources()

    p_staticstate.apply(p_owner->caps(), p_owner, this);
}

void xGSStateImpl::bindNullProgram()
{
    // TODO: xGSStateImpl::bindNullProgram
}

void xGSStateImpl::ReleaseRendererResources()
{
    // TODO: xGSStateImpl::ReleaseRendererResources
    ::Release(p_inputlayout);

    ::Release(p_vs);
    ::Release(p_ps);

    for (auto &is : p_input) {
        if (is.buffer) {
            is.buffer->Release();
        }
    }
    p_input.clear();

    p_staticstate.ReleaseRendererResources(p_owner);
}

GSbool xGSStateImpl::uniformIsSampler(int type) const
{
    return false;
}

void xGSStateImpl::EnumAttributes(const _D3D11_SHADER_DESC &desc, ID3D11ShaderReflection *vsr)
{
    // TODO: xGSStateImpl::EnumAttributes

#ifdef _DEBUG
    p_owner->debug(DebugMessageLevel::Information, "Program active attributes:\n");
#endif

    for (UINT n = 0; n < desc.InputParameters; ++n) {
        D3D11_SIGNATURE_PARAMETER_DESC pdesc;
        vsr->GetInputParameterDesc(n, &pdesc);

        Attribute attr = { 1, pdesc.SemanticName };
        p_attributes.emplace_back(attr);

#ifdef _DEBUG
        p_owner->debug(
            DebugMessageLevel::Information,
            "    Attribute #%i: %s, location: %i, size: %i, type: %s\n",
            n, pdesc.SemanticName, pdesc.Register, 0, "TODO" //uniform_type(type)
        );
#endif
    }
}

void xGSStateImpl::EnumUniforms(const _D3D11_SHADER_DESC &desc, ID3D11ShaderReflection *sr)
{
    // TODO: xGSStateImpl::EnumUniforms

    for (UINT n = 0; n < desc.BoundResources; ++n) {
        D3D11_SHADER_INPUT_BIND_DESC pdesc;
        sr->GetResourceBindingDesc(n, &pdesc);

        if (pdesc.Type == D3D_SIT_CBUFFER) {
            continue;
        }

        Uniform uniform = {};
        uniform.name = pdesc.Name;

        switch (pdesc.Type) {
            case D3D_SIT_TEXTURE:
                uniform.sampler = GS_TRUE;
                break;
        }

        p_uniforms.emplace_back(uniform);
    }
}

void xGSStateImpl::EnumUniformBlocks(const _D3D11_SHADER_DESC &desc, ID3D11ShaderReflection *sr)
{
    // TODO: xGSStateImpl::EnumUniformBlocks

    for (UINT n = 0; n < desc.BoundResources; ++n) {
        D3D11_SHADER_INPUT_BIND_DESC pdesc;
        sr->GetResourceBindingDesc(n, &pdesc);

        if (pdesc.Type == D3D_SIT_CBUFFER) {
            UniformBlock block = { 0, pdesc.Name };
            p_uniformblocks.emplace_back(block);
        }
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

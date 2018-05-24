/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    dx12/xGSstate.cpp
        State object implementation class
*/

#include "xGSstate.h"
#include "xGSgeometrybuffer.h"
#include "xGSdatabuffer.h"
#include "xGStexture.h"
#include <d3d12.h>

// temp. stuff for compiling HLSL
#include <d3dcompiler.h>


using namespace xGS;
using namespace std;


xGSStateImpl::xGSStateImpl(xGSImpl *owner) :
    xGSObjectBase(owner)
    //p_inputlayout(nullptr),
    //p_vs(nullptr),
    //p_ps(nullptr),
{}

xGSStateImpl::~xGSStateImpl()
{}

GSbool xGSStateImpl::AllocateImpl(const GSstatedescription &desc, GSuint staticinputslots, const GSparameterlayout *staticparams, GSuint staticset)
{
    D3D12_INPUT_ELEMENT_DESC inputelements[32];
    UINT inputelementscount = 0;

    const GSinputlayout *inputlayout = desc.inputlayout;
    while (inputlayout->slottype != GSI_END) {
        switch (inputlayout->slottype) {
            case GSI_DYNAMIC:
                break;

            case GSI_STATIC: {
                const GSvertexcomponent *comp = inputlayout->decl;
                while (comp->type != GSVD_END) {
                    D3D12_INPUT_ELEMENT_DESC &el = inputelements[inputelementscount];

                    el.AlignedByteOffset =
                        inputelementscount == 0 ? 0 : D3D12_APPEND_ALIGNED_ELEMENT;

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
                    }

                    el.InputSlot = 0;
                    el.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                    el.InstanceDataStepRate = 0;
                    el.SemanticIndex = inputelementscount;
                    el.SemanticName = "INPUT";

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

    ID3D12ShaderReflection *vsreflection = nullptr;
    ID3D12ShaderReflection *psreflection = nullptr;

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

    // TODO: check status
    EnumAttributes();
    EnumUniforms();
    EnumUniformBlocks();


    // gather parameters info
    p_parametersets.clear();
    p_parameterslots.clear();

    GSuint currenttextureslot = 0;

    D3D12_ROOT_PARAMETER parameters[64];
    size_t numsamplers = 0;
    D3D12_STATIC_SAMPLER_DESC samplers[16];

    // TODO: bind parameters to shaders
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

            D3D12_ROOT_PARAMETER &dxparam = parameters[p_parameterslots.size()];
            dxparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // TODO: optimize this

            switch (param->type) {
                case GSPD_CONSTANT:
                    slot.location = GS_DEFAULT;
                    if (slot.location == GS_DEFAULT) {
                        p_owner->debug(DebugMessageLevel::Warning, "Requested parameter \"%s\" not found in program parameters\n", param->name);
                    }
                    ++set.constantcount;

                    dxparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
                    dxparam.Constants.Num32BitValues = 1; // TODO
                    dxparam.Constants.RegisterSpace = 0;
                    dxparam.Constants.ShaderRegister = 0;

                    break;

                case GSPD_BLOCK:
                    slot.location = GS_DEFAULT;
                    if (slot.location == GS_DEFAULT) {
                        p_owner->debug(DebugMessageLevel::Warning, "Requested parameter \"%s\" not found in program parameters\n", param->name);
                    }

                    // TODO: this should be packed into set table
                    dxparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                    dxparam.Descriptor.RegisterSpace = 0;
                    dxparam.Descriptor.ShaderRegister = 1; // TODO: allocate regs

                    break;

                case GSPD_TEXTURE: {
                    GSint location = GS_DEFAULT;
                    if (location == GS_DEFAULT) {
                        p_owner->debug(DebugMessageLevel::Warning, "Requested parameter \"%s\" not found in program parameters\n", param->name);
                    } else {
                        slot.location = currenttextureslot - set.firstsampler;

                        // TODO

                        p_owner->debug(
                            DebugMessageLevel::Information,
                            "Program texture slot \"%s\" with location %i got texture slot #%i\n",
                            param->name, location + param->index, currenttextureslot
                        );

                        // TODO: this should be packed into set table
                        dxparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
                        dxparam.Descriptor.RegisterSpace = 0;
                        dxparam.Descriptor.ShaderRegister = 0;

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
            // TODO: failure handling
            p_staticstate.allocate(
                p_owner, this, set,
                paramset->uniforms, paramset->textures, nullptr
            );
        }

        ++paramset;
    }

    // TODO: allocate input streams
    if (p_inputavail == 0 && staticinputslots) {
        //res = p_owner->device()->CreateInputLayout(
        //    inputelements, inputelementscount,
        //    vsblob->GetBufferPointer(), vsblob->GetBufferSize(), &p_inputlayout
        //);
    }

    ID3DBlob *rsblob = nullptr;

    D3D12_ROOT_SIGNATURE_DESC rsdesc = {
        UINT(p_parameterslots.size()), parameters,
        UINT(numsamplers), samplers,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    };
    res = D3D12SerializeRootSignature(&rsdesc, D3D_ROOT_SIGNATURE_VERSION_1, &rsblob, &err);
    if (res != S_OK && err != nullptr) {
        const char *text = reinterpret_cast<const char*>(err->GetBufferPointer());
        string errtext(text, text + err->GetBufferSize());
        p_owner->debug(
            DebugMessageLevel::Error,
            "Error serializing root signature: %s\n",
            errtext.c_str()
        );
        ::Release(err);
    }

    res = p_owner->device()->CreateRootSignature(
        0, rsblob->GetBufferPointer(), rsblob->GetBufferSize(),
        IID_PPV_ARGS(&p_signature)
    );
    rsblob->Release();

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psodesc = {};

    res = p_owner->device()->CreateGraphicsPipelineState(&psodesc, IID_PPV_ARGS(&p_state));

    ::Release(vsblob);
    ::Release(psblob);

    ::Release(vsreflection);
    ::Release(psreflection);

    // fixed state
    p_rasterizerdiscard = desc.rasterizer.discard;
    p_sampleshading = desc.rasterizer.sampleshading;
    p_fill = dx12_fill_mode(desc.rasterizer.fill);
    p_cull = desc.rasterizer.cull != GS_CULL_NONE;
    p_cullface = dx12_cull_face(desc.rasterizer.cull);
    p_pointsize = desc.rasterizer.pointsize;
    p_programpointsize = desc.rasterizer.programpointsize != 0;
    p_colormask = desc.blend.writemask != 0;
    p_depthmask = desc.depthstencil.depthmask != 0;
    p_depthtest = desc.depthstencil.depthtest != GS_DEPTHTEST_NONE;
    p_depthfunc = dx12_compare_func(desc.depthstencil.depthtest);
    p_blendseparate = desc.blend.separate != 0;
    for (size_t n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        p_blend[n] = desc.blend.parameters[n].colorop != GS_BLEND_NONE && desc.blend.parameters[n].alphaop != GS_BLEND_NONE;
        p_blendeq[n] = dx12_blend_eq(desc.blend.parameters[n].colorop);
        p_blendsrc[n] = dx12_blend_factor(desc.blend.parameters[n].src);
        p_blenddst[n] = dx12_blend_factor(desc.blend.parameters[n].dst);
        p_blendeqalpha[n] = dx12_blend_eq(desc.blend.parameters[n].alphaop);
        p_blendsrcalpha[n] = dx12_blend_factor(desc.blend.parameters[n].srcalpha);
        p_blenddstalpha[n] = dx12_blend_factor(desc.blend.parameters[n].dstalpha);
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

    //ID3D11DeviceContext *context = p_owner->context();
    //
    //if (p_inputlayout) {
    //    context->IASetInputLayout(p_inputlayout);
    //}
    //
    //context->VSSetShader(p_vs, nullptr, 0);
    //context->PSSetShader(p_ps, nullptr, 0);

    p_staticstate.apply(p_owner->caps(), p_owner, this);
}

void xGSStateImpl::bindNullProgram()
{
    // TODO: xGSStateImpl::bindNullProgram
}

void xGSStateImpl::ReleaseRendererResources()
{
    // TODO: xGSStateImpl::ReleaseRendererResources
    //::Release(p_inputlayout);

    //::Release(p_vs);
    //::Release(p_ps);

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

void xGSStateImpl::EnumAttributes()
{
    // TODO: xGSStateImpl::EnumAttributes
}

void xGSStateImpl::EnumUniforms()
{
    // TODO: xGSStateImpl::EnumUniforms
}

void xGSStateImpl::EnumUniformBlocks()
{
    // TODO: xGSStateImpl::EnumUniformBlocks
}

template<typename T>
void xGSStateImpl::CreateElementIndex(ElementIndexMap &map, const T &elementlist) const
{
    map.clear();
    for (size_t n = 0; n < elementlist.size(); ++n) {
        map.insert(std::make_pair(elementlist[n].name, GSint(n)));
    }
}

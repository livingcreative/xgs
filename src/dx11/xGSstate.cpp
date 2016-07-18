/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    dx11/xGSstate.cpp
        State object implementation class
*/

#include "xGSstate.h"
#include "xGSgeometrybuffer.h"
#include "xGSdatabuffer.h"
#include "xGStexture.h"


using namespace xGS;
using namespace std;


xGSStateImpl::xGSStateImpl(xGSImpl *owner) :
    xGSObjectImpl(owner),
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
                // TODO
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

    //const GSpixelformat &fmt = p_owner->DefaultRenderTargetFormat();
    GSpixelformat fmt; // TODO
    for (size_t n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        p_colorformats[n] =
            desc.colorformats[n] == GS_COLOR_DEFAULT ?
                ColorFormatFromPixelFormat(fmt) :
                desc.colorformats[n];
    }
    p_depthstencilformat = desc.depthstencilformat == GS_DEPTH_DEFAULT ?
        DepthFormatFromPixelFormat(fmt) :
        desc.depthstencilformat;

    // TODO: shader code

    // TODO: check status
    EnumAttributes();
    EnumUniforms();
    EnumUniformBlocks();


    // gather parameters info
    p_parametersets.clear();
    p_parameterslots.clear();

    GSuint currenttextureslot = 0;

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

            switch (param->type) {
                case GSPD_CONSTANT:
                    slot.location = GS_DEFAULT;
                    if (slot.location == GS_DEFAULT) {
                        p_owner->debug(DebugMessageLevel::Warning, "Requested parameter \"%s\" not found in program parameters\n", param->name);
                    }
                    ++set.constantcount;
                    break;

                case GSPD_BLOCK:
                    slot.location = GS_DEFAULT;
                    if (slot.location == GS_DEFAULT) {
                        p_owner->debug(DebugMessageLevel::Warning, "Requested parameter \"%s\" not found in program parameters\n", param->name);
                    }
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

    // fixed state
    p_rasterizerdiscard = desc.rasterizer.discard;
    p_sampleshading = desc.rasterizer.sampleshading;
    p_fill = dx11_fill_mode(desc.rasterizer.fill);
    p_cull = desc.rasterizer.cull != GS_CULL_NONE;
    p_cullface = dx11_cull_face(desc.rasterizer.cull);
    p_pointsize = desc.rasterizer.pointsize;
    p_programpointsize = desc.rasterizer.programpointsize;
    p_colormask = desc.blend.writemask != 0;
    p_depthmask = desc.depthstencil.depthmask;
    p_depthtest = desc.depthstencil.depthtest != GS_DEPTHTEST_NONE;
    p_depthfunc = dx11_compare_func(desc.depthstencil.depthtest);
    p_blendseparate = desc.blend.separate;
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
    p_multisample = desc.rasterizer.multisample;

    return p_owner->error(GS_OK);
 }

int xGSStateImpl::attribLocation(const char *name) const
{
    // TODO
    return 0;
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

void xGSStateImpl::apply(const GScaps &caps)
{
    // TODO: xGSStateImpl::apply
}

void xGSStateImpl::bindNullProgram()
{
    // TODO: xGSStateImpl::bindNullProgram
}

void xGSStateImpl::ReleaseRendererResources()
{
    // TODO: xGSStateImpl::ReleaseRendererResources

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

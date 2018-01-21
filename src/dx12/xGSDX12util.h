/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/xgs

    dx12/xGSDX12util.h
        DirectX 12 specific utility functions and classes
*/

#pragma once

#include "xGS/xGS.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>


namespace xGS
{

    // forwards
    class xGSImpl;
    class xGSStateImpl;
    class xGSDataBufferImpl;
    class xGSTextureImpl;

    struct GScaps;
    struct GSParameterSet;


    struct GScaps
    {
        // TODO: define caps for DX12 implementation
    };


    inline DXGI_FORMAT dx12_index_type(GSenum type)
    {
        switch (type) {
            case GS_INDEX_16: return DXGI_FORMAT_R16_UINT;
            case GS_INDEX_32: return DXGI_FORMAT_R32_UINT;
        }

        return DXGI_FORMAT(0);
    }

    inline D3D12_PRIMITIVE_TOPOLOGY dx12_primitive_type(GSenum type, unsigned int patchverts = 0)
    {
        switch (type) {
            case GS_PRIM_POINTS:        return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            case GS_PRIM_LINES:         return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            case GS_PRIM_LINESTRIP:     return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            case GS_PRIM_TRIANGLES:     return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case GS_PRIM_TRIANGLESTRIP: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            //case GS_PRIM_TRIANGLEFAN:   return 0; // TODO: do we need FANs?
            case GS_PRIM_PATCHES:       return D3D12_PRIMITIVE_TOPOLOGY(D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST + patchverts);
        }

        return D3D12_PRIMITIVE_TOPOLOGY(0);
    }

    inline D3D12_TEXTURE_ADDRESS_MODE dx12_texture_wrap(GSenum wrap)
    {
        switch (wrap) {
            case GS_WRAP_CLAMP:  return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            case GS_WRAP_REPEAT: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }

        return D3D12_TEXTURE_ADDRESS_MODE(0);
    }

    inline D3D12_COMPARISON_FUNC dx12_compare_func(GSenum func)
    {
        switch (func) {
            case GS_DEPTHTEST_NEVER:    return D3D12_COMPARISON_FUNC_NEVER;
            case GS_DEPTHTEST_LESS:     return D3D12_COMPARISON_FUNC_LESS;
            case GS_DEPTHTEST_LEQUAL:   return D3D12_COMPARISON_FUNC_LESS_EQUAL;
            case GS_DEPTHTEST_EQUAL:    return D3D12_COMPARISON_FUNC_EQUAL;
            case GS_DEPTHTEST_NOTEQUAL: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
            case GS_DEPTHTEST_GREATER:  return D3D12_COMPARISON_FUNC_GREATER;
            case GS_DEPTHTEST_GEQUAL:   return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
            case GS_DEPTHTEST_ALWAYS:   return D3D12_COMPARISON_FUNC_ALWAYS;
        }

        return D3D12_COMPARISON_FUNC(0);
    }

    inline int dx12_fill_mode(GSenum mode)
    {
        switch (mode) {
            case GS_FILL_POINT:   return 0;
            case GS_FILL_LINE:    return 0;
            case GS_FILL_POLYGON: return 0;
        }

        return 0;
    }

    inline int dx12_cull_face(GSenum mode)
    {
        return mode == GS_CULL_CW ? 0 : 0;
    }

    inline int dx12_blend_eq(GSenum eq)
    {
        switch (eq) {
            case GS_BLEND_ADD:      return 0;
            case GS_BLEND_SUBTRACT: return 0;
            case GS_BLEND_MIN:      return 0;
            case GS_BLEND_MAX:      return 0;
        }

        return 0;
    }

    inline int dx12_blend_factor(GSenum f)
    {
        switch (f) {
            case GSBF_ZERO:             return 0;
            case GSBF_ONE:              return 0;
            case GSBF_SRCCOLOR:         return 0;
            case GSBF_INVSRCCOLOR:      return 0;
            case GSBF_DSTCOLOR:         return 0;
            case GSBF_INVDSTCOLOR:      return 0;
            case GSBF_SRCALPHA:         return 0;
            case GSBF_INVSRCALPHA:      return 0;
            case GSBF_DSTALPHA:         return 0;
            case GSBF_INVDSTALPHA:      return 0;
            case GSBF_CONSTANTCOLOR:    return 0;
            case GSBF_INVCONSTANTCOLOR: return 0;
            case GSBF_CONSTANTALPHA:    return 0;
            case GSBF_INVCONSTANTALPHA: return 0;
            case GSBF_SRCALPHA_SAT:     return 0;
            case GSBF_SRC1COLOR:        return 0;
            case GSBF_INVSRC1COLOR:     return 0;
            case GSBF_SRC1ALPHA:        return 0;
            case GSBF_INVSRC1ALPHA:     return 0;
        }

        return 0;
    }

#ifdef _DEBUG
    const char* uniform_type(int type);
#endif

    // internal parameters object implementation
    // which is used by state object to hold static resource bindings
    class GSParametersState
    {
    public:
        GSParametersState();

        GSerror allocate(xGSImpl *impl, xGSStateImpl *state, const GSParameterSet &set, const GSuniformbinding *uniforms, const GStexturebinding *textures, const GSconstantvalue *constants);
        void apply(const GScaps &caps, xGSImpl *impl, xGSStateImpl *state);

        void ReleaseRendererResources(xGSImpl *impl);

    protected:
        struct UniformBlockData
        {
            GSuint             index;
            GSuint             offset;
            GSuint             size;
            xGSDataBufferImpl *buffer;
        };

        struct TextureSlot
        {
            xGSTextureImpl *texture;
            GSuint          sampler;
        };

        struct ConstantValue
        {
            GSenum type;
            GSenum location;
            size_t offset;
        };

        typedef std::vector<UniformBlockData> UniformBlockDataSet;
        typedef std::vector<TextureSlot> TextureSet;
        typedef std::vector<ConstantValue> ConstantValues;
        typedef std::vector<char> ConstantMemory;

    protected:
        UniformBlockDataSet p_uniformblockdata;
        TextureSet          p_textures;
        GSuint              p_firstslot;
        ConstantValues      p_constants;
        ConstantMemory      p_constantmemory;
    };

} // namespace xGS

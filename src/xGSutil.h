/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    xGSutil.h
        Common utility functions and classes for implementations
*/

#pragma once

#include "xGS/xGS.h"
#include <vector>


namespace xGS
{

    static const GSuint GS_UNDEFINED = 0xFFFFFFFF;

    enum class DebugMessageLevel
    {
        Information,
        Warning,
        Error,
        SystemError
    };



    enum GSbits
    {
        b2   = 0x01,
        b4   = 0x02,
        b8   = 0x04,
        b16  = 0x08,
        b24  = 0x10,
        b32  = 0x20,
        b64  = 0x40,
        b128 = 0x80
    };

    enum GSsamples
    {
        s2  = 0x01,
        s4  = 0x02,
        s8  = 0x04,
        s16 = 0x08
    };

    struct GSpixelformat     // compact pixel format structure
    {
        GSint  pfColorBits;   // bits for color
        GSint  pfAlphaBits;   // bits for alpha channel
        GSint  pfDepthBits;   // bits for depth
        GSint  pfStencilBits; // bits for stencil
        GSint  pfMultisample; // AA type (samples count)
        bool   pfSRGB;
    };


    inline GSptr ptr_offset(GSptr ptr, GSint offset)
    {
        return reinterpret_cast<char*>(ptr) + offset;
    }


    inline GSenum ColorFormatFromPixelFormat(const GSpixelformat &fmt)
    {
        switch (fmt.pfColorBits) {
            case 32: return fmt.pfAlphaBits ? GS_COLOR_RGBA : GS_COLOR_RGBX;
        }

        return GS_NONE;
    }

    inline GSenum DepthFormatFromPixelFormat(const GSpixelformat &fmt)
    {
        if (fmt.pfDepthBits == 24 && fmt.pfStencilBits == 8) {
            return GS_DEPTHSTENCIL_D24S8;
        } else {
            switch (fmt.pfDepthBits) {
                case 16: return GS_DEPTH_16;
                case 24: return GS_DEPTH_24;
                case 32: return GS_DEPTH_32;
            }
            return GS_NONE;
        }
    }

    inline GSuint vertex_component_count(const GSvertexcomponent &c)
    {
        switch (c.type) {
            case GSVD_POSITION:  return 3;
            case GSVD_POSITIONW: return 4;
            case GSVD_VECTOR1:   return 1;
            case GSVD_VECTOR2:   return 2;
            case GSVD_VECTOR3:   return 3;
            case GSVD_VECTOR4:   return 4;

            default:
                return 0;
        }
    }

    inline GSuint vertex_component_size(const GSvertexcomponent &c)
    {
        switch (c.type) {
            case GSVD_POSITION:  return sizeof(float) * 3;
            case GSVD_POSITIONW: return sizeof(float) * 4;
            case GSVD_VECTOR1:   return sizeof(float) * 1;
            case GSVD_VECTOR2:   return sizeof(float) * 2;
            case GSVD_VECTOR3:   return sizeof(float) * 3;
            case GSVD_VECTOR4:   return sizeof(float) * 4;

            default:
                return 0;
        }
    }

    inline GSuint index_buffer_size(GSenum format, int count = 1)
    {
        int result = 0;

        switch (format) {
            case GS_INDEX_16: result = 2; break;
            case GS_INDEX_32: result = 4; break;
        }

        return result * count;
    }

    inline GSuint align(GSuint value, GSuint align)
    {
        GSuint result = value / align;
        if (value % align) {
            ++result;
        }
        return result * align;
    }

    inline GSptr buffercast(GSuint addr)
    {
        return GSptr(reinterpret_cast<char*>(0) + addr);
    }

    inline GSuint buffercast(GSptr addr)
    {
        // TODO: check more robust solution for that
        GSuint *ints = reinterpret_cast<GSuint*>(&addr);
        return *ints;
    }

    class GSvertexdecl
    {
    public:
        GSvertexdecl();
        GSvertexdecl(const GSvertexcomponent *decl);

        GSuint buffer_size(int count = 1) const;
        GSbool dynamic() const { return p_dynamic; }
        const std::vector<GSvertexcomponent>& declaration() const { return p_decl; }

        void initialize(const GSvertexcomponent *decl);

    protected:
        std::vector<GSvertexcomponent> p_decl;
        bool                           p_dynamic;
    };


    struct GSParameterSet
    {
        GSenum settype;
        GSuint first;
        GSuint onepastlast;
        GSuint firstsampler;
        GSuint onepastlastsampler;
        GSuint constantcount;
    };

} // namespace xGS

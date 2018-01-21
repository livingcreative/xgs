/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    xGSimplbase.h
        xGS API object common implementation class header
            defines main API (system) object common structure
*/

#pragma once

#include "xGS/xGS.h"
#include "xGSutil.h"
#include "IUnknownImpl.h"
#include <unordered_set>


namespace xGS
{

    class xGSGeometryImpl;
    class xGSGeometryBufferImpl;
    class xGSDataBufferImpl;
    class xGSTextureImpl;
    class xGSFrameBufferImpl;
    class xGSStateImpl;
    class xGSInputImpl;
    class xGSParametersImpl;


    class xGSImplBase : public xGSIUnknownImpl<xGSSystem>
    {
    public:
        xGSImplBase();
        ~xGSImplBase() override;

    public:
        // TODO: make GScaps common for all platforms
        //          split caps into two parts - xGS common caps, exposed by API
        //          and implementation specific caps
        // TODO: think of samplers and how to make common interface for all platforms
        //          if it's actually needed (it might be platform specific though)

        // memory management
        GSptr allocate(GSuint size);
        void free(GSptr &memory);

        // error set-up
        bool error(GSerror code, DebugMessageLevel level = DebugMessageLevel::Error);

        // debug logging
        void debug(DebugMessageLevel level, const char *format, ...);

        // children objects add/remove
        template <typename T> void AddObject(T *object);
        template <typename T> void RemoveObject(T *object);

    protected:
        enum SystemState
        {
            SYSTEM_NOTREADY,
            SYSTEM_READY,
            RENDERER_READY,
            CAPTURE
        };

        typedef std::unordered_set<xGSGeometryImpl*>       GeometryList;
        typedef std::unordered_set<xGSGeometryBufferImpl*> GeometryBufferList;
        typedef std::unordered_set<xGSDataBufferImpl*>     DataBufferList;
        typedef std::unordered_set<xGSTextureImpl*>        TextureList;
        typedef std::unordered_set<xGSFrameBufferImpl*>    FrameBufferList;
        typedef std::unordered_set<xGSStateImpl*>          StateList;
        typedef std::unordered_set<xGSInputImpl*>          InputList;
        typedef std::unordered_set<xGSParametersImpl*>     ParametersList;

        static IxGS            gs;

        SystemState            p_systemstate;
        GSerror                p_error;

        GeometryList           p_geometrylist;
        GeometryBufferList     p_geometrybufferlist;
        DataBufferList         p_databufferlist;
        TextureList            p_texturelist;
        FrameBufferList        p_framebufferlist;
        StateList              p_statelist;
        InputList              p_inputlist;
        ParametersList         p_parameterslist;

        xGSFrameBufferImpl    *p_rendertarget;
        GSenum                 p_colorformats[GS_MAX_FB_COLORTARGETS];
        GSenum                 p_depthstencilformat;

        xGSStateImpl          *p_state;
        xGSInputImpl          *p_input;
        xGSParametersImpl     *p_parameters[GS_MAX_PARAMETER_SETS];

        xGSGeometryBufferImpl *p_capturebuffer;

        xGSGeometryBufferImpl *p_immediatebuffer;

#ifdef _DEBUG
        GSuint                 dbg_memory_allocs;
#endif
    };

} // namespace xGS

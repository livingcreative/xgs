/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    xGSimplBase.h
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
        GSbool error(GSerror code, DebugMessageLevel level = DebugMessageLevel::Error);

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

        GSbool ValidateState(SystemState requiredstate, bool exactmatch, bool matchimmediate, bool requiredimmediate);

        template <typename T>
        inline void CheckObjectList(const T &list, const std::string &listname);

        template <typename T>
        inline void ReleaseObjectList(T &list, const std::string &listname);

        template <typename T, typename C>
        void AttachObject(const C &caps, T &attachpoint, T object)
        {
            if (attachpoint) {
                attachpoint->Release();
            }

            attachpoint = object;

            if (attachpoint) {
                attachpoint->AddRef();
                attachpoint->apply(caps);
            }
        }

        // NOTE: State and draw commands not implemented here even if they are
        //          common for any implementation, they will be moved into
        //          rendering list soon

        void CleanupObjects();

        // TODO: this needs to be moved in common IxGS interface implementation
        template <typename C>
        void SetState(const C &caps, xGSStateImpl *state)
        {
            AttachObject(caps, p_state, state);
            AttachObject(caps, p_input, static_cast<xGSInputImpl*>(nullptr));
            for (size_t n = 0; n < GS_MAX_PARAMETER_SETS; ++n) {
                AttachObject(caps, p_parameters[n], static_cast<xGSParametersImpl*>(nullptr));
            }
#ifdef _DEBUG
            if (!p_state) {
                xGSStateImpl::bindNullProgram();
            }
#endif
        }

        // TODO: this needs to be moved in common IxGS interface implementation
        //       and in render list eventually
        template <typename T>
        GSbool Draw(IxGSGeometry geometry_to_draw, const T &drawer);
        template <typename T>
        GSbool MultiDraw(IxGSGeometry *geometries_to_draw, GSuint count, const T &drawer);

    protected:
        typedef std::unordered_set<xGSGeometryImpl*>       GeometryList;
        typedef std::unordered_set<xGSGeometryBufferImpl*> GeometryBufferList;
        typedef std::unordered_set<xGSDataBufferImpl*>     DataBufferList;
        typedef std::unordered_set<xGSTextureImpl*>        TextureList;
        typedef std::unordered_set<xGSFrameBufferImpl*>    FrameBufferList;
        typedef std::unordered_set<xGSStateImpl*>          StateList;
        typedef std::unordered_set<xGSInputImpl*>          InputList;
        typedef std::unordered_set<xGSParametersImpl*>     ParametersList;

        static IxGS            gs;

        GSerror                p_error;
        SystemState            p_systemstate;

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

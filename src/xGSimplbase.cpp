/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    xGSimplbase.cpp
        xGS API object common implementation class
*/

#include "xGSimplbase.h"
#include "xGSgeometry.h"
#include "xGSgeometrybuffer.h"
#include "xGSdatabuffer.h"
#include "xGStexture.h"
#include "xGSframebuffer.h"
#include "xGSstate.h"
#include "xGSinput.h"
#include "xGSparameters.h"
#include <cstdlib>

#ifdef _DEBUG
    #include <cstdarg>

    #ifdef WIN32
        #include <Windows.h>
    #endif

    #ifdef WIN32
        #define DEBUG_PRINT(T) OutputDebugStringA(T)
    #endif

    #ifdef __APPLE__
        #define DEBUG_PRINT(T) printf(T)
    #endif
#endif



using namespace xGS;


// xGS system object instance
IxGS xGSImplBase::gs = nullptr;


xGSImplBase::xGSImplBase() :
    p_systemstate(SYSTEM_NOTREADY),

    p_geometrylist(),
    p_geometrybufferlist(),
    p_databufferlist(),
    p_texturelist(),
    p_framebufferlist(),
    p_statelist(),
    p_inputlist(),
    p_parameterslist(),

    p_rendertarget(nullptr),
    p_state(nullptr),
    p_input(nullptr),
    p_capturebuffer(nullptr),
    p_immediatebuffer(nullptr)
{
    for (size_t n = 0; n < GS_MAX_PARAMETER_SETS; ++n) {
        p_parameters[n] = nullptr;
    }

#ifdef _DEBUG
    dbg_memory_allocs = 0;
#endif
}

xGSImplBase::~xGSImplBase()
{
    gs = nullptr;
}

GSptr xGSImplBase::allocate(GSuint size)
{
#if defined(_DEBUG) || defined(DEBUG)
    dbg_memory_allocs += size;
#endif
    return malloc(size_t(size));
}

void xGSImplBase::free(GSptr &memory)
{
#if defined(_DEBUG) || defined(DEBUG)
    dbg_memory_allocs -= GSuint(_msize(memory));
#endif
    ::free(memory);
    memory = nullptr;
}

bool xGSImplBase::error(GSerror code, DebugMessageLevel level)
{
    p_error = code;
    if (p_error != GS_OK) {
        debug(level, "Code: 0x%X\n", code);

#ifdef _MSC_VER
        //_CrtDbgBreak();
#endif
    }

    return code == GS_OK;
}

void xGSImplBase::debug(DebugMessageLevel level, const char *format, ...)
{
#ifdef _DEBUG
    va_list args;
    va_start(args, format);

    if (*format && (*format != '\n')) {
        switch (level) {
            case DebugMessageLevel::Information:
                DEBUG_PRINT("xGS INFO: ");
                break;

            case DebugMessageLevel::Warning:
                DEBUG_PRINT("xGS WARNING: ");
                break;

            case DebugMessageLevel::Error:
                DEBUG_PRINT("xGS ERROR: ");
                break;

            case DebugMessageLevel::SystemError:
                DEBUG_PRINT("xGS SYSTEM ERROR: ");
                break;
        }
    }

#ifdef WIN32
    char buf[4096];
    vsprintf_s(buf, format, args);

    OutputDebugStringA(buf);
#endif

#ifdef __APPLE__
    vprintf(format, args);
#endif

    va_end(args);
#endif
}

#define GS_ADD_REMOVE_OBJECT_IMPL(list, type)\
template <> void xGSImplBase::AddObject(type *object)\
{\
    list.insert(object);\
}\
\
template <> void xGSImplBase::RemoveObject(type *object)\
{\
    list.erase(object);\
}

GS_ADD_REMOVE_OBJECT_IMPL(p_geometrylist, xGSGeometryImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_geometrybufferlist, IxGSGeometryBufferImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_databufferlist, IxGSDataBufferImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_texturelist, xGSTextureImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_framebufferlist, IxGSFrameBufferImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_statelist, xGSStateImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_inputlist, IxGSInputImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_parameterslist, xGSParametersImpl)

#undef GS_ADD_REMOVE_OBJECT_IMPL

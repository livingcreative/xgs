/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    xGSimplBase.cpp
        xGS API object common implementation class
*/

#include "xGSimplBase.h"
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

GSbool xGSImplBase::error(GSerror code, DebugMessageLevel level)
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
GS_ADD_REMOVE_OBJECT_IMPL(p_geometrybufferlist, xGSGeometryBufferImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_databufferlist, xGSDataBufferImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_texturelist, xGSTextureImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_framebufferlist, xGSFrameBufferImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_statelist, xGSStateImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_inputlist, xGSInputImpl)
GS_ADD_REMOVE_OBJECT_IMPL(p_parameterslist, xGSParametersImpl)

#undef GS_ADD_REMOVE_OBJECT_IMPL

GSbool xGSImplBase::ValidateState(SystemState requiredstate, bool exactmatch, bool matchimmediate, bool requiredimmediate)
{
    if (p_systemstate == SYSTEM_NOTREADY) {
        return error(GSE_SYSTEMNOTREADY);
    }

    if (p_systemstate == SYSTEM_READY && requiredstate > p_systemstate) {
        return error(GSE_RENDERERNOTREADY);
    }

    if (p_systemstate == RENDERER_READY && requiredstate > p_systemstate) {
        return error(GSE_INVALIDOPERATION);
    }

    if (exactmatch && p_systemstate != requiredstate) {
        return error(GSE_INVALIDOPERATION);
    }

    bool immediatemode = p_immediatebuffer != nullptr;
    if (matchimmediate && requiredimmediate != immediatemode) {
        return error(GSE_INVALIDOPERATION);
    }

    return GS_TRUE;
}

template <typename T>
void xGSImplBase::CheckObjectList(const T &list, const std::string &listname)
{
#ifdef _DEBUG
    if (list.size()) {
        debug(DebugMessageLevel::Warning, "Not all resources released in list %s ar renderer destroy", listname.c_str());
#ifdef _MSC_VER
        _CrtDbgBreak();
#endif
    }
#endif
}

template <typename T>
void xGSImplBase::ReleaseObjectList(T &list, const std::string &listname)
{
    CheckObjectList(list, listname);
    for (auto &object : list) {
        object->ReleaseRendererResources();
        object->DetachFromRenderer();
    }
}

void xGSImplBase::CleanupObjects()
{
    ::Release(p_state);
    ::Release(p_input);
    for (size_t n = 0; n < GS_MAX_PARAMETER_SETS; ++n) {
        ::Release(p_parameters[n]);
    }
    ::Release(p_rendertarget);

    ReleaseObjectList(p_statelist, "State");
    ReleaseObjectList(p_inputlist, "Input");
    ReleaseObjectList(p_parameterslist, "Parameters");
    ReleaseObjectList(p_geometrylist, "Geomtery");
    ReleaseObjectList(p_framebufferlist, "Framebuffer");
    ReleaseObjectList(p_geometrybufferlist, "GeomteryBuffer");
    ReleaseObjectList(p_databufferlist, "DataBuffer");
    ReleaseObjectList(p_texturelist, "Texture");
}

template <typename T>
GSbool xGSImplBase::Draw(IxGSGeometry geometry_to_draw, const T &drawer)
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!geometry_to_draw) {
        return error(GSE_INVALIDOBJECT);
    }

    if (!p_state) {
        return error(GSE_INVALIDSTATE);
    }

    xGSGeometryImpl *geometry = static_cast<xGSGeometryImpl*>(geometry_to_draw);
    xGSGeometryBufferImpl *buffer = geometry->buffer();

#ifdef _DEBUG
    size_t primaryslot = p_state->inputPrimarySlot();
    xGSGeometryBufferImpl *boundbuffer =
        primaryslot == GS_UNDEFINED ? nullptr : p_state->input(primaryslot).buffer;
    if (p_input && boundbuffer == nullptr)  {
        boundbuffer = p_input->primaryBuffer();
    }
    if (buffer != boundbuffer) {
        return error(GSE_INVALIDOBJECT);
    }
#endif

    geometry->setup();

    if (geometry->indexFormat() == GS_INDEX_NONE) {
        drawer.DrawArrays(geometry->type(), geometry->baseVertex(), geometry->vertexCount());
    } else {
        drawer.DrawElementsBaseVertex(
            geometry->type(),
            geometry->indexCount(), buffer->indexFormat(),
            geometry->indexPtr(), geometry->baseVertex()
        );
    }

    return error(GS_OK);
}

template <typename T>
GSbool xGSImplBase::MultiDraw(IxGSGeometry *geometries_to_draw, GSuint count, const T &drawer)
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!geometries_to_draw) {
        return error(GSE_INVALIDVALUE);
    }

    if (!p_state) {
        return error(GSE_INVALIDSTATE);
    }

#ifdef _DEBUG
    size_t primaryslot = p_state->inputPrimarySlot();
    xGSGeometryBufferImpl *boundbuffer =
        primaryslot == GS_UNDEFINED ? nullptr : p_state->input(primaryslot).buffer;
    if (p_input && boundbuffer == nullptr)  {
        boundbuffer = p_input->primaryBuffer();
    }
#endif

    GSenum primtype = GS_NONE;
    GSenum indextype = GS_NONE;
    int first[4096];
    int counts[4096];
    GSptr indices[4096];
    size_t current = 0;

    for (GSuint n = 0; n < count; ++n) {
        xGSGeometryImpl *geometry = static_cast<xGSGeometryImpl*>(geometries_to_draw[n]);

#ifdef _DEBUG
        xGSGeometryBufferImpl *buffer = geometry->buffer();
        if (buffer != boundbuffer) {
            return error(GSE_INVALIDOBJECT);
        }
#endif

        first[current] = geometry->baseVertex();
        counts[current] = geometry->vertexCount(); // TODO: index count for indexed
        indices[current] = geometry->indexPtr(); // TODO: only for indexed
        ++current;

        // TODO: geometry setup, geometry type, index type
        //geometry->setup();
    }

    if (indextype == GS_INDEX_NONE) {
        drawer.MultiDrawArrays(primtype, first, counts, count);
    } else {
        drawer.MultiDrawElementsBaseVertex(
            primtype, counts,
            indextype, indices, count, first
        );
    }

    return error(GS_OK);
}

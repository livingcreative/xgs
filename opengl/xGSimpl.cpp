/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    xGSimpl.cpp
        xGS API object implementation class
*/

#include "xGSimpl.h"
#include "xGScontext.h"
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

    #ifdef __APPLE__
        #define CALLBACK
    #endif
#endif


using namespace xGS;


#if defined(_DEBUG) && defined(GS_CONFIG_DEBUG_CALLBACK)
void CALLBACK errorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, GLvoid *userParam)
{
    reinterpret_cast<xGSImpl*>(userParam)->debug(
        DebugMessageLevel::Error, 
        "CB Error:\n\tSource: %i\n\tType: %i\n\tId: %d\n\tSeverity: %i\n\tMessage: %s\n",
        source, type, id, severity, message
    );

#ifdef _MSC_VER
    _CrtDbgBreak();
#endif
}
#endif

xGSImpl::xGSImpl(xGScontext *context) :
    p_context(context),

    p_systemstate(SYSTEM_NOTREADY),

    p_geometrylist(),
    p_geometrybufferlist(),
    p_databufferlist(),
    p_texturelist(),
    p_samplerlist(),
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

    p_error = context->Initialize();
    if (p_error != GS_OK) {
        return;
    }

    p_systemstate = SYSTEM_READY;

#ifdef _DEBUG
    dbg_memory_allocs = 0;
#endif
}

xGSImpl::~xGSImpl()
{
    if (p_systemstate == CAPTURE) {
        EndCapture(nullptr);
    }
    if (p_systemstate == RENDERER_READY) {
        DestroyRenderer(true);
    }
    gs = nullptr;
}


GSptr xGSImpl::allocate(GSint size)
{
#if defined(_DEBUG) && defined(WIN32)
    dbg_memory_allocs += size;
#endif
    return malloc(size_t(size));
}

void xGSImpl::free(GSptr &memory)
{
#if defined(_DEBUG) && defined(WIN32)
    dbg_memory_allocs -= GSint(_msize(memory));
#endif
    ::free(memory);
    memory = nullptr;
}

GSbool xGSImpl::error(GSerror code, DebugMessageLevel level)
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

#ifdef _DEBUG
    #ifdef WIN32
        #define DEBUG_PRINT(T) OutputDebugStringA(T)
    #endif

    #ifdef __APPLE__
        #define DEBUG_PRINT(T) printf(T)
    #endif
#endif

void xGSImpl::debug(DebugMessageLevel level, const char *format, ...)
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

#ifdef _DEBUG
void xGSImpl::debugTrackGLError(const char *text)
{
    GLenum error = glGetError();
    if (error) {
        const char *formatstr = text ?
            "xGS: OpenGL error %i at %s\n" :
            "xGS: OpenGL error %i\n";

#ifdef WIN32
        char buf[1024];
        sprintf_s(buf, formatstr, error, text);

        OutputDebugStringA(buf);
#endif

#ifdef __APPLE__
        printf(formatstr, error, text);
#endif
    }
}
#endif

GSbool xGSImpl::CreateRenderer(const GSrendererdescription &desc)
{
    if (!ValidateState(SYSTEM_READY, true, false, false)) {
        return GS_FALSE;
    }

    // videomode
    //

    p_error = p_context->CreateRenderer(desc);
    if (p_error != GS_OK) {
        return error(p_error);
    }

#ifdef _DEBUG
    debug(DebugMessageLevel::Information, "GL_VENDOR: %s\n", glGetString(GL_VENDOR));
    debug(DebugMessageLevel::Information, "GL_VERSION: %s\n", glGetString(GL_VERSION));

#ifdef GS_CONFIG_LIST_EXTENSIONS
    GLint numexts = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numexts);

    debug(DebugMessageLevel::Information, "GL_EXTENSIONS (%i):\n", numexts);
    for (int n = 0; n < numexts; ++n) {
        const char *cext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, n));
        debug(DebugMessageLevel::Information, "\t%s\n", cext);
    }
#endif

#ifdef GS_CONFIG_DEBUG_CALLBACK
    if (glDebugMessageCallback) {
        glDebugMessageCallback(errorCallback, this);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }
#endif

#endif

#if 0
    // check must have extensions
    bool neededexts = 
        GLEW_ARB_shading_language_100 &&  // core
        GLEW_ARB_shader_objects &&        // core
        GLEW_ARB_vertex_array_object &&   // core
        GLEW_ARB_framebuffer_object &&    // core
        GLEW_ARB_uniform_buffer_object && // core
        GLEW_ARB_pixel_buffer_object &&   // core
        GLEW_ARB_sampler_objects;         // core

    // TODO: add must have exts
    //      ARB_draw_elements_base_vertex
    //      ARB_draw_instanced

    // TODO: add support for
    //      ARB_vertex_attrib_binding
    //      ARB_bindless_texture
    //      ARB_sparse_texture
    //      ARB_sparse_buffer
    //      ARB_bindless_texture

    if (!neededexts) {
#ifdef _DEBUG
        if (!GLEW_ARB_shading_language_100) {
            debug(DebugMessageLevel::SystemError, "ARB_shading_language_100 not supported!\n");
        }
        if (!GLEW_ARB_shader_objects) {
            debug(DebugMessageLevel::SystemError, "ARB_shader_objects not supported!\n");
        }
        if (!GLEW_ARB_vertex_array_object) {
            debug(DebugMessageLevel::SystemError, "ARB_vertex_array_object not supported!\n");
        }
        if (!GLEW_ARB_framebuffer_object) {
            debug(DebugMessageLevel::SystemError, "ARB_framebuffer_object not supported!\n");
        }
        if (!GLEW_ARB_uniform_buffer_object) {
            debug(DebugMessageLevel::SystemError, "ARB_uniform_buffer_object not supported!\n");
        }
        if (!GLEW_ARB_pixel_buffer_object) {
            debug(DebugMessageLevel::SystemError, "ARB_pixel_buffer_object not supported!\n");
        }
        if (!GLEW_ARB_sampler_objects) {
            debug(DebugMessageLevel::SystemError, "ARB_sampler_objects not supported!\n");
        }
#endif
        return error(GSE_INCOMPATIBLE);
    }
#endif

    glGenQueries(1, &p_capturequery);

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &p_caps.max_active_attribs);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &p_caps.max_texture_units);

    // TODO: review caps (some are static, not run-time)
    p_caps.multi_bind           = GS_CAPS_MULTI_BIND;
    p_caps.multi_blend          = GS_CAPS_MULTI_BLEND;
    p_caps.vertex_format        = GS_CAPS_VERTEX_FORMAT;
    p_caps.texture_srgb         = GS_CAPS_TEXTURE_SRGB;
    p_caps.texture_float        = GS_CAPS_TEXTURE_FLOAT;
    p_caps.texture_depth        = GS_CAPS_TEXTURE_DEPTH;
    p_caps.texture_depthstencil = GS_CAPS_TEXTURE_DEPTHSTENCIL;

    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &p_caps.ubo_alignment);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &p_caps.max_ubo_size);

#ifdef _DEBUG
    debug(DebugMessageLevel::Information, "CAPS: max_active_attribs:   %i\n", p_caps.max_active_attribs);
    debug(DebugMessageLevel::Information, "CAPS: max_texture_units:    %i\n", p_caps.max_texture_units);
    debug(DebugMessageLevel::Information, "CAPS: multibind:            %s\n", p_caps.multi_bind ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: multiblend:           %s\n", p_caps.multi_blend ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: vertex format:        %s\n", p_caps.vertex_format ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: ubo_alignment:        %i\n", p_caps.ubo_alignment);
    debug(DebugMessageLevel::Information, "CAPS: max_ubo_size:         %i\n", p_caps.max_ubo_size);
    debug(DebugMessageLevel::Information, "CAPS: texture sRGB:         %s\n", p_caps.texture_srgb ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: texture float:        %s\n", p_caps.texture_float ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: texture depth:        %s\n", p_caps.texture_depth ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: texture depthstencil: %s\n", p_caps.texture_depthstencil ? "Yes" : "No");
#endif

    AddTextureFormatDescriptor(GS_COLOR_RGBX, 4, GL_RGB8, GL_BGRA, GL_UNSIGNED_BYTE);
    AddTextureFormatDescriptor(GS_COLOR_RGBA, 4, GL_RGBA8, GL_BGRA, GL_UNSIGNED_BYTE);
    if (p_caps.texture_srgb) {
        AddTextureFormatDescriptor(GS_COLOR_S_RGBX, 4, GL_SRGB8, GL_BGRA, GL_UNSIGNED_BYTE);
        AddTextureFormatDescriptor(GS_COLOR_S_RGBA, 4, GL_SRGB8_ALPHA8, GL_BGRA, GL_UNSIGNED_BYTE);
    }
    if (p_caps.texture_float) {
        AddTextureFormatDescriptor(GS_COLOR_RGBA_HALFFLOAT, 8, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT);
        AddTextureFormatDescriptor(GS_COLOR_RGBA_FLOAT, 16, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        AddTextureFormatDescriptor(GS_COLOR_RGBX_HALFFLOAT, 8, GL_RGB16F, GL_RGB, GL_HALF_FLOAT);
        AddTextureFormatDescriptor(GS_COLOR_RGBX_FLOAT, 16, GL_RGB32F, GL_RGB, GL_FLOAT);
    }
    if (p_caps.texture_depth) {
        AddTextureFormatDescriptor(GS_DEPTH_16, 2, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT);
        AddTextureFormatDescriptor(GS_DEPTH_24, 3, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT);
    }
    if (p_caps.texture_depthstencil) {
        AddTextureFormatDescriptor(GS_DEPTHSTENCIL_D24S8, 4, GL_DEPTH24_STENCIL8, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT);
    }

    // turn sRGB for default frame buffer if framebuffer was created with sRGB support
    if (p_context->RenderTargetFormat().pfSRGB) {
        glEnable(GL_FRAMEBUFFER_SRGB);
    }

    glFrontFace(GL_CW);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glEnable(GL_POLYGON_OFFSET_FILL);

    // TEMP
    glPointSize(10.0f);


    memset(p_timerqueries, 0, sizeof(p_timerqueries));
    p_timerindex = 0;
    p_timerscount = 0;

#ifdef _DEBUG
    debugTrackGLError("xGSImpl::CreateRenderer");
#endif

    DefaultRTFormats();

    p_systemstate = RENDERER_READY;

    return error(GS_OK);
}

GSbool xGSImpl::DestroyRenderer(GSbool restorevideomode)
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

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

    for (auto &sampler : p_samplerlist) {
        glDeleteSamplers(1, &sampler.sampler);
    }

    glDeleteQueries(1, &p_capturequery);

    glDeleteQueries(p_timerscount, p_timerqueries);

    p_context->DestroyRenderer();

    p_systemstate = SYSTEM_READY;

    return GS_TRUE;
}


#define GS_CREATE_OBJECT(typeconst, impltype, desctype)\
        case typeconst: {\
            impltype *object = impltype::create(this);\
            if (object->allocate(*reinterpret_cast<const desctype*>(desc))) {\
                *result = object;\
                return error(GS_OK);\
            } else {\
                object->Release();\
                return GS_FALSE;\
            }\
        }

GSbool xGSImpl::CreateObject(GSenum type, const void *desc, void **result)
{
    // TODO: make refcounting and adding to object list only after successful creation
    switch (type) {
        GS_CREATE_OBJECT(GS_OBJECTTYPE_GEOMETRY, xGSGeometryImpl, GSgeometrydescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_GEOMETRYBUFFER, xGSGeometryBufferImpl, GSgeometrybufferdescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_DATABUFFER, xGSDataBufferImpl, GSdatabufferdescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_TEXTURE, xGSTextureImpl, GStexturedescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_FRAMEBUFFER, xGSFrameBufferImpl, GSframebufferdescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_STATE, xGSStateImpl, GSstatedescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_INPUT, xGSInputImpl, GSinputdescription)
        GS_CREATE_OBJECT(GS_OBJECTTYPE_PARAMETERS, xGSParametersImpl, GSparametersdescription)
    }

    return error(GSE_INVALIDENUM);
}

#undef GS_CREATE_OBJECT

GSbool xGSImpl::CreateSamplers(const GSsamplerdescription *samplers, GSuint count)
{
    GSuint references = 0;
    for (auto &s : p_samplerlist) {
        references += s.refcount;
    }

    if (references) {
        return error(GSE_INVALIDOPERATION);
    }

    for (auto &s : p_samplerlist) {
        glDeleteSamplers(1, &s.sampler);
    }

    p_samplerlist.resize(count);
    for (size_t n = 0; n < count; ++n) {
        Sampler &s = p_samplerlist[n];

        glGenSamplers(1, &s.sampler);
        s.refcount = 0;

        // TODO: check values

        glSamplerParameteri(s.sampler, GL_TEXTURE_WRAP_S, gl_texture_wrap(samplers->wrapu));
        glSamplerParameteri(s.sampler, GL_TEXTURE_WRAP_T, gl_texture_wrap(samplers->wrapv));
        glSamplerParameteri(s.sampler, GL_TEXTURE_WRAP_R, gl_texture_wrap(samplers->wrapw));

        glSamplerParameteri(s.sampler, GL_TEXTURE_MIN_LOD, samplers->minlod);
        glSamplerParameteri(s.sampler, GL_TEXTURE_MAX_LOD, samplers->maxlod);
        glSamplerParameterf(s.sampler, GL_TEXTURE_LOD_BIAS, samplers->bias);

        switch (samplers->filter) {
            case GS_FILTER_NEAREST:
                glSamplerParameteri(s.sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glSamplerParameteri(s.sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                break;

            case GS_FILTER_LINEAR:
                glSamplerParameteri(s.sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glSamplerParameteri(s.sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                break;

            case GS_FILTER_TRILINEAR:
                glSamplerParameteri(s.sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glSamplerParameteri(s.sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                break;
        }

        if (samplers->depthcompare) {
            glSamplerParameteri(s.sampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glSamplerParameteri(s.sampler, GL_TEXTURE_COMPARE_FUNC, gl_compare_func(samplers->depthcompare));
        }

        ++samplers;
    }

    return error(GS_OK);
}


GSbool xGSImpl::GetRenderTargetSize(GSsize &size)
{
    if (!ValidateState(RENDERER_READY, false, false, false)) {
        return GS_FALSE;
    }
    RenderTargetSize(size);
    return error(GS_OK);
}


GSbool xGSImpl::Clear(GSbool color, GSbool depth, GSbool stencil, const GScolor &colorvalue, float depthvalue, GSdword stencilvalue)
{
    // NOTE: clear doesn't work if rasterizer discard enabled
    //       depth buffer is not cleared when depth mask is FALSE

    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    GLuint mask = 0;

    if (color) {
        mask |= GL_COLOR_BUFFER_BIT;
        glClearColor(colorvalue.r, colorvalue.g, colorvalue.b, colorvalue.a);
    }

    if (depth) {
        mask |= GL_DEPTH_BUFFER_BIT;
        glClearDepth(depthvalue);
    }

    if (stencil) {
        mask |= GL_STENCIL_BUFFER_BIT;
        glClearStencil(stencilvalue);
    }

    glClear(mask);

    return error(GS_OK);
}

GSbool xGSImpl::Display()
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    // TODO: think about resetting timers here
    p_timerindex = 0;

    return p_context->Display();
}


GSbool xGSImpl::SetRenderTarget(IxGSFrameBuffer rendertarget)
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    if (p_rendertarget) {
        p_rendertarget->unbind();
        p_rendertarget->Release();
    }

    p_rendertarget = static_cast<xGSFrameBufferImpl*>(rendertarget);

    if (p_rendertarget) {
        p_rendertarget->AddRef();
        p_rendertarget->bind();
    }

    bool srgb = false;

    if (p_rendertarget == nullptr) {
        DefaultRTFormats();
        srgb = p_context->RenderTargetFormat().pfSRGB;
    } else {
        p_rendertarget->getformats(p_colorformats, p_depthstencilformat);
        srgb = p_rendertarget->srgb();
    }

    srgb ? glEnable(GL_FRAMEBUFFER_SRGB) : glDisable(GL_FRAMEBUFFER_SRGB);

    // reset current state, after RT change any state should be rebound
    SetState(static_cast<xGSStateImpl*>(nullptr));

    return error(GS_OK);
}

GSbool xGSImpl::SetViewport(const GSrect &viewport)
{
    // NOTE: actually there's no dependency on immediate mode
    //          consider ignore immediate mode
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    GSsize sz;
    RenderTargetSize(sz);

    glViewport(
        viewport.left, sz.height - viewport.height - viewport.top,
        viewport.width, viewport.height
    );

    return error(GS_OK);
}

GSbool xGSImpl::SetState(IxGSState state)
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    if (!state) {
        return error(GSE_INVALIDOBJECT);
    }

    xGSStateImpl *stateimpl = static_cast<xGSStateImpl*>(state);

    if (!stateimpl->allocated()) {
        return error(GSE_INVALIDOBJECT);
    }

    if (!stateimpl->validate(p_colorformats, p_depthstencilformat)) {
        return error(GSE_INVALIDOBJECT);
    }

    SetState(stateimpl);

    return error(GS_OK);
}

GSbool xGSImpl::SetInput(IxGSInput input)
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    xGSInputImpl *inputimpl = static_cast<xGSInputImpl*>(input);
    if (inputimpl == nullptr || !inputimpl->allocated()) {
        return error(GSE_INVALIDOBJECT);
    }

    if (p_state == nullptr || inputimpl->state() != p_state) {
        return error(GSE_INVALIDOPERATION);
    }

    AttachObject(p_input, inputimpl);

    return error(GS_OK);
}

GSbool xGSImpl::SetParameters(IxGSParameters parameters)
{
    // TODO: ability to change parameters while being in immediate mode
    //      actually parameters change can be recorded into immediate sequence
    //      and executed later, when immediate buffer will be flushed
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!parameters) {
        return error(GSE_INVALIDOBJECT);
    }

    xGSParametersImpl *parametersimpl = static_cast<xGSParametersImpl*>(parameters);

    if (!parametersimpl->allocated()) {
        return error(GSE_INVALIDOBJECT);
    }

    if (p_state == nullptr || parametersimpl->state() != p_state) {
        return error(GSE_INVALIDSTATE);
    }

    AttachObject(p_parameters[parametersimpl->setindex()], parametersimpl);

    return error(GS_OK);
}

GSbool xGSImpl::SetStencilReference(GSuint ref)
{
    // TODO: same as SetParameters behaviour
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!p_state) {
        return error(GSE_INVALIDSTATE);
    }

    // TODO

    return error(GSE_UNIMPLEMENTED);
}

GSbool xGSImpl::SetBlendColor(const GScolor &color)
{
    // TODO: same as SetParameters behaviour
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!p_state) {
        return error(GSE_INVALIDSTATE);
    }

    glBlendColor(color.r, color.g, color.b, color.a);

    return error(GSE_UNIMPLEMENTED);
}

GSbool xGSImpl::SetUniformValue(GSenum set, GSenum slot, GSenum type, const void *value)
{
    // TODO: same as SetParameters behaviour
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!p_state) {
        return error(GSE_INVALIDSTATE);
    }

    GSuint setindex = set - GSPS_0;
    if (setindex >= p_state->parameterSetCount()) {
        return error(GSE_INVALIDENUM);
    }

    const GSParameterSet &paramset = p_state->parameterSet(setindex);

    GSuint slotindex = slot - GSPS_0 + paramset.first;
    if (slotindex >= paramset.onepastlast) {
        return error(GSE_INVALIDENUM);
    }

    const xGSStateImpl::ParameterSlot &paramslot = p_state->parameterSlot(slotindex);

    if (paramslot.type != GSPD_CONSTANT) {
        return error(GSE_INVALIDOPERATION);
    }

    // TODO: check type or remove type parameter?
    if (paramslot.location != GS_DEFAULT) {
        switch (type) {
            case GSU_SCALAR:
                glUniform1fv(paramslot.location, 1, reinterpret_cast<const GLfloat*>(value));
                break;

            case GSU_VEC2:
                glUniform2fv(paramslot.location, 1, reinterpret_cast<const GLfloat*>(value));
                break;

            case GSU_VEC3:
                glUniform3fv(paramslot.location, 1, reinterpret_cast<const GLfloat*>(value));
                break;

            case GSU_VEC4:
                glUniform4fv(paramslot.location, 1, reinterpret_cast<const GLfloat*>(value));
                break;

            case GSU_MAT2:
                glUniformMatrix2fv(paramslot.location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(value));
                break;

            case GSU_MAT3:
                glUniformMatrix3fv(paramslot.location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(value));
                break;

            case GSU_MAT4:
                glUniformMatrix4fv(paramslot.location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(value));
                break;

            default:
                return error(GSE_INVALIDENUM);
        }
    }

    return error(GS_OK);
}

struct SimpleDrawer
{
    void DrawArrays(GLenum mode, GLint first, GLsizei count) const
    {
        glDrawArrays(mode, first, count);
    }

    void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex) const
    {
        glDrawElementsBaseVertex(mode, count, type, indices, basevertex);
    }
};

struct InstancedDrawer
{
    InstancedDrawer(GSuint count) :
        p_count(count)
    {}

    void DrawArrays(GLenum mode, GLint first, GLsizei count) const
    {
        glDrawArraysInstanced(mode, first, count, p_count);
    }

    void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex) const
    {
        glDrawElementsInstancedBaseVertex(mode, count, type, indices, p_count, basevertex);
    }

private:
    GSuint p_count;
};

struct SimpleMultiDrawer
{
    void MultiDrawArrays(GLenum mode, GLint *first, GLsizei *count, GLsizei drawcount) const
    {
        glMultiDrawArrays(mode, first, count, drawcount);
    }

    void MultiDrawElementsBaseVertex(GLenum mode, GLsizei *count, GLenum type, void **indices, GLsizei primcount, GLint *basevertex) const
    {
        glMultiDrawElementsBaseVertex(mode, count, type, indices, primcount, basevertex);
    }
};

struct InstancedMultiDrawer
{
    InstancedMultiDrawer(GSuint count) :
        p_count(count)
    {}

    void MultiDrawArrays(GLenum mode, GLint *first, GLsizei *count, GLsizei drawcount) const
    {
        // TODO
    }

    void MultiDrawElementsBaseVertex(GLenum mode, GLsizei *count, GLenum type, void **indices, GLsizei primcount, GLint *basevertex) const
    {
        // TODO
    }

private:
    GSuint p_count;
};

GSbool xGSImpl::DrawGeometry(IxGSGeometry geometry)
{
    SimpleDrawer drawer;
    return Draw(geometry, drawer);
}

GSbool xGSImpl::DrawGeometryInstanced(IxGSGeometry geometry, GSuint count)
{
    InstancedDrawer drawer(count);
    return Draw(geometry, drawer);
}

GSbool xGSImpl::DrawGeometries(IxGSGeometry *geometries, GSuint count)
{
    SimpleMultiDrawer drawer;
    return MultiDraw(geometries, count, drawer);
}

GSbool xGSImpl::DrawGeometriesInstanced(IxGSGeometry *geometries, GSuint count, GSuint instancecount)
{
    InstancedMultiDrawer drawer(instancecount);
    return MultiDraw(geometries, count, drawer);
}

GSbool xGSImpl::BeginCapture(GSenum mode, IxGSGeometryBuffer buffer)
{
    if (!ValidateState(RENDERER_READY, true, true, false)) {
        return GS_FALSE;
    }

    if (!buffer) {
        return error(GSE_INVALIDOBJECT);
    }

    xGSGeometryBufferImpl *bufferimpl = static_cast<xGSGeometryBufferImpl*>(buffer);
    if (!bufferimpl->allocated()) {
        return error(GSE_INVALIDOBJECT);
    }

    p_capturebuffer = bufferimpl;
    p_capturebuffer->AddRef();

    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, p_capturequery);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, p_capturebuffer->getVertexBufferID());
    glBeginTransformFeedback(gl_primitive_type(mode));

    p_systemstate = CAPTURE;

    return error(GS_OK);
}

GSbool xGSImpl::EndCapture(GSuint *elementcount)
{
    if (!ValidateState(CAPTURE, true, true, false)) {
        return GS_FALSE;
    }

    glEndTransformFeedback();
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

    if (elementcount) {
        glGetQueryObjectiv(
            p_capturequery, GL_QUERY_RESULT,
            reinterpret_cast<GLint*>(elementcount)
        );
    }

    p_capturebuffer->Release();
    p_capturebuffer = nullptr;
    p_systemstate = RENDERER_READY;

    return error(GS_OK);
}

GSbool xGSImpl::BeginImmediateDrawing(IxGSGeometryBuffer buffer, GSuint flags)
{
    if (!ValidateState(RENDERER_READY, false, true, false)) {
        return GS_FALSE;
    }

    if (!buffer) {
        return error(GSE_INVALIDOBJECT);
    }

    xGSGeometryBufferImpl *bufferimpl = static_cast<xGSGeometryBufferImpl*>(buffer);
    if (!bufferimpl->allocated() || bufferimpl->type() != GS_GBTYPE_IMMEDIATE) {
        return error(GSE_INVALIDOBJECT);
    }


    // TODO: do i need parrallel buffer fill only and then rendering from filled buffer?
    //          if so, special flag should indicate filling only behaviour and
    //          in that case current input check isn't needed

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
    if (bufferimpl != boundbuffer) {
        return error(GSE_INVALIDOBJECT);
    }
#endif

    p_immediatebuffer = bufferimpl;
    p_immediatebuffer->AddRef();

    p_immediatebuffer->BeginImmediateDrawing();

    return error(GS_OK);
}

GSbool xGSImpl::ImmediatePrimitive(GSenum type, GSuint vertexcount, GSuint indexcount, GSuint flags, GSimmediateprimitive *primitive)
{
    if (!ValidateState(RENDERER_READY, false, true, true)) {
        return GS_FALSE;
    }

    bool flushnotneeded = p_immediatebuffer->EmitPrimitive(
        type, vertexcount, indexcount,
        flags, primitive
    );

    if (!flushnotneeded) {
        // requested primitive can not be added, try to flush buffer
        p_immediatebuffer->EndImmediateDrawing();
        DrawImmediatePrimitives(p_immediatebuffer);
        p_immediatebuffer->BeginImmediateDrawing();

        if (!p_immediatebuffer->EmitPrimitive(type, vertexcount, indexcount, flags, primitive)) {
            // primitive can not be added at all
            return error(GSE_INVALIDVALUE);
        }
    }

    return error(GS_OK);
}

GSbool xGSImpl::EndImmediateDrawing()
{
    if (!ValidateState(RENDERER_READY, false, true, true)) {
        return GS_FALSE;
    }

    p_immediatebuffer->EndImmediateDrawing();
    DrawImmediatePrimitives(p_immediatebuffer);

    p_immediatebuffer->Release();
    p_immediatebuffer = nullptr;

    return error(GS_OK);
}

GSbool xGSImpl::BuildMIPs(IxGSTexture texture)
{
    if (!ValidateState(RENDERER_READY, true, false, false)) {
        return GS_FALSE;
    }

    xGSTextureImpl *tex = static_cast<xGSTextureImpl*>(texture);
    if (tex->getID() == 0) {
        return error(GSE_INVALIDOBJECT);
    }

    // TODO: fix state break-up with MIP level generation
    glBindTexture(tex->target(), tex->getID());
    glGenerateMipmap(tex->target());

    return error(GS_OK);
}

GSbool xGSImpl::CopyImage(
    IxGSTexture src, GSuint srclevel, GSuint srcx, GSuint srcy, GSuint srcz,
    IxGSTexture dst, GSuint dstlevel, GSuint dstx, GSuint dsty, GSuint dstz,
    GSuint width, GSuint height, GSuint depth
)
{
    // TODO: checks

    xGSTextureImpl *srctex = static_cast<xGSTextureImpl*>(src);
    xGSTextureImpl *dsttex = static_cast<xGSTextureImpl*>(dst);

    glCopyImageSubData(
        srctex->getID(), srctex->target(), GLint(srclevel), GLint(srcx), GLint(srcy), GLint(srcz),
        dsttex->getID(), dsttex->target(), GLint(dstlevel), GLint(dstx), GLint(dsty), GLint(dstz),
        width, height, depth
    );

    return error(GS_OK);
}

static bool BindCopyObject(xGSObject *obj, GLenum copytarget)
{
    GSenum type = static_cast<xGSUnknownObjectImpl*>(obj)->objecttype();

    GLuint objectid = 0;
    switch (type) {
        case GS_OBJECTTYPE_GEOMETRYBUFFER: {
            xGSGeometryBufferImpl *impl = static_cast<xGSGeometryBufferImpl*>(obj);
            objectid = impl->getVertexBufferID();
            break;
        }

        case GS_OBJECTTYPE_DATABUFFER: {
            xGSDataBufferImpl *impl = static_cast<xGSDataBufferImpl*>(obj);
            objectid = impl->getID();
            break;
        }

        case GS_OBJECTTYPE_TEXTURE: {
            xGSTextureImpl *impl = static_cast<xGSTextureImpl*>(obj);
            if (impl->target() != GL_TEXTURE_BUFFER) {
                return false;
            }
            objectid = impl->getID();
            break;
        }

        default:
            return false;
    }

    glBindBuffer(copytarget, objectid);

    return true;
}

GSbool xGSImpl::CopyData(xGSObject *src, xGSObject *dst, GSuint64 readoffset, GSuint64 writeoffset, GSuint64 size, GSuint flags)
{
    // TODO: checks

    GSenum dsttype = static_cast<xGSUnknownObjectImpl*>(dst)->objecttype();

    if (!BindCopyObject(src, GL_COPY_READ_BUFFER)) {
        return error(GSE_INVALIDOPERATION);
    }

    if (!BindCopyObject(dst, GL_COPY_WRITE_BUFFER)) {
        return error(GSE_INVALIDOPERATION);
    }

    glCopyBufferSubData(
        GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
        readoffset, writeoffset, size
    );

    return error(GS_OK);
}

GSbool xGSImpl::Compute(IxGSComputeState state, GSuint x, GSuint y, GSuint z)
{
    // TODO:

    return GS_FALSE;
}

GSbool xGSImpl::BeginTimerQuery()
{
    // TODO: checks

    if (p_timerqueries[p_timerindex] == 0) {
        glGenQueries(1, &p_timerqueries[p_timerindex]);
    }

    glBeginQuery(GL_TIME_ELAPSED, p_timerqueries[p_timerindex]);

    ++p_timerindex;
    if (p_timerindex > p_timerscount) {
        p_timerscount = p_timerindex;
    }

    return GS_TRUE;
}

GSbool xGSImpl::EndTimerQuery()
{
    // TODO: checks

    glEndQuery(GL_TIME_ELAPSED);

    return GS_TRUE;
}

GSbool xGSAPI xGSImpl::TimstampQuery()
{
    // TODO: checks
    // TODO: remove copypasta

    if (p_timerqueries[p_timerindex] == 0) {
        glGenQueries(1, &p_timerqueries[p_timerindex]);
    }

    glQueryCounter(p_timerqueries[p_timerindex], GL_TIMESTAMP);

    ++p_timerindex;
    if (p_timerindex > p_timerscount) {
        p_timerscount = p_timerindex;
    }

    return GS_TRUE;
}

GSbool xGSImpl::GatherTimers(GSuint64 *values, GSuint count)
{
    // TODO: checks

    if (count >= p_timerindex) {
        GLint available = 0;
        while (!available) {
            glGetQueryObjectiv(p_timerqueries[p_timerindex - 1], GL_QUERY_RESULT_AVAILABLE, &available);
        }

        for (GSuint n = 0; n < p_timerindex; ++n) {
            glGetQueryObjecti64v(p_timerqueries[n], GL_QUERY_RESULT, reinterpret_cast<GLint64*>(values));
            ++values;
        }
    }

    p_timerindex = 0;

    return GS_TRUE;
}


#define GS_ADD_REMOVE_OBJECT_IMPL(list, type)\
template <> void xGSImpl::AddObject(type *object)\
{\
    list.insert(object);\
}\
\
template <> void xGSImpl::RemoveObject(type *object)\
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


IxGS xGSImpl::gs = nullptr;

IxGS xGSImpl::create(xGScontextCreator *contextcreator)
{
    if (!gs) {
        gs = new xGSImpl(contextcreator->create());
    }

    gs->AddRef();
    return gs;
}


GSbool xGSImpl::GetTextureFormatDescriptor(GSvalue format, TextureFormatDescriptor &descriptor)
{
    auto d = p_texturedescs.find(format);
    if (d == p_texturedescs.end()) {
        return GS_FALSE;
    }

    descriptor = d->second;

    return GS_TRUE;
}

const GSpixelformat& xGSImpl::DefaultRenderTargetFormat()
{
    return p_context->RenderTargetFormat();
}


GSbool xGSImpl::ValidateState(SystemState requiredstate, bool exactmatch, bool matchimmediate, bool requiredimmediate)
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

void xGSImpl::AddTextureFormatDescriptor(GSvalue format, GSint _bpp, GLenum _intformat, GLenum _format, GLenum _type)
{
    p_texturedescs.insert(std::make_pair(
        format,
        TextureFormatDescriptor(_bpp, _intformat, _format, _type)
    ));
}

void xGSImpl::RenderTargetSize(GSsize &size)
{
    size = p_rendertarget ? p_rendertarget->size() : p_context->RenderTargetSize();
}

void xGSImpl::DefaultRTFormats()
{
    // TODO: fill in current RT formats with default RT formats
    for (size_t n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
        p_colorformats[n] = GS_NONE;
    }

    const GSpixelformat &fmt = p_context->RenderTargetFormat();

    p_colorformats[0] = ColorFormatFromPixelFormat(fmt);
    p_depthstencilformat = DepthFormatFromPixelFormat(fmt);
}

void xGSImpl::SetState(xGSStateImpl *state)
{
    AttachObject(p_state, state);
    AttachObject(p_input, static_cast<xGSInputImpl*>(nullptr));
    for (size_t n = 0; n < GS_MAX_PARAMETER_SETS; ++n) {
        AttachObject(p_parameters[n], static_cast<xGSParametersImpl*>(nullptr));
    }
#ifdef _DEBUG
    if (!p_state) {
        xGSStateImpl::bindNullProgram();
    }
#endif
}

template <typename T>
GSbool xGSImpl::Draw(IxGSGeometry geometry_to_draw, const T &drawer)
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
    if (!geometry->allocated()) {
        return error(GSE_INVALIDOBJECT);
    }

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
        drawer.DrawArrays(
            gl_primitive_type(geometry->type()), geometry->baseVertex(),
            geometry->vertexCount()
        );
    } else {
        drawer.DrawElementsBaseVertex(
            gl_primitive_type(geometry->type()),
            geometry->indexCount(), gl_index_type(buffer->indexFormat()),
            geometry->indexPtr(), geometry->baseVertex()
        );
    }

    return error(GS_OK);
}

template <typename T>
GSbool xGSImpl::MultiDraw(IxGSGeometry *geometries_to_draw, GSuint count, const T &drawer)
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
    GLint first[4096];
    GLsizei counts[4096];
    GSptr indices[4096];
    size_t current = 0;

    for (GSuint n = 0; n < count; ++n) {
        xGSGeometryImpl *geometry = static_cast<xGSGeometryImpl*>(geometries_to_draw[n]);
        if (!geometry->allocated()) {
            return error(GSE_INVALIDOBJECT);
        }

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
        drawer.MultiDrawArrays(gl_primitive_type(primtype), first, counts, count);
    } else {
        drawer.MultiDrawElementsBaseVertex(
            gl_primitive_type(primtype), counts,
            gl_index_type(indextype), indices, count, first
        );
    }

    return error(GS_OK);
}

void xGSImpl::DrawImmediatePrimitives(xGSGeometryBufferImpl *buffer)
{
    // TODO: think about MultiDraw implementation for this

    for (size_t n = 0; n < buffer->immediateCount(); ++n) {
        const xGSGeometryBufferImpl::Primitive &p = buffer->immediatePrimitive(n);

        if (p.indexcount == 0) {
            glDrawArrays(p.type, p.firstvertex, p.vertexcount);
        } else {
            glDrawElementsBaseVertex(
                p.type,
                p.indexcount, gl_index_type(buffer->indexFormat()),
                reinterpret_cast<GSptr>(index_buffer_size(buffer->indexFormat(), p.firstindex)),
                p.firstvertex
            );
        }
    }
}

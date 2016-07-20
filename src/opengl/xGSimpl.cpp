/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    opengl/xGSimpl.cpp
        xGS API object implementation class
*/

#include "xGSimpl.h"
#include "xGScontextplatform.h"
#include "xGSgeometry.h"
#include "xGSgeometrybuffer.h"
#include "xGSdatabuffer.h"
#include "xGStexture.h"
#include "xGSframebuffer.h"
#include "xGSstate.h"
#include "xGSinput.h"
#include "xGSparameters.h"

#ifdef _DEBUG
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

xGSImpl::xGSImpl() :
    xGSImplBase(),
    p_context(new xGScontext())
{
    p_error = p_context->Initialize();
    if (p_error != GS_OK) {
        return;
    }

    p_systemstate = SYSTEM_READY;
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

void xGSImpl::CreateRendererImpl(const GSrendererdescription &desc)
{
    p_error = p_context->CreateRenderer(desc);
    if (p_error != GS_OK) {
        return;
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
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &p_caps.max_draw_buffers);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &p_caps.max_texture_size);
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE,  &p_caps.max_3d_texture_size);
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &p_caps.max_array_texture_layers);
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &p_caps.max_cube_map_texture_size);
    glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE, &p_caps.max_rectangle_texture_size);
    glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &p_caps.max_texture_buffer_size);
    p_caps.copy_image           = GS_CAPS_COPY_IMAGE;
    p_caps.sparse_texture       = GS_CAPS_SPARSE_TEXTURE;
    p_caps.sparse_buffer        = GS_CAPS_SPARSE_BUFFER;
#ifdef GS_CONFIG_SPARSE_TEXTURE
    glGetIntegerv(GL_MAX_SPARSE_TEXTURE_SIZE_ARB, &p_caps.max_sparse_texture_size);
    glGetIntegerv(GL_MAX_SPARSE_3D_TEXTURE_SIZE_ARB, &p_caps.max_sparse_3dtexture_size);
    glGetIntegerv(GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_ARB, &p_caps.max_sparse_texture_layers);
#endif
#ifdef GS_CONFIG_SPARSE_BUFFER
    glGetIntegerv(GL_SPARSE_BUFFER_PAGE_SIZE_ARB, &p_caps.sparse_buffer_pagesize);
#endif
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &p_caps.ubo_alignment);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &p_caps.max_ubo_size);

#ifdef _DEBUG
    debug(DebugMessageLevel::Information, "CAPS: max_active_attribs:        %i\n", p_caps.max_active_attribs);
    debug(DebugMessageLevel::Information, "CAPS: max_texture_units:         %i\n", p_caps.max_texture_units);
    debug(DebugMessageLevel::Information, "CAPS: multibind:                 %s\n", p_caps.multi_bind ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: multiblend:                %s\n", p_caps.multi_blend ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: vertex format:             %s\n", p_caps.vertex_format ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: ubo_alignment:             %i\n", p_caps.ubo_alignment);
    debug(DebugMessageLevel::Information, "CAPS: max_ubo_size:              %i\n", p_caps.max_ubo_size);
    debug(DebugMessageLevel::Information, "CAPS: texture sRGB:              %s\n", p_caps.texture_srgb ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: texture float:             %s\n", p_caps.texture_float ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: texture depth:             %s\n", p_caps.texture_depth ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: texture depthstencil:      %s\n", p_caps.texture_depthstencil ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: max_draw_buffers:          %i\n", p_caps.max_draw_buffers);
    debug(DebugMessageLevel::Information, "CAPS: max_texture_size:          %i\n", p_caps.max_texture_size);
    debug(DebugMessageLevel::Information, "CAPS: max_3d_texture_size:       %i\n", p_caps.max_3d_texture_size);
    debug(DebugMessageLevel::Information, "CAPS: max_array_texture_layers:  %i\n", p_caps.max_array_texture_layers);
    debug(DebugMessageLevel::Information, "CAPS: max_cube_map_texture_size: %i\n", p_caps.max_cube_map_texture_size);
    debug(DebugMessageLevel::Information, "CAPS: max_rect_texture_size:     %i\n", p_caps.max_rectangle_texture_size);
    debug(DebugMessageLevel::Information, "CAPS: max_texture_buffer_size:   %i\n", p_caps.max_texture_buffer_size);
    debug(DebugMessageLevel::Information, "CAPS: copy image:                %s\n", p_caps.copy_image ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: sparse texture:            %s\n", p_caps.sparse_texture ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: max_sparse_texture_size:   %i\n", p_caps.max_sparse_texture_size);
    debug(DebugMessageLevel::Information, "CAPS: max_sparse_3dtexture_size: %i\n", p_caps.max_sparse_3dtexture_size);
    debug(DebugMessageLevel::Information, "CAPS: max_sparse_texture_layers: %i\n", p_caps.max_sparse_texture_layers);
    debug(DebugMessageLevel::Information, "CAPS: sparse buffer:             %s\n", p_caps.sparse_buffer ? "Yes" : "No");
    debug(DebugMessageLevel::Information, "CAPS: sparse_buffer_pagesize:    %i\n", p_caps.sparse_buffer_pagesize);
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

    memset(p_timerqueries, 0, sizeof(p_timerqueries));
    p_timerindex = 0;
    p_opentimerqueries = 0;
    p_timerscount = 0;


#ifdef _DEBUG
    debugTrackGLError("xGSImpl::CreateRenderer");
#endif

    DefaultRTFormats();
}

void xGSImpl::DestroyRendererImpl()
{
    for (auto &sampler : p_samplerlist) {
        glDeleteSamplers(1, &sampler.sampler);
    }
    glDeleteQueries(1, &p_capturequery);
    glDeleteQueries(p_timerscount, p_timerqueries);

    p_context->DestroyRenderer();
}

void xGSImpl::CreateSamplersImpl(const GSsamplerdescription *samplers, GSuint count)
{
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

    p_error = GS_OK;
}

void xGSImpl::GetRenderTargetSizeImpl(GSsize &size)
{
    size = p_rendertarget ? p_rendertarget->size() : p_context->RenderTargetSize();
}

void xGSImpl::ClearImpl(GSbool color, GSbool depth, GSbool stencil, const GScolor &colorvalue, float depthvalue, GSdword stencilvalue)
{
    // NOTE: clear doesn't work if rasterizer discard enabled
    //       depth buffer is not cleared when depth mask is FALSE

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
}

void xGSImpl::DisplayImpl()
{
    error(p_context->Display() ? GS_OK : GSE_SUBSYSTEMFAILED);
}

void xGSImpl::SetRenderTargetImpl()
{
    bool srgb = false;

    if (p_rendertarget == nullptr) {
        DefaultRTFormats();
        srgb = p_context->RenderTargetFormat().pfSRGB;
    } else {
        p_rendertarget->getformats(p_colorformats, p_depthstencilformat);
        srgb = p_rendertarget->srgb();
    }

    srgb ? glEnable(GL_FRAMEBUFFER_SRGB) : glDisable(GL_FRAMEBUFFER_SRGB);
}

void xGSImpl::SetViewportImpl(const GSrect &viewport)
{
    GSsize sz;
    GetRenderTargetSizeImpl(sz);

    glViewport(
        viewport.left, sz.height - viewport.height - viewport.top,
        viewport.width, viewport.height
    );
}

void xGSImpl::SetStencilReferenceImpl(GSuint ref)
{
    // TODO
    error(GSE_UNIMPLEMENTED);
}

void xGSImpl::SetBlendColorImpl(const GScolor &color)
{
    glBlendColor(color.r, color.g, color.b, color.a);
}

void xGSImpl::SetUniformValueImpl(GSenum type, GSint location, const void *value)
{
    switch (type) {
        case GSU_SCALAR:
            glUniform1fv(location, 1, reinterpret_cast<const GLfloat*>(value));
            break;

        case GSU_VEC2:
            glUniform2fv(location, 1, reinterpret_cast<const GLfloat*>(value));
            break;

        case GSU_VEC3:
            glUniform3fv(location, 1, reinterpret_cast<const GLfloat*>(value));
            break;

        case GSU_VEC4:
            glUniform4fv(location, 1, reinterpret_cast<const GLfloat*>(value));
            break;

        case GSU_MAT2:
            glUniformMatrix2fv(location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(value));
            break;

        case GSU_MAT3:
            glUniformMatrix3fv(location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(value));
            break;

        case GSU_MAT4:
            glUniformMatrix4fv(location, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(value));
            break;

        default:
            break;
    }
}

void xGSImpl::SetupGeometryImpl(xGSGeometryImpl *geometry)
{
    // set up patch parameters
    if (geometry->type() == GS_PRIM_PATCHES) {
        glPatchParameteri(GL_PATCH_VERTICES, geometry->patchvertices());
    }

    // set up restart parameters
    if (!geometry->restart()) {
        glDisable(GL_PRIMITIVE_RESTART);
    } else {
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(geometry->restartindex());
    }
}

struct SimpleDrawer
{
    void DrawArrays(GSenum mode, GSuint first, GSuint count) const
    {
        glDrawArrays(gl_primitive_type(mode), first, count);
    }

    void DrawElementsBaseVertex(GSenum mode, GSuint count, GSenum type, const void *indices, GSuint basevertex) const
    {
        glDrawElementsBaseVertex(gl_primitive_type(mode), count, gl_index_type(type), indices, basevertex);
    }
};

struct InstancedDrawer
{
    InstancedDrawer(GSuint count) :
        p_count(count)
    {}

    void DrawArrays(GSenum mode, GSuint first, GSuint count) const
    {
        glDrawArraysInstanced(gl_primitive_type(mode), first, count, p_count);
    }

    void DrawElementsBaseVertex(GSenum mode, GSuint count, GSenum type, const void *indices, GSuint basevertex) const
    {
        glDrawElementsInstancedBaseVertex(gl_primitive_type(mode), count, gl_index_type(type), indices, p_count, basevertex);
    }

private:
    GSuint p_count;
};

struct SimpleMultiDrawer
{
    void MultiDrawArrays(GSenum mode, int *first, int *count, GSuint drawcount) const
    {
        glMultiDrawArrays(gl_primitive_type(mode), first, count, drawcount);
    }

    void MultiDrawElementsBaseVertex(GSenum mode, int *count, GSenum type, void **indices, GSuint primcount, int *basevertex) const
    {
        glMultiDrawElementsBaseVertex(gl_primitive_type(mode), count, gl_index_type(type), indices, primcount, basevertex);
    }
};

struct InstancedMultiDrawer
{
    InstancedMultiDrawer(GSuint count) :
        p_count(count)
    {}

    void MultiDrawArrays(GSenum mode, int *first, int *count, GSuint drawcount) const
    {
        // TODO
    }

    void MultiDrawElementsBaseVertex(GSenum mode, int *count, GSenum type, void **indices, GSuint primcount, int *basevertex) const
    {
        // TODO
    }

private:
    GSuint p_count;
};


void xGSImpl::BeginCaptureImpl(GSenum mode)
{
    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, p_capturequery);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, p_capturebuffer->getVertexBufferID());
    glBeginTransformFeedback(gl_primitive_type(mode));
}

void xGSImpl::EndCaptureImpl(GSuint *elementcount)
{
    glEndTransformFeedback();
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

    if (elementcount) {
        glGetQueryObjectiv(
            p_capturequery, GL_QUERY_RESULT,
            reinterpret_cast<GLint*>(elementcount)
        );
    }
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

void xGSImpl::BuildMIPsImpl(xGSTextureImpl *texture)
{
    // TODO: fix state break-up with MIP level generation
    glBindTexture(texture->target(), texture->getID());
    glGenerateMipmap(texture->target());
}

void xGSImpl::CopyImageImpl(
    xGSTextureImpl *src, GSuint srclevel, GSuint srcx, GSuint srcy, GSuint srcz,
    xGSTextureImpl *dst, GSuint dstlevel, GSuint dstx, GSuint dsty, GSuint dstz,
    GSuint width, GSuint height, GSuint depth
)
{
    glCopyImageSubData(
        src->getID(), src->target(), GLint(srclevel), GLint(srcx), GLint(srcy), GLint(srcz),
        dst->getID(), dst->target(), GLint(dstlevel), GLint(dstx), GLint(dsty), GLint(dstz),
        width, height, depth
    );
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

void xGSImpl::CopyDataImpl(xGSObject *src, xGSObject *dst, GSuint readoffset, GSuint writeoffset, GSuint size, GSuint flags)
{
    if (!BindCopyObject(src, GL_COPY_READ_BUFFER)) {
        error(GSE_INVALIDOPERATION);
        return;
    }

    if (!BindCopyObject(dst, GL_COPY_WRITE_BUFFER)) {
        error(GSE_INVALIDOPERATION);
        return;
    }

    glCopyBufferSubData(
        GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
        readoffset, writeoffset, size
    );

    p_error = GS_OK;
}

void xGSImpl::BufferCommitmentImpl(xGSObject *buffer, GSuint offset, GSuint size, GSbool commit, GSuint flags)
{
    GSenum type = static_cast<xGSUnknownObjectImpl*>(buffer)->objecttype();

    GLuint objectid = 0;
    GLenum target = 0;
    switch (type) {
        // TODO: do we really need to commit geometry buffers separate from their geometries?
        case GS_OBJECTTYPE_GEOMETRYBUFFER: {
            xGSGeometryBufferImpl *impl = static_cast<xGSGeometryBufferImpl*>(buffer);
            // TODO: consider flags to index buffer commit
            objectid = impl->getVertexBufferID();
            target = GL_ARRAY_BUFFER;
            break;
        }

        case GS_OBJECTTYPE_DATABUFFER: {
            xGSDataBufferImpl *impl = static_cast<xGSDataBufferImpl*>(buffer);
            objectid = impl->getID();
            target = impl->target();
            break;
        }

        case GS_OBJECTTYPE_TEXTURE: {
            xGSTextureImpl *impl = static_cast<xGSTextureImpl*>(buffer);
            if (impl->target() != GL_TEXTURE_BUFFER) {
                error(GSE_INVALIDOBJECT);
                return;
            }
            objectid = impl->getID();
            target = GL_TEXTURE_BUFFER;
            break;
        }

        default:
            error(GSE_INVALIDOBJECT);
            return;
    }

    glBindBuffer(target, objectid);
    glBufferPageCommitmentARB(target, offset, size, commit);

    p_error = GS_OK;
}

void xGSImpl::GeometryBufferCommitmentImpl(xGSGeometryBufferImpl *buffer)
{
    // TODO: this breaks current input binding, resolve it
    glBindBuffer(GL_ARRAY_BUFFER, buffer->getVertexBufferID());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->getIndexBufferID());
}

void xGSImpl::GeometryBufferCommitmentGeometry(xGSGeometryImpl *geometry, GSuint vertexsize, GSuint indexsize, GSbool commit)
{
    glBufferPageCommitmentARB(
        GL_ARRAY_BUFFER, GLintptr(geometry->vertexPtr()),
        geometry->vertexCount() * vertexsize, commit
    );

    if (geometry->indexCount() == 0) {
        return;
    }

    glBufferPageCommitmentARB(
        GL_ELEMENT_ARRAY_BUFFER, GLintptr(geometry->indexPtr()),
        geometry->indexCount() * indexsize, commit
    );
}

void xGSImpl::TextureCommitmentImpl(xGSTextureImpl *texture, GSuint level, GSuint x, GSuint y, GSuint z, GSuint width, GSuint height, GSuint depth, GSbool commit)
{
    if (texture->target() == GL_TEXTURE_BUFFER) {
        error(GSE_INVALIDOBJECT);
        return;
    }

    // TODO: this breaks state, resolve
    glBindTexture(texture->target(), texture->getID());
    glTexPageCommitmentARB(texture->target(), level, x, y, z, width, height, depth, commit);

    p_error = GS_OK;
}

void xGSImpl::BeginTimerQueryImpl()
{
    if (p_timerqueries[p_timerindex] == 0) {
        glGenQueries(1, &p_timerqueries[p_timerindex]);
    }

    glBeginQuery(GL_TIME_ELAPSED, p_timerqueries[p_timerindex]);
    ++p_opentimerqueries;

    ++p_timerindex;
    if (p_timerindex > p_timerscount) {
        p_timerscount = p_timerindex;
    }
}

void xGSImpl::EndTimerQueryImpl()
{
    if (p_opentimerqueries == 0) {
        error(GSE_INVALIDOPERATION);
        return;
    }

    --p_opentimerqueries;
    glEndQuery(GL_TIME_ELAPSED);

    p_error = GS_OK;
}

void xGSImpl::TimestampQueryImpl()
{
    // TODO: remove copypasta

    if (p_timerqueries[p_timerindex] == 0) {
        glGenQueries(1, &p_timerqueries[p_timerindex]);
    }

    glQueryCounter(p_timerqueries[p_timerindex], GL_TIMESTAMP);

    ++p_timerindex;
    if (p_timerindex > p_timerscount) {
        p_timerscount = p_timerindex;
    }
}

void xGSImpl::GatherTimersImpl(GSuint flags, GSuint64 *values, GSuint count)
{
    if (p_timerindex == 0) {
        error(GSE_INVALIDOPERATION);
        return;
    }

    if (count >= p_timerindex) {
        GLint available = 0;

        if (flags) {
            while (!available) {
                glGetQueryObjectiv(p_timerqueries[p_timerindex - 1], GL_QUERY_RESULT_AVAILABLE, &available);
            }
        } else {
            glGetQueryObjectiv(p_timerqueries[p_timerindex - 1], GL_QUERY_RESULT_AVAILABLE, &available);
            if (!available) {
                // TODO: fix error code for not available query
                p_error = GSE_INVALIDOPERATION;
                return;
            }
        }

        for (GSuint n = 0; n < p_timerindex; ++n) {
            glGetQueryObjecti64v(p_timerqueries[n], GL_QUERY_RESULT, reinterpret_cast<GLint64*>(values));
            ++values;
        }
    }

    p_timerindex = 0;

    p_error = GS_OK;
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


void xGSImpl::AddTextureFormatDescriptor(GSvalue format, GSint _bpp, GLenum _intformat, GLenum _format, GLenum _type)
{
    p_texturedescs.insert(std::make_pair(
        format,
        TextureFormatDescriptor(_bpp, _intformat, _format, _type)
    ));
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

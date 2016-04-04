#include "xGSimpl.h"
#include "xGSgeometrybuffer.h"
#include "xGSgeometry.h"
#include "xGSdatabuffer.h"
#include "xGStexture.h"
#include "xGSstate.h"
#include "xGSutil.h"


xGSimpl::xGSimpl() :
    p_window(0),
    p_windowdc(0),
    p_glcontext(0)
{}

xGSimpl::~xGSimpl()
{
    DestroyRenderer();
}

void APIENTRY OpenGLDebugCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const char *message, const void *userParam
)
{
    char buf[4096];
    sprintf_s(
        buf, "OpenGL debug callback: %d %d %d %d %s\n",
        source, type, id, severity, message
    );
    OutputDebugStringA(buf);
}

bool xGSimpl::CreateRenderer(const GSrendererdesc &desc)
{
    if (p_glcontext) {
        // TODO: implement error codes
        return false;
    }

    p_window = HWND(desc.window);

    // Get window DC we want to render to
    p_windowdc = GetDC(p_window);
    if (!p_windowdc) {
        // TODO: implement error codes
        return false;
    }

    // TODO: implement WGL pixel formats
    // TODO: implement ARB_create_context for modern context creation

    // setup Window pixel format, before context can be created
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cAlphaBits = 0; // in case we need transparent framebuffer this should be set to 8
    pfd.cDepthBits = 32;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int npf = ChoosePixelFormat(p_windowdc, &pfd);
    if (!SetPixelFormat(p_windowdc, npf, nullptr)) {
        // TODO: implement error codes
        DestroyRenderer();
        return false;
    }

    p_glcontext = wglCreateContext(p_windowdc);
    if (!p_glcontext) {
        // TODO: implement error codes
        DestroyRenderer();
        return false;
    }

    wglMakeCurrent(p_windowdc, p_glcontext);

    // init GLEW lib so all extensions and modern GL can be used
    glewInit();

#ifdef _DEBUG
    if (GLEW_ARB_debug_output) {
        glDebugMessageCallback(OpenGLDebugCallback, nullptr);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }
#endif

    TrackGLError();

    memset(p_samplers, 0, sizeof(p_samplers));
    p_samplerscount = 0;

    // make a basic setup for OpenGL

    return true;
}

bool xGSimpl::DestroyRenderer()
{
    if (!p_glcontext) {
        // TODO: implement error codes
        return false;
    }

    glDeleteSamplers(p_samplerscount, p_samplers);

#ifdef _DEBUG
    // all xGS objects should be deleted before renderer destruction
    // check here for objects lists
    CheckObjectList(p_geometrybufferlist, "GeometryBuffer");
    CheckObjectList(p_geometrylist, "Geometry");
    CheckObjectList(p_databufferlist, "DataBuffer");
    CheckObjectList(p_texturelist, "Texture");
    CheckObjectList(p_statelist, "State");
#endif

    if (p_glcontext) {
        wglMakeCurrent(0, 0);
        wglDeleteContext(p_glcontext);
        p_glcontext = 0;
    }

    if (p_windowdc) {
        ReleaseDC(p_window, p_windowdc);
        p_windowdc = 0;
        p_window = 0;
    }

    return true;
}

bool xGSimpl::CreateSamplers(GSsamplerdesc *samplers, unsigned int count)
{
    if (!p_glcontext) {
        // TODO: implement error codes
        return false;
    }

    // TODO: handle 2nd calls

    // TODO: remove magic number
    if (count > 8) {
        count = 8;
    }

    p_samplerscount = count;
    glGenSamplers(p_samplerscount, p_samplers);
    for (unsigned int n = 0; n < count; ++n) {
        switch (samplers->filter) {
            case GS_FILTER_NEAREST:
                glSamplerParameteri(p_samplers[n], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glSamplerParameteri(p_samplers[n], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                break;

            case GS_FILTER_LINEAR:
                glSamplerParameteri(p_samplers[n], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glSamplerParameteri(p_samplers[n], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                break;

            case GS_FILTER_TRILINEAR:
                glSamplerParameteri(p_samplers[n], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glSamplerParameteri(p_samplers[n], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                break;
        }

        glSamplerParameteri(p_samplers[n], GL_TEXTURE_WRAP_S, glwrapmode(samplers->wrapu));
        glSamplerParameteri(p_samplers[n], GL_TEXTURE_WRAP_T, glwrapmode(samplers->wrapv));
        glSamplerParameteri(p_samplers[n], GL_TEXTURE_WRAP_R, glwrapmode(samplers->wrapw));

        ++samplers;
    }

    return true;
}

#define GS_CREATE_OBJECT(type, impl, desctype) \
    case type: { \
        impl *objectimpl = new impl(this); \
        if (!objectimpl->Allocate(*reinterpret_cast<const desctype*>(desc))) { \
            delete objectimpl; \
            break; \
        } \
        *object = objectimpl; \
        break; \
    }

bool xGSimpl::CreateObject(GSobjecttype type, const void *desc, void **object)
{
    switch (type) {
        GS_CREATE_OBJECT(GS_OBJECT_GEOMETRYBUFFER, xGSgeometrybufferImpl, GSgeometrybufferdesc)
        GS_CREATE_OBJECT(GS_OBJECT_GEOMETRY, xGSgeometryImpl, GSgeometrydesc)
        GS_CREATE_OBJECT(GS_OBJECT_DATABUFFER, xGSdatabufferImpl, GSdatabufferdesc)
        GS_CREATE_OBJECT(GS_OBJECT_TEXTURE, xGStextureImpl, GStexturedesc)
        GS_CREATE_OBJECT(GS_OBJECT_STATE, xGSstateImpl, GSstatedesc)

        default:
            // TODO: implement error codes
            return false;
    }

    return true;
}

#undef GS_CREATE_OBJECT

bool xGSimpl::GetRenderTargetSize(/* out */ GSsize &size)
{
    if (!p_glcontext) {
        // TODO: implement error codes
        return false;
    }

    RenderTargetSize(size);

    return true;
}


bool xGSimpl::Clear(bool clearcolor, bool cleardepth, bool clearstencil, const GScolor &color, float depth, unsigned int stencil)
{
    if (!p_glcontext) {
        // TODO: implement error codes
        return false;
    }

    GLbitfield mask = 0;

    if (clearcolor) {
        glClearColor(color.r, color.g, color.b, color.a);
        mask |= GL_COLOR_BUFFER_BIT;
    }

    if (cleardepth) {
        glClearDepth(depth);
        mask |= GL_DEPTH_BUFFER_BIT;
    }

    if (clearstencil) {
        glClearStencil(stencil);
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    glClear(mask);

    TrackGLError();

    return true;
}

bool xGSimpl::SetViewport(const GSviewport &viewport)
{
    if (!p_glcontext) {
        // TODO: implement error codes
        return false;
    }

    // OpenGL viewport originates at bottom left, treat GSviewport origin as top left
    GSsize sz;
    RenderTargetSize(sz);

    glViewport(
        viewport.x, sz.height - viewport.height - viewport.y,
        viewport.width, viewport.height
    );

    return true;
}

bool xGSimpl::SetState(xGSstate *state)
{
    if (!p_glcontext) {
        // TODO: implement error codes
        return false;
    }

    if (state == nullptr) {
        // TODO: implement error codes
        return false;
    }

    TrackGLError();

    xGSstateImpl *impl = static_cast<xGSstateImpl*>(state);
    impl->Apply();

    TrackGLError();

    return true;
}

bool xGSimpl::DrawGeometry(xGSgeometry *geometry)
{
    if (!p_glcontext) {
        // TODO: implement error codes
        return false;
    }

    // TODO: checks

    TrackGLError();

    xGSgeometryImpl *impl = static_cast<xGSgeometryImpl*>(geometry);
    if (impl->buffer()->indexBufferId()) {
        glDrawElementsBaseVertex(
            impl->primtype(), impl->indexcount(),
            glindextype(impl->buffer()->indexformat()),
            reinterpret_cast<void*>(indexsize(impl->buffer()->indexformat(), impl->baseindex())),
            impl->basevertex()
        );
    } else  {
        glDrawArrays(impl->primtype(), impl->basevertex(), impl->vertexcount());
    }

    TrackGLError();

    return true;
}

bool xGSimpl::BuildMIPs(xGStexture *texture)
{
    if (!p_glcontext) {
        // TODO: implement error codes
        return false;
    }

    // TODO: checks

    xGStextureImpl *impl = static_cast<xGStextureImpl*>(texture);
    // TODO: prevent active texture state trashing
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(impl->target(), impl->textureId());
    glGenerateMipmap(impl->target());

    return true;
}

bool xGSimpl::Display()
{
    if (!p_glcontext) {
        // TODO: implement error codes
        return false;
    }

    return SwapBuffers(p_windowdc) != 0;
}

#define IMPL_ADD_REMOVE_OBJECT(T, list) \
void xGSimpl::AddObject(T *object) \
{ \
    list.insert(object); \
} \
\
void xGSimpl::RemoveObject(T *object) \
{ \
    list.erase(list.find(object)); \
}

IMPL_ADD_REMOVE_OBJECT(xGSgeometrybuffer, p_geometrybufferlist)
IMPL_ADD_REMOVE_OBJECT(xGSgeometry, p_geometrylist)
IMPL_ADD_REMOVE_OBJECT(xGSdatabuffer, p_databufferlist)
IMPL_ADD_REMOVE_OBJECT(xGStexture, p_texturelist)
IMPL_ADD_REMOVE_OBJECT(xGSstate, p_statelist)

#undef IMPL_ADD_REMOVE_OBJECT

void xGSimpl::TrackGLError()
{
#ifdef _DEBUG
    GLenum error = glGetError();
    if (error) {


    }
#endif
}

void xGSimpl::RenderTargetSize(/* out */ GSsize &size)
{
    // TODO: reimplement when concept of multiple render targets implemented
    RECT rc;
    GetClientRect(p_window, &rc);

    size.width = rc.right;
    size.height = rc.bottom;
}

template <typename T>
void xGSimpl::CheckObjectList(const T &list, const char *type)
{
    if (list.size()) {
        // TODO: replace with xGS's debug output (when it will be implemented)
        char buf[4096];
        sprintf_s(
            buf, "xGS debug: list %s still has %d objects!\n",
            type, list.size()
        );
        OutputDebugStringA(buf);
    }
}


xGS *xGScreate()
{
    // leave new for now, later I'll add refcounted objects so
    // library users won't bother how to delete xGS instance
    return new xGSimpl();
}

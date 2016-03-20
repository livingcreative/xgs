#include "xGSimpl.h"
#include "xGSgeometrybuffer.h"
#include "xGSgeometry.h"
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

    // make a basic setup for OpenGL

    return true;
}

bool xGSimpl::DestroyRenderer()
{
    if (!p_glcontext) {
        // TODO: implement error codes
        return false;
    }

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

#define GS_CREATE_OBJECT(type, impl, desctype) \
    case type: { \
        impl *objectimpl = new impl(); \
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

bool xGSimpl::Display()
{
    if (!p_glcontext) {
        // TODO: implement error codes
        return false;
    }

    return SwapBuffers(p_windowdc) != 0;
}

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


xGS *xGScreate()
{
    // leave new for now, later I'll add refcounted objects so
    // library users won't bother how to delete xGS instance
    return new xGSimpl();
}
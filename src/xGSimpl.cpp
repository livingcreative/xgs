#include "xGSimpl.h"
#include "xGSgeometrybuffer.h"
#include "xGSgeometry.h"
#include "xGSstate.h"


xGSimpl::xGSimpl() :
    p_window(0),
    p_windowdc(0),
    p_glcontext(0)
{}

xGSimpl::~xGSimpl()
{
    DestroyRenderer();
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


bool xGSimpl::CreateObject(GSobjecttype type, const void *desc, void **object)
{
    switch (type) {
        case GS_OBJECT_GEOMETRYBUFFER: {
            xGSgeometrybufferImpl *buffer = new xGSgeometrybufferImpl();
            if (!buffer->Allocate(*reinterpret_cast<const GSgeometrybufferdesc*>(desc))) {
                delete buffer;
                break;
            }

            *object = buffer;
            break;
        }

        case GS_OBJECT_GEOMETRY: {
            xGSgeometryImpl *geometry = new xGSgeometryImpl();
            if (!geometry->Allocate(*reinterpret_cast<const GSgeometrydesc*>(desc))) {
                delete geometry;
                break;
            }

            *object = geometry;
            break;
        }

        case GS_OBJECT_STATE: {
            xGSstateImpl *state = new xGSstateImpl();
            if (!state->Allocate(*reinterpret_cast<const GSstatedesc*>(desc))) {
                delete state;
                break;
            }

            *object = state;
            break;
        }

        default:
            // TODO: implement error codes
            return false;
    }

    return true;
}


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

    glViewport(viewport.x, sz.height - viewport.y, viewport.width, viewport.height);

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

    xGSstateImpl *impl = static_cast<xGSstateImpl*>(state);
    impl->Apply();

    return true;
}

bool xGSimpl::DrawGeometry(xGSgeometry *geometry)
{
    if (!p_glcontext) {
        // TODO: implement error codes
        return false;
    }

    // TODO: checks

    xGSgeometryImpl *impl = static_cast<xGSgeometryImpl*>(geometry);
    if (impl->buffer()->indexBufferId()) {
        // TODO: make this into utility func
        GLenum type = 0;
        GLuint size = 0;
        switch (impl->buffer()->indexformat()) {
            case GS_INDEX_WORD: type = GL_UNSIGNED_SHORT; size = 2; break;
            case GS_INDEX_DWORD: type = GL_UNSIGNED_INT; size = 4; break;
        }

        glDrawElementsBaseVertex(
            impl->primtype(), impl->indexcount(), type,
            reinterpret_cast<void*>(impl->baseindex() * size), impl->basevertex()
        );
    } else  {
        glDrawArrays(impl->primtype(), impl->basevertex(), impl->vertexcount());
    }

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

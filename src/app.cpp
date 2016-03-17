#include "app.h"
#include "xGS/xGS.h"

// leave this for now, until all needed code moved to wrapper class
#include <Windows.h>
#include "GL/glew.h"


// yep, plain global vars, don't do this at home kids, global vars are bad )
static xGS *gs = nullptr;


GSvertexcomponent boxcomponents[] = {
    GS_VEC2, -1, "pos",
    GS_LAST_COMPONENT
};

struct Vertex
{
    float x, y;
};

static Vertex box[] = {
    -0.5f, +0.5f,
    +0.5f, +0.5f,
    +0.5f, -0.5f,
    -0.5f, -0.5f
};

static GLushort box_indices[] = {
    0, 1, 2,
    2, 3, 0
};

static xGSgeometrybuffer *buffer = nullptr;
static xGSgeometry *boxgeom = nullptr;
static xGSstate *state = nullptr;



void initialize(void *hwnd)
{
    gs = xGScreate();
    if (!gs) {
        return;
    }

    GSrendererdesc rdesc = {
        hwnd,
        true, // use double buffer
        GS_DEFAULT, // use default color format for framebuffer
        GS_DEFAULT, // use default depth
        GS_DEFAULT  // use default stencil
    };
    if (!gs->CreateRenderer(rdesc)) {
        // TODO: implement refcounting and remove delete
        delete gs;
    }

    // create geometry buffer for storing box data, vertex and index buffers
    GSgeometrybufferdesc gbdesc = {
        boxcomponents, 4, GS_INDEX_WORD, 6, 0
    };
    gs->CreateObject(GS_OBJECT_GEOMETRYBUFFER, &gbdesc, reinterpret_cast<void**>(&buffer));

    // fill in vertex data
    if (void *ptr = buffer->Lock(GS_LOCK_VERTEXBUFFER, GS_WRITE)) {
        memcpy(ptr, box, sizeof(box));
        buffer->Unlock();
    }
    // fill in index data
    if (void *ptr = buffer->Lock(GS_LOCK_INDEXBUFFER, GS_WRITE)) {
        memcpy(ptr, box_indices, sizeof(box_indices));
        buffer->Unlock();
    }

    // create geometry object for box
    GSgeometrydesc gdesc = {
        GS_PRIM_TRIANGLES,
        buffer,
        4, 6
    };
    gs->CreateObject(GS_OBJECT_GEOMETRY, &gdesc, reinterpret_cast<void**>(&boxgeom));


    GSinputslot input[] = {
        GS_STATIC, boxcomponents, buffer,
        GS_LAST_SLOT
    };

    const char *vss[] = {
        "in vec2 pos;\n"
        "void main() {\n"
        "   gl_Position = vec4(pos, 0, 1);\n"
        "}",
        nullptr
    };

    const char *pss[] = {
        "#version 330\n"
        "out vec4 color;\n"
        "void main() {\n"
        "   color = vec4(1, 0, 0, 1);\n"
        "}",
        nullptr
    };

    GSstatedesc statedesc = {
        input,
        vss,
        nullptr,
        nullptr,
        nullptr,
        pss
    };
    gs->CreateObject(GS_OBJECT_STATE, &statedesc, reinterpret_cast<void**>(&state));


}

void step()
{
    if (!gs) {
        return;
    }

    GSsize sz;
    gs->GetRenderTargetSize(sz);

    gs->Clear(true, true, false, GScolor::construct(0, 0.5f, 0.5f));

    gs->SetViewport(GSviewport::construct(0, 0, sz.width, sz.height));

    // activate shaders
    gs->SetState(state);

    gs->DrawGeometry(boxgeom);

    gs->Display();
}

void finalize()
{
    if (boxgeom) {
        // TODO: implement refcounting and remove delete
        delete boxgeom;
    }

    if (state) {
        // TODO: implement refcounting and remove delete
        delete state;
    }

    if (buffer) {
        // TODO: implement refcounting and remove delete
        delete buffer;
    }

    if (gs) {
        gs->DestroyRenderer();
        // TODO: implement refcounting and remove delete
        delete gs;
    }
}

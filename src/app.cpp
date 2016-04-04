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
static xGSdatabuffer *transforms = nullptr;
static xGStexture *tex = nullptr;
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


    // create data buffer for uniform storage
    GSuniform mvp[] = {
        GS_MAT4, 1,
        GS_LAST_COMPONENT
    };

    GSuniformblock blocks[] = {
        mvp, 1,
        nullptr
    };

    GSdatabufferdesc dbdesc = {
        blocks, 0
    };
    gs->CreateObject(GS_OBJECT_DATABUFFER, &dbdesc, reinterpret_cast<void**>(&transforms));

    float mat[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    transforms->Update(0, 0, 1, mat);

    // allocate samplers
    GSsamplerdesc samplers[] = {
        GS_FILTER_TRILINEAR, GS_WRAP_REPEAT, GS_WRAP_REPEAT, GS_WRAP_REPEAT
    };
    gs->CreateSamplers(samplers, sizeof(samplers) / sizeof(samplers[0]));

    // create texture

    // TODO: load texture from file

    GStexturedesc tdesc = GStexturedesc::construct();
    tdesc.width = 256;
    tdesc.height = 256;
    tdesc.levels = 8;
    gs->CreateObject(GS_OBJECT_TEXTURE, &tdesc, reinterpret_cast<void**>(&tex));
    // test checker fill
    if (void *ptr = tex->Lock(GS_LOCK_TEXTURE, 0, 0, GS_WRITE)) {
        unsigned int *pixel = reinterpret_cast<unsigned int*>(ptr);
        for (unsigned int y = 0; y < tdesc.height; ++y) {
            for (unsigned int x = 0; x< tdesc.width; ++x) {
                *pixel++ = ((x / 16 + y / 16) & 1) ? 0xFFFFFFFF : 0xFF000000;
            }
        }
        tex->Unlock();

        gs->BuildMIPs(tex);
    }


    GSinputslot input[] = {
        GS_STATIC, boxcomponents, buffer,
        GS_LAST_SLOT
    };

    const char *vss[] = {
        "#version 330\n"
        "in vec2 pos;\n"
        "out vec2 texcoord;\n"
        "layout(std140) uniform Transforms {\n"
        "    mat4x4 mvp_matrix;\n"
        "};\n"
        "void main() {\n"
        "   texcoord = pos;\n"
        "   gl_Position = mvp_matrix * vec4(pos, 0, 1);\n"
        "}",
        nullptr
    };

    const char *pss[] = {
        "#version 330\n"
        "uniform sampler2D tex;\n"
        "in vec2 texcoord;\n"
        "out vec4 color;\n"
        "void main() {\n"
        "   color = texture(tex, texcoord);\n"
        "}",
        nullptr
    };

    GSparametersslot slots[] = {
        GS_DATABUFFER, -1, "Transforms",
        GS_TEXTURE, -1, "tex",
        GS_LAST_PARAMETER
    };

    GStexturebinding textures[] = {
        tex, 0,
        nullptr
    };

    GSdatabufferbinding buffers[] = {
        transforms, 0,
        nullptr
    };

    GSparametersset parameters[] = {
        GS_STATIC_SET, slots, textures, buffers,
        GS_LAST_SET
    };

    GSstatedesc statedesc = {
        input,
        vss,
        nullptr,
        nullptr,
        nullptr,
        pss,
        parameters
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
        boxgeom->Release();
    }

    if (state) {
        state->Release();
    }

    if (tex) {
        tex->Release();
    }

    if (transforms) {
        transforms->Release();
    }

    if (buffer) {
        buffer->Release();
    }

    if (gs) {
        gs->DestroyRenderer();
        gs->Release();
    }
}

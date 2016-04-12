#include "app.h"
#include "xGS/xGS.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include "kcommon/c_geometry.h"

// leave this for now, until all needed code moved to wrapper class
#include <Windows.h>
#include "GL/glew.h"


// yep, plain global vars, don't do this at home kids, global vars are bad )
static xGS *gs = nullptr;


GSvertexcomponent boxcomponents[] = {
    GS_VEC3, -1, "pos",
    GS_VEC3, -1, "norm",
    GS_VEC2, -1, "tex",
    GS_LAST_COMPONENT
};

struct Vertex
{
    float x, y, z;    // position
    float nx, ny, nz; // normal
    float u, v;       // tex coords
};

static Vertex cube[] = {
//       x     y     z   nx    ny    nz  u  v
    { -1.0,  1.0,  1.0, 0.0,  1.0,  0.0, 0, 1 },
    {  1.0,  1.0,  1.0, 0.0,  1.0,  0.0, 1, 1 },
    {  1.0,  1.0, -1.0, 0.0,  1.0,  0.0, 1, 0 },
    { -1.0,  1.0, -1.0, 0.0,  1.0,  0.0, 0, 0 },

    { -1.0,  1.0, -1.0, 0.0,  0.0, -1.0, 0, 1 },
    {  1.0,  1.0, -1.0, 0.0,  0.0, -1.0, 1, 1 },
    {  1.0, -1.0, -1.0, 0.0,  0.0, -1.0, 1, 0 },
    { -1.0, -1.0, -1.0, 0.0,  0.0, -1.0, 0, 0 },

    {  1.0,  1.0, -1.0, 1.0,  0.0,  0.0, 0, 1 },
    {  1.0,  1.0,  1.0, 1.0,  0.0,  0.0, 1, 1 },
    {  1.0, -1.0,  1.0, 1.0,  0.0,  0.0, 1, 0 },
    {  1.0, -1.0, -1.0, 1.0,  0.0,  0.0, 0, 0 },

    {  1.0,  1.0,  1.0, 0.0,  0.0,  1.0, 0, 1 },
    { -1.0,  1.0,  1.0, 0.0,  0.0,  1.0, 1, 1 },
    { -1.0, -1.0,  1.0, 0.0,  0.0,  1.0, 1, 0 },
    {  1.0, -1.0,  1.0, 0.0,  0.0,  1.0, 0, 0 },

    { -1.0,  1.0,  1.0, 1.0,  0.0,  0.0, 0, 1 },
    { -1.0,  1.0, -1.0, 1.0,  0.0,  0.0, 1, 1 },
    { -1.0, -1.0, -1.0, 1.0,  0.0,  0.0, 1, 0 },
    { -1.0, -1.0,  1.0, 1.0,  0.0,  0.0, 0, 0 },

    {  1.0, -1.0,  1.0, 0.0, -1.0,  0.0, 0, 1 },
    { -1.0, -1.0,  1.0, 0.0, -1.0,  0.0, 1, 1 },
    { -1.0, -1.0, -1.0, 0.0, -1.0,  0.0, 1, 0 },
    {  1.0, -1.0, -1.0, 0.0, -1.0,  0.0, 0, 0 }
};

static GLushort cube_indices[] = {
#if 0
     0,  1,  3,  1,  2,  3,
     4,  5,  7,  5,  6,  7,
     8,  9, 11,  9, 10, 11,
    12, 13, 15, 13, 14, 15,
    16, 17, 19, 17, 18, 19,
    20, 21, 23, 21, 22, 23
#else
     0,  1,  2,  3,
     4,  5,  6,  7,
     8,  9, 10, 11,
    12, 13, 14, 15,
    16, 17, 18, 19,
    20, 21, 22, 23
#endif
};

static xGSgeometrybuffer *buffer = nullptr;
static xGSgeometry *boxgeom = nullptr;
static xGSdatabuffer *transforms = nullptr;
static xGStexture *tex = nullptr;
static xGSstate *state = nullptr;


static c_geometry::mat4x4f world;


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
        gs->Release();
    }

    // create geometry buffer for storing box data, vertex and index buffers
    GSgeometrybufferdesc gbdesc = {
        boxcomponents, 24, GS_INDEX_WORD, 24, 0
    };
    gs->CreateObject(GS_OBJECT_GEOMETRYBUFFER, &gbdesc, reinterpret_cast<void**>(&buffer));

    // fill in vertex data
    if (void *ptr = buffer->Lock(GS_LOCK_VERTEXBUFFER, GS_WRITE)) {
        memcpy(ptr, cube, sizeof(cube));
        buffer->Unlock();
    }
    // fill in index data
    if (void *ptr = buffer->Lock(GS_LOCK_INDEXBUFFER, GS_WRITE)) {
        memcpy(ptr, cube_indices, sizeof(cube_indices));
        buffer->Unlock();
    }

    // create geometry object for box
    GSgeometrydesc gdesc = {
        GS_PRIM_PATCHES,
        buffer,
        24, 24,
        4
    };
    gs->CreateObject(GS_OBJECT_GEOMETRY, &gdesc, reinterpret_cast<void**>(&boxgeom));


    // create data buffer for uniform storage
    GSuniform mvp[] = {
        GS_MAT4, 1, // mvp matrix
        GS_MAT4, 1, // normal matrix
        GS_LAST_COMPONENT
    };

    GSuniform lightpos[] = {
        GS_VEC4, 1,
        GS_LAST_COMPONENT
    };

    GSuniformblock blocks[] = {
        mvp, 1,
        lightpos, 1,
        nullptr
    };

    GSdatabufferdesc dbdesc = {
        blocks, 0
    };
    gs->CreateObject(GS_OBJECT_DATABUFFER, &dbdesc, reinterpret_cast<void**>(&transforms));

    c_geometry::vec4f lightdir(1, 1, -1, 0);
    lightdir.normalize();
    transforms->Update(1, 0, 1, &lightdir);

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
        "in vec3 pos;\n"
        "in vec3 norm;\n"
        "in vec2 tex;\n"
        "out vec3 vnormal;\n"
        "out vec2 vtexcoord;\n"
        "void main() {\n"
        "    vnormal = norm;"
        "    vtexcoord = tex;\n"
        "    gl_Position = vec4(pos, 1);\n"
        "}",
        nullptr
    };

    const char *css[] = {
        "#version 400\n"
        "layout(vertices=4) out;\n"
        "in vec3 vnormal[];\n"
        "in vec2 vtexcoord[];\n"
        "out vec3 ctlnormal[];\n"
        "out vec2 ctltexcoord[];\n"
        "void main() {\n"
        "    if (gl_InvocationID == 0) {\n"
        "        float inner = 16;\n"
        "        float outer = 16;\n"
        "        gl_TessLevelInner[0] = inner;\n"
        "        gl_TessLevelInner[1] = inner;\n"
        "        gl_TessLevelOuter[0] = outer;\n"
        "        gl_TessLevelOuter[1] = outer;\n"
        "        gl_TessLevelOuter[2] = outer;\n"
        "        gl_TessLevelOuter[3] = outer;\n"
        "    }\n"
        "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;\n"
        "    ctlnormal[gl_InvocationID] = vnormal[gl_InvocationID];\n"
        "    ctltexcoord[gl_InvocationID] = vtexcoord[gl_InvocationID];\n"
        "}",
        nullptr
    };

    const char *ess[] = {
        "#version 400\n"
        "layout(quads, equal_spacing, ccw) in;\n"
        "in vec3 ctlnormal[];\n"
        "in vec2 ctltexcoord[];\n"
        "out vec3 normal;\n"
        "out vec2 texcoord;\n"
        "layout(std140) uniform Transforms {\n"
        "    mat4x4 mvp_matrix;\n"
        "    mat4x4 nrm_matrix;\n"
        "};\n"
        "void main() {\n"
        "    vec3 a = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x).xyz;\n"
        "    vec3 b = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x).xyz;\n"
        "    vec3 pos = mix(a, b, gl_TessCoord.y);\n"
        "    pos = normalize(pos);\n" // "spherify cube"
        "    vec2 tc0 = mix(ctltexcoord[0], ctltexcoord[1], gl_TessCoord.x);\n"
        "    vec2 tc1 = mix(ctltexcoord[3], ctltexcoord[2], gl_TessCoord.x);\n"
        "    normal = (nrm_matrix * vec4(pos, 1)).xyz;\n"
        "    texcoord = mix(tc0, tc1, gl_TessCoord.y);\n"
        "    gl_Position = mvp_matrix * vec4(pos, 1);\n"
        "}",
        nullptr
    };

    const char *pss[] = {
        "#version 330\n"
        "uniform sampler2D tex;\n"
        "in vec3 normal;\n"
        "in vec2 texcoord;\n"
        "out vec4 color;\n"
        "layout(std140) uniform Light {\n"
        "    vec4 lightdir;\n"
        "};\n"
        "void main() {\n"
        "    float L = max(0, dot(normalize(normal), lightdir.xyz));\n"
        "    color = texture(tex, texcoord) * L;\n"
        //"    color = vec4(normalize(normal) * 0.5 + 0.5, 1);\n"
        //"    color = texture(tex, texcoord);\n"
        "}",
        nullptr
    };

    GSparametersslot slots[] = {
        GS_DATABUFFER, -1, "Transforms",
        GS_DATABUFFER, -1, "Light",
        GS_TEXTURE, -1, "tex",
        GS_LAST_PARAMETER
    };

    GStexturebinding textures[] = {
        tex, 0,
        nullptr
    };

    GSdatabufferbinding buffers[] = {
        transforms, 0,
        transforms, 1,
        nullptr
    };

    GSparametersset parameters[] = {
        GS_STATIC_SET, slots, textures, buffers,
        GS_LAST_SET
    };

    GSstatedesc statedesc = GSstatedesc::construct();
    statedesc.input = input;
    statedesc.vs = vss;
    statedesc.cs = css;
    statedesc.es = ess;
    statedesc.ps = pss;
    statedesc.parameters = parameters;
    gs->CreateObject(GS_OBJECT_STATE, &statedesc, reinterpret_cast<void**>(&state));

    world.rotatebyx(30);
}

void step()
{
    if (!gs) {
        return;
    }

    GSsize sz;
    gs->GetRenderTargetSize(sz);

    world.rotatebyy(0.1f);

    c_geometry::mat4x4f view;
    view.translate(0, 0, 2);

    c_geometry::mat4x4f proj;
    proj.projectionL(90.0f, float(sz.width) / sz.height, 0.1f, 10.0f);

    c_geometry::mat4x4f nrm = world * view;
    nrm.inverse();
    nrm.transpose();

    c_geometry::mat4x4f tfm[] = {
        world * view * proj,
        c_geometry::mat4x4f(
            c_geometry::vec4f(nrm.row(0).x, nrm.row(0).y, nrm.row(0).z, 0),
            c_geometry::vec4f(nrm.row(1).x, nrm.row(1).y, nrm.row(1).z, 0),
            c_geometry::vec4f(nrm.row(2).x, nrm.row(2).y, nrm.row(2).z, 0),
            c_geometry::vec4f()
        )
    };
    transforms->Update(0, 0, 1, tfm);

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

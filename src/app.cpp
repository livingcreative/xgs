#include "app.h"
#include "xGS/xGS.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include "kcommon/c_geometry.h"


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
    { -1.0,  1.0,  1.0,  0.0,  1.0,  0.0, 0, 1 },
    {  1.0,  1.0,  1.0,  0.0,  1.0,  0.0, 1, 1 },
    {  1.0,  1.0, -1.0,  0.0,  1.0,  0.0, 1, 0 },
    { -1.0,  1.0, -1.0,  0.0,  1.0,  0.0, 0, 0 },

    { -1.0,  1.0, -1.0,  0.0,  0.0, -1.0, 0, 1 },
    {  1.0,  1.0, -1.0,  0.0,  0.0, -1.0, 1, 1 },
    {  1.0, -1.0, -1.0,  0.0,  0.0, -1.0, 1, 0 },
    { -1.0, -1.0, -1.0,  0.0,  0.0, -1.0, 0, 0 },

    {  1.0,  1.0, -1.0,  1.0,  0.0,  0.0, 0, 1 },
    {  1.0,  1.0,  1.0,  1.0,  0.0,  0.0, 1, 1 },
    {  1.0, -1.0,  1.0,  1.0,  0.0,  0.0, 1, 0 },
    {  1.0, -1.0, -1.0,  1.0,  0.0,  0.0, 0, 0 },

    {  1.0,  1.0,  1.0,  0.0,  0.0,  1.0, 0, 1 },
    { -1.0,  1.0,  1.0,  0.0,  0.0,  1.0, 1, 1 },
    { -1.0, -1.0,  1.0,  0.0,  0.0,  1.0, 1, 0 },
    {  1.0, -1.0,  1.0,  0.0,  0.0,  1.0, 0, 0 },

    { -1.0,  1.0,  1.0, -1.0,  0.0,  0.0, 0, 1 },
    { -1.0,  1.0, -1.0, -1.0,  0.0,  0.0, 1, 1 },
    { -1.0, -1.0, -1.0, -1.0,  0.0,  0.0, 1, 0 },
    { -1.0, -1.0,  1.0, -1.0,  0.0,  0.0, 0, 0 },

    {  1.0, -1.0,  1.0,  0.0, -1.0,  0.0, 0, 1 },
    { -1.0, -1.0,  1.0,  0.0, -1.0,  0.0, 1, 1 },
    { -1.0, -1.0, -1.0,  0.0, -1.0,  0.0, 1, 0 },
    {  1.0, -1.0, -1.0,  0.0, -1.0,  0.0, 0, 0 }
};

//#define USE_TESS

static unsigned short cube_indices[] = {
#ifndef USE_TESS
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
static xGStexture *rendertex = nullptr;
static xGSframebuffer *fb = nullptr;
static xGSstate *state = nullptr;


static c_geometry::mat4x4f world;


enum UniformBlock
{
    TRANSFORMS,
    POSITIONS,
    COLORS,
    LIGHT
};


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

    const size_t vertex_count = sizeof(cube) / sizeof(cube[0]);
    const size_t index_count = sizeof(cube_indices) / sizeof(cube_indices[0]);

    // create geometry buffer for storing box data, vertex and index buffers
    GSgeometrybufferdesc gbdesc = {
        boxcomponents, vertex_count, GS_INDEX_WORD, index_count, 0
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
#ifdef USE_TESS
        GS_PRIM_PATCHES,
#else
        GS_PRIM_TRIANGLES,
#endif
        buffer,
        vertex_count, index_count
#ifdef USE_TESS
        , 4
#endif
    };
    gs->CreateObject(GS_OBJECT_GEOMETRY, &gdesc, reinterpret_cast<void**>(&boxgeom));


    // create data buffer for uniform storage
    GSuniform mvp[] = {
        GS_MAT4, 1, // world matrix
        GS_MAT4, 1, // vp matrix
        GS_MAT4, 1, // normal matrix
        GS_LAST_COMPONENT
    };

    GSuniform lightpos[] = {
        GS_VEC4, 1,
        GS_LAST_COMPONENT
    };

    GSuniform instpos[] = {
        GS_VEC4, 4,
        GS_LAST_COMPONENT
    };

    GSuniform instcolors[] = {
        GS_VEC4, 4,
        GS_LAST_COMPONENT
    };

    GSuniformblock blocks[] = {
        mvp, 1,
        instpos, 1,
        instcolors, 1,
        lightpos, 1,
        nullptr
    };

    GSdatabufferdesc dbdesc = {
        blocks, 0
    };
    gs->CreateObject(GS_OBJECT_DATABUFFER, &dbdesc, reinterpret_cast<void**>(&transforms));

    c_geometry::vec4f lightdir(1, 1, -1, 0);
    lightdir.normalize();
    transforms->Update(LIGHT, 0, 1, &lightdir);

    c_geometry::vec4f positions[] = {
        c_geometry::vec4f( 2,  2, 2, 0),
        c_geometry::vec4f(-2,  2, 2, 0),
        c_geometry::vec4f(-2, -2, 2, 0),
        c_geometry::vec4f( 2, -2, 2, 0)
    };
    transforms->Update(POSITIONS, 0, 1, positions);

    c_geometry::vec4f colors[] = {
        c_geometry::vec4f(1, 0, 0, 0),
        c_geometry::vec4f(0, 1, 0, 0),
        c_geometry::vec4f(0, 0, 1, 0),
        c_geometry::vec4f(1, 1, 1, 0)
    };
    transforms->Update(COLORS, 0, 1, colors);

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


    // create texture for rendering to
    GStexturedesc rtdesc = GStexturedesc::construct();
    rtdesc.width = 1024;
    rtdesc.height = 1024;
    rtdesc.levels = 10;
    gs->CreateObject(GS_OBJECT_TEXTURE, &rtdesc, reinterpret_cast<void**>(&rendertex));


    // create frame buffer
    GSframebufferattachment fbattachments[] = {
        GS_ATTACHMENT0, rendertex, GS_LOCK_TEXTURE, 0, 0,
        GS_LAST_ATTACHMENT
    };

    GSframebufferdesc fbdesc = GSframebufferdesc::construct();
    fbdesc.width = rtdesc.width;
    fbdesc.height = rtdesc.height;
    fbdesc.depthformat = GS_DEPTH_24;
    fbdesc.attachments = fbattachments;
    gs->CreateObject(GS_OBJECT_FRAMEBUFFER, &fbdesc, reinterpret_cast<void**>(&fb));


    GSinputslot input[] = {
        GS_STATIC, boxcomponents, buffer,
        GS_LAST_SLOT
    };

    const char *vss[] = {
        "#version 330\n"
        "in vec3 pos;\n"
        "in vec3 norm;\n"
        "in vec2 tex;\n"
#ifdef USE_TESS
        "out vec3 vnormal;\n"
        "out vec2 vtexcoord;\n"
        "out flat int instid;\n"
#else
        "out vec3 normal;\n"
        "out vec2 texcoord;\n"
        "out vec4 vertexcolor;\n"
        "layout(std140) uniform Transforms {\n"
        "    mat4x4 w_matrix;\n"
        "    mat4x4 vp_matrix;\n"
        "    mat4x4 nrm_matrix;\n"
        "};\n"
        "layout(std140) uniform Positions {\n"
        "    vec4 positions[4];\n"
        "};\n"
        "layout(std140) uniform Colors {\n"
        "    vec4 colors[4];\n"
        "};\n"
#endif
        "void main() {\n"
#ifdef USE_TESS
        "    vnormal = norm;"
        "    vtexcoord = tex;\n"
        "    instid = gl_InstanceID;\n"
        "    gl_Position = vec4(pos, 1);\n"
#else
        "    normal = (nrm_matrix * vec4(norm, 1)).xyz;\n"
        "    texcoord = tex;\n"
        "    vertexcolor = colors[gl_InstanceID];\n"
        "    gl_Position = vp_matrix * (w_matrix * vec4(pos, 1) + positions[gl_InstanceID]);\n"
#endif
        "}",
        nullptr
    };

#ifdef USE_TESS
    const char *css[] = {
        "#version 400\n"
        "layout(vertices=4) out;\n"
        "in vec3 vnormal[];\n"
        "in vec2 vtexcoord[];\n"
        "in flat int instid[];\n"
        "out vec3 ctlnormal[];\n"
        "out vec2 ctltexcoord[];\n"
        "out flat int ctlinstid[];\n"
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
        "    ctlinstid[gl_InvocationID] = instid[gl_InvocationID];\n"
        "}",
        nullptr
    };

    const char *ess[] = {
        "#version 400\n"
        "layout(quads, equal_spacing, ccw) in;\n"
        "in vec3 ctlnormal[];\n"
        "in vec2 ctltexcoord[];\n"
        "in flat int ctlinstid[];\n"
        "out vec3 normal;\n"
        "out vec2 texcoord;\n"
        "out vec4 vertexcolor;\n"
        "layout(std140) uniform Transforms {\n"
        "    mat4x4 w_matrix;\n"
        "    mat4x4 vp_matrix;\n"
        "    mat4x4 nrm_matrix;\n"
        "};\n"
        "layout(std140) uniform Positions {\n"
        "    vec4 positions[4];\n"
        "};\n"
        "layout(std140) uniform Colors {\n"
        "    vec4 colors[4];\n"
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
        "    vertexcolor = colors[ctlinstid[0]];\n"
        "    gl_Position = vp_matrix * (w_matrix * vec4(pos, 1) + positions[ctlinstid[0]]);\n"
        "}",
        nullptr
    };
#endif

    const char *pss[] = {
        "#version 330\n"
        "uniform sampler2D tex;\n"
        "in vec3 normal;\n"
        "in vec2 texcoord;\n"
        "in vec4 vertexcolor;\n"
        "out vec4 color;\n"
        "layout(std140) uniform Light {\n"
        "    vec4 lightdir;\n"
        "};\n"
        "void main() {\n"
        "    float L = max(0, dot(normalize(normal), lightdir.xyz));\n"
        "    color = texture(tex, texcoord) * L * vertexcolor;\n"
        //"    color = vec4(normalize(normal) * 0.5 + 0.5, 1);\n"
        //"    color = texture(tex, texcoord);\n"
        "}",
        nullptr
    };

    GSparametersslot slots[] = {
        GS_DATABUFFER, -1, "Transforms",
        GS_DATABUFFER, -1, "Positions",
        GS_DATABUFFER, -1, "Colors",
        GS_DATABUFFER, -1, "Light",
        GS_TEXTURE, -1, "tex",
        GS_LAST_PARAMETER
    };

    GStexturebinding textures[] = {
        tex, 0,
        nullptr
    };

    GSdatabufferbinding buffers[] = {
        transforms, TRANSFORMS,
        transforms, POSITIONS,
        transforms, COLORS,
        transforms, LIGHT,
        nullptr
    };

    GSparametersset parameters[] = {
        GS_STATIC_SET, slots, textures, buffers,
        GS_LAST_SET
    };

    GSstatedesc statedesc = GSstatedesc::construct();
    statedesc.input = input;
    statedesc.vs = vss;
#ifdef USE_TESS
    statedesc.cs = css;
    statedesc.es = ess;
#endif
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


    // TODO: check rendering to texture via framebuffer


    GSsize sz;
    gs->GetRenderTargetSize(sz);

    world.rotatebyy(0.1f);

    c_geometry::mat4x4f view;
    view.translate(0, 0, 3);

    c_geometry::mat4x4f proj;
    proj.projectionL(90.0f, float(sz.width) / sz.height, 0.1f, 10.0f);

    c_geometry::mat4x4f nrm = world * view;
    nrm.inverse();
    nrm.transpose();

    c_geometry::mat4x4f tfm[] = {
        world,
        view * proj,
        c_geometry::mat4x4f(
            c_geometry::vec4f(nrm.row(0).x, nrm.row(0).y, nrm.row(0).z, 0),
            c_geometry::vec4f(nrm.row(1).x, nrm.row(1).y, nrm.row(1).z, 0),
            c_geometry::vec4f(nrm.row(2).x, nrm.row(2).y, nrm.row(2).z, 0),
            c_geometry::vec4f()
        )
    };
    transforms->Update(TRANSFORMS, 0, 1, tfm);

    gs->Clear(true, true, false, GScolor::construct(0, 0.5f, 0.5f));

    gs->SetViewport(GSviewport::construct(0, 0, sz.width, sz.height));

    // activate shaders
    gs->SetState(state);

    gs->DrawGeometryInstanced(boxgeom, 4);

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

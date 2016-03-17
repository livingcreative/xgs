#pragma once



enum GSformat
{
    GS_NONE,
    GS_DEFAULT,

    // color formats (for framebuffers, render targets and textures)
    GS_COLOR_32,

    // depth formats (for framebuffers, render targets and textures)
    GS_DEPTH_32,

    // stencil formats
    GS_STENCIL_8
};


enum GSobjecttype
{
    GS_OBJECT_GEOMETRYBUFFER,
    GS_OBJECT_GEOMETRY,
    GS_OBJECT_STATE
};


enum GSvertexcomponenttype
{
    GS_LAST_COMPONENT,
    GS_FLOAT,
    GS_VEC2,
    GS_VEC3,
    GS_VEC4
};

enum GSindexformat
{
    GS_INDEX_NONE,
    GS_INDEX_WORD,
    GS_INDEX_DWORD
};

enum GSlockbuffer
{
    GS_LOCK_VERTEXBUFFER,
    GS_LOCK_INDEXBUFFER
};

enum GSlockflags
{
    GS_READ  = 1 << 0,
    GS_WRITE = 1 << 1
};

enum GSinputslottype
{
    GS_LAST_SLOT,
    GS_STATIC,
    GS_DYNAMIC
};

enum GSprimitivetype
{
    GS_PRIM_LINES,
    GS_PRIM_LINESTRIP,
    GS_PRIM_TRIANGLES,
    GS_PRIM_TRIANGLESTRIP,
    GS_PRIM_TRIANGLEFAN
};


// struct for holding arbitrary size
struct GSsize
{
    int width;
    int height;
};

// struct for viewport
struct GSviewport
{
    int x;
    int y;
    int width;
    int height;

    static GSviewport construct(int _x, int _y, int _w, int _h)
    {
        GSviewport result = {
            _x, _y, _w, _h
        };
        return result;
    }
};

// color steuct
struct GScolor
{
    float r, g, b, a;

    static GScolor construct(float _r = 0, float _g = 0, float _b = 0, float _a = 0)
    {
        GScolor result = {
            _r, _g, _b, _a
        };
        return result;
    }
};


// forward declarations
class xGS;
class xGSgeometrybuffer;
class xGSgeometry;
class xGSstate;


// structure with renderer description,
// declares renderer parameters
struct GSrendererdesc
{
    void     *window;
    bool      doublebuffer;
    GSformat  color;
    GSformat  depth;
    GSformat  stencil;
};

struct GSvertexcomponent
{
    GSvertexcomponenttype  type;
    int                    biniding;
    const char            *name;
};

// structure with geometry buffer description,
// declares geometry buffer parameters
struct GSgeometrybufferdesc
{
    const GSvertexcomponent *components;
    unsigned int             vertexcount;
    GSindexformat            indexformat;
    unsigned int             indexcount;
    unsigned int             flags;
};


struct GSgeometrydesc
{
    GSprimitivetype    type;
    xGSgeometrybuffer *buffer;
    unsigned int       vertexcount;
    unsigned int       indexcount;
};


// structure declares programmable stage input slot
// slot is like a binding point to sourc (vertex)e data
// only binding point as a whole can be bound to geometry buffer
//      static slots constantly bound to buffer upon state creation
//      dynamic slots are linked with their buffers via GS input objects (will be defined later)
struct GSinputslot
{
    GSinputslottype    type;
    GSvertexcomponent *decl;
    xGSgeometrybuffer *buffer;
};

// structure with sate object description
// declares state object parameters, bindings and data
struct GSstatedesc
{
    GSinputslot *input;

    const char **vs; // vertex shader sources
    const char **cs; // tess control shader sources
    const char **es; // tess eval shader sources
    const char **gs; // geometry shader sources
    const char **ps; // pixel (fragment) shader sources
};



// xGS geometry buffer object
// holds data for geometry - vertex and index data in one place
class xGSgeometrybuffer
{
public:
    // lock/unlock buffer data
    virtual void *Lock(GSlockbuffer buffer, unsigned int flags) = 0;
    virtual bool Unlock() = 0;
};


// xGS geometry object
// holds data about geometry to be rendered
class xGSgeometry
{
public:

};


// xGS state object
// holds all necessary graphics state for rendering complete primitives
//      this includes (will include):
//          * all shaders in pipline (vertex, pixel, geometry and tess)
//          * resterizer, depth & stencil state
//          * other fixed state (blending, filling, culling, etc.)
class xGSstate
{
public:
    // now state object on its own, has no methods
};


// xGS system class, manages all initialization and lifetime of
// library
class xGS
{
public:
    // initialization and shutdown for renderer
    virtual bool CreateRenderer(const GSrendererdesc &desc) = 0;
    virtual bool DestroyRenderer() = 0;

    // create object API (it's ugly, I know)
    virtual bool CreateObject(GSobjecttype type, const void *desc, void **object) = 0;

    // different query APIs
    virtual bool GetRenderTargetSize(/* out */ GSsize &size) = 0;

    // clear current RT
    virtual bool Clear(bool clearcolor, bool cleardepth, bool clearstencil, const GScolor &color, float depth = 1, unsigned int stencil = 0) = 0;

    // immediate state API
    virtual bool SetViewport(const GSviewport &viewport) = 0;
    virtual bool SetState(xGSstate *state) = 0;

    // rendering API
    virtual bool DrawGeometry(xGSgeometry *geometry) = 0;

    // present rendered data to screen/window
    virtual bool Display() = 0;
};


// create xGS instance object
xGS *xGScreate();


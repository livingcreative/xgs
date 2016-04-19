#pragma once


/*
    xGS concepts
        All xGS objects are immutable
        Only object data can be modified after object has been created
            (buffer content or texture image data)

    xGS - represents system object which holds renderer and all internal structures

    xGSgeometrybuffer - holds geometry data (vertex and index data)
        Geometry buffer layout defined by GSvertexcomponent structures
        Each GSvertexcomponent defines vertex attribute (its type and name)

    xGSgeometry - holds data about single primitive (e.g. set of triangle to be drawn)
        Geometry object defines type of primitives to be rendered
        Source of geometry data (every geometry is bound to geometry buffer)
        Location of vertex and index data inside geometry buffer and its size
        Other parameters (primitive restart index, vertices in patch, etc)

    xGStexture - holds texture image data and its format description

    xGSframebuffer - holds description for frame buffer layout and attached textures

    xGSstate - holds all pipeline state required for rendering
        All shaders for programmable pipeline
        All fixed rasterizer state (blending, depth/stencil test, etc)
        Layout for shader input
            Shader input layout declares how geometry (vertex) data is fed to shader
            All input devided into slots
            Every slot can be bound to geometry buffer (so it can hold several attributes)
            Slot can be dynamic or static
                Static slot bound to geometry buffer when state object created
                Dynamic slots bound to their buffer via special input objects
        Layout for shader parameters
            Shader parameters are all uniform data (constants, buffers, textures)
            Parameters devided into slots (like input)
            Every slot is bound as a whole
            Slots can be static or dynamic
                Static slots bound to their resources when state object created
                Dynamic slots bound to their resources with special state objects
        Layout for shader output
            Declares how shader outputs are mapped into render target buffers

    xGSinput - defines connection between state inputs and geometry buffers

    xGSparameters - defines connection between state parameters and their resources
*/




enum GSformat
{
    GS_NONE,
    GS_DEFAULT,

    // color formats (for framebuffers, render targets and textures)
    GS_COLOR_R,
    GS_COLOR_RG,
    GS_COLOR_RGBX,
    GS_COLOR_RGBA,
    GS_COLOR_R_HALF_FLOAT,
    GS_COLOR_RG_HALF_FLOAT,
    GS_COLOR_RGBX_HALF_FLOAT,
    GS_COLOR_RGBA_HALF_FLOAT,
    GS_COLOR_R_FLOAT,
    GS_COLOR_RG_FLOAT,
    GS_COLOR_RGBX_FLOAT,
    GS_COLOR_RGBA_FLOAT,

    // depth formats (for framebuffers, render targets and textures)
    GS_DEPTH_16,
    GS_DEPTH_24,
    GS_DEPTH_32,
    GS_DEPTH_32_FLOAT,

    // stencil formats
    GS_STENCIL_8,

    // depth/stencil formats
    GS_DEPTH_STENCIL_D24S8
};


enum GSobjecttype
{
    GS_OBJECT_GEOMETRYBUFFER,
    GS_OBJECT_GEOMETRY,
    GS_OBJECT_DATABUFFER,
    GS_OBJECT_TEXTURE,
    GS_OBJECT_FRAMEBUFFER,
    GS_OBJECT_STATE
};


enum GSvertexcomponenttype
{
    GS_LAST_COMPONENT,
    GS_FLOAT,
    GS_VEC2,
    GS_VEC3,
    GS_VEC4,
    GS_MAT2,
    GS_MAT3,
    GS_MAT4
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

enum GSlocktexture
{
    GS_LOCK_TEXTURE,
    GS_LOCK_POSX,
    GS_LOCK_NEGX,
    GS_LOCK_POSY,
    GS_LOCK_NEGY,
    GS_LOCK_POSZ,
    GS_LOCK_NEGZ
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

enum GSparameterslottype
{
    GS_LAST_PARAMETER,
    GS_DATABUFFER,
    GS_TEXTURE
};

enum GSparameterssettype
{
    GS_LAST_SET,
    GS_STATIC_SET,
    GS_DYNAMIC_SET
};

enum GSprimitivetype
{
    GS_PRIM_LINES,
    GS_PRIM_LINESTRIP,
    GS_PRIM_TRIANGLES,
    GS_PRIM_TRIANGLESTRIP,
    GS_PRIM_TRIANGLEFAN,
    GS_PRIM_PATCHES
};

enum GSfiltertype
{
    GS_FILTER_NEAREST,
    GS_FILTER_LINEAR,
    GS_FILTER_TRILINEAR
};

enum GSwrapmode
{
    GS_WRAP_REPEAT,
    GS_WRAP_CLAMP
};

enum GStexturetype
{
    GS_TEXTURE_1D,
    GS_TEXTURE_2D,
    GS_TEXTURE_3D,
    GS_TEXTURE_RECTANGLE,
    GS_TEXTURE_CUBEMAP,
    GS_TEXTURE_BUFFER
};

enum GSattachment
{
    GS_LAST_ATTACHMENT,
    GS_ATTACHMENT0,
    GS_ATTACHMENT1,
    GS_ATTACHMENT2,
    GS_ATTACHMENT3,
    GS_ATTACHMENT4,
    GS_ATTACHMENT5,
    GS_ATTACHMENT6,
    GS_ATTACHMENT7,
    GS_ATTACHMENT_DEPTH
};

enum GSdepthtesttype
{
    GS_DEPTHTEST_NONE,
    GS_DEPTHTEST_LESS,
    GS_DEPTHTEST_LEQUAL,
    GS_DEPTHTEST_EQUAL,
    GS_DEPTHTEST_GREATER,
    GS_DEPTHTEST_GEQUAL,
    GS_DEPTHTEST_ALWAYS
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
class xGSdatabuffer;
class xGStexture;
class xGSframebuffer;
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

// structure with geometry description,
// declares primitive type, buffer where geometry data stored
struct GSgeometrydesc
{
    GSprimitivetype    type;
    xGSgeometrybuffer *buffer;
    unsigned int       vertexcount;
    unsigned int       indexcount;
    unsigned int       patchvertices;
};


struct GSuniform
{
    // TODO: think about common type enum for vertex components and uniform types
    GSvertexcomponenttype type;
    unsigned int          count;
};

struct GSuniformblock
{
    const GSuniform *uniforms;
    unsigned int     count;
};

struct GSdatabufferdesc
{
    const GSuniformblock *blocks;
    unsigned int          flags;
};


// structure with sampler description,
// declares sampler parameters (wrap mode, compare mode, filtering)
struct GSsamplerdesc
{
    GSfiltertype filter;
    GSwrapmode   wrapu;
    GSwrapmode   wrapv;
    GSwrapmode   wrapw;
    // TODO: anisotropy
    // TODO: compare mode
};

// structure with texture description,
// declares texture image format and size
struct GStexturedesc
{
    GStexturetype type;
    GSformat      format;
    bool          sRGB;
    unsigned int  width;
    unsigned int  height;
    unsigned int  depth;
    unsigned int  layers;
    unsigned int  levels;
    unsigned int  multisample;

    static GStexturedesc construct(GStexturetype type = GS_TEXTURE_2D, GSformat format = GS_COLOR_RGBA)
    {
        GStexturedesc result = {
            type, format, false,
            0, 0, 0, 0, 1, 0
        };
        return result;
    }
};

// frame buffer object attachment description
// defines data for assignment given texture level/slice to FB attachment point
struct GSframebufferattachment
{
    GSattachment   attachment;
    xGStexture    *texture;
    GSlocktexture  face;
    unsigned int   level;
    unsigned int   slice;
};

const unsigned int GS_MAX_FB_ATTACHMENTS = 8;

// frame buffer object description struct, defines FB parameters and config
struct GSframebufferdesc
{
    unsigned int             width;
    unsigned int             height;
    unsigned int             colortargets;
    GSformat                 colorformats[GS_MAX_FB_ATTACHMENTS];
    GSformat                 depthformat;
    GSframebufferattachment *attachments;

    static GSframebufferdesc construct()
    {
        GSframebufferdesc result = {
            0, 0, 1
        };

        result.colorformats[0] = GS_DEFAULT;

        return result;
    }
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


struct GStexturebinding
{
    xGStexture   *texture;
    unsigned int  sampler;
};


struct GSdatabufferbinding
{
    xGSdatabuffer *buffer;
    unsigned int   block;
};


struct GSparametersslot
{
    GSparameterslottype  type;
    int                  biniding;
    const char          *name;
};


struct GSparametersset
{
    GSparameterssettype  type;
    GSparametersslot    *slots;
    GStexturebinding    *textures;
    GSdatabufferbinding *databuffers;
};


struct GSdepthstencilstate
{
    GSdepthtesttype depthtest;
    // TODO: stencil state, other depth params
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

    GSparametersset *parameters;

    GSdepthstencilstate depthstencil;

    static GSstatedesc construct()
    {
        GSstatedesc result = {
            nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr,
            GS_DEPTHTEST_LEQUAL
        };
        return result;
    }
};


class xGSrefcounted
{
public:
    virtual void AddRef() = 0;
    virtual void Release() = 0;
};



// xGS geometry buffer object
// holds data for geometry - vertex and index data in one place
class xGSgeometrybuffer : public xGSrefcounted
{
public:
    // lock/unlock buffer data
    virtual void *Lock(GSlockbuffer buffer, unsigned int flags) = 0;
    virtual bool Unlock() = 0;
};


// xGS geometry object
// holds data about geometry to be rendered
class xGSgeometry : public xGSrefcounted
{
public:

};


// xGS data buffer object
// holds uniform (constant) data
class xGSdatabuffer : public xGSrefcounted
{
public:
    // update specific value inside structured buffer
    virtual bool Update(unsigned int slot, unsigned int parameter, unsigned int count, const void *data) = 0;

    // lock/unlock data range
    virtual void *Lock(unsigned int offset, unsigned int size, unsigned int flags) = 0;
    virtual bool Unlock() = 0;
};


// xGS texture object
// holds texture image and its format
class xGStexture : public xGSrefcounted
{
public:
    // lock/unlock texture level/layer data
    virtual void *Lock(GSlocktexture lock, unsigned int level, unsigned int layer, unsigned int flags) = 0;
    virtual bool Unlock() = 0;
};


// xGS framebuffer object
// holds framebuffer configuration and attachments
class xGSframebuffer : public xGSrefcounted
{
public:
    // now state object on its own, has no methods
};


// xGS state object
// holds all necessary graphics state for rendering complete primitives
//      this includes (will include):
//          * all shaders in pipline (vertex, pixel, geometry and tess)
//          * resterizer, depth & stencil state
//          * other fixed state (blending, filling, culling, etc.)
class xGSstate : public xGSrefcounted
{
public:
    // now state object on its own, has no methods
};


// xGS system class, manages all initialization and lifetime of
// library
class xGS : public xGSrefcounted
{
public:
    // initialization and shutdown for renderer
    virtual bool CreateRenderer(const GSrendererdesc &desc) = 0;
    virtual bool DestroyRenderer() = 0;

    // create object API (it's ugly, I know)
    virtual bool CreateSamplers(GSsamplerdesc *samplers, unsigned int count) = 0;
    virtual bool CreateObject(GSobjecttype type, const void *desc, void **object) = 0;

    // different query APIs
    virtual bool GetRenderTargetSize(/* out */ GSsize &size) = 0;

    // clear current RT
    virtual bool Clear(bool clearcolor, bool cleardepth, bool clearstencil, const GScolor &color, float depth = 1, unsigned int stencil = 0) = 0;

    // immediate state API
    virtual bool SetRenderTarget(xGSframebuffer *target) = 0;
    virtual bool SetViewport(const GSviewport &viewport) = 0;
    virtual bool SetState(xGSstate *state) = 0;

    // rendering API
    virtual bool DrawGeometry(xGSgeometry *geometry) = 0;
    virtual bool DrawGeometryInstanced(xGSgeometry *geometry, unsigned int count) = 0;
    virtual bool BuildMIPs(xGStexture *texture) = 0;

    // present rendered data to screen/window
    virtual bool Display() = 0;
};


// create xGS instance object
xGS *xGScreate();


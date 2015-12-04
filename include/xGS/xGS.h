/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    xGS.h
        xGS API types, structures and interface declarations
*/

#pragma once

#include "IUnknown.h"
#include "InterfaceReference.h"
#include <stddef.h>

#ifdef WIN32
    #define xGSAPI __stdcall
#endif

#ifdef __APPLE__
    #define xGSAPI
#endif


typedef bool         GSbool;

typedef short        GSword;
typedef int          GSint;
typedef int          GSvalue;
typedef unsigned int GSdword;
typedef unsigned int GSuint;
typedef int          GSerror;
typedef int          GSenum;
typedef float        GSfloat;
typedef void*        GSptr;

typedef void*        GShandle;
typedef void*        GSwidget;


struct GSsize
{
    GSint width;
    GSint height;
};

struct GSrect
{
    GSint left;
    GSint top;
    GSint width;
    GSint height;
};


struct GScolor
{
    GSfloat r;
    GSfloat g;
    GSfloat b;
    GSfloat a;

    static GScolor construct(GSfloat r = 0.0f, GSfloat g = 0.0f, GSfloat b = 0.0f, GSfloat a = 1.0f) {
        GScolor result = { r, g, b, a };
        return result;
    }
};


// -----------------------------------------------------------------------------
// xGS error codes
// -----------------------------------------------------------------------------

const GSerror GS_OK                       = 0;       // no error

// system initialization errors (might be set up after xGS interface query)
const GSerror GSE_STARTUP_INITFAILED      = 0x10000; // initialization failed
const GSerror GSE_STARTUP_SUBSYSTEMFAILED = 0x10001; // subsystem initialization failed
const GSerror GSE_STARTUP_INCOMPATIBLE    = 0x10002; // incompatible or unsupported system
// system errors
const GSerror GSE_SYSTEMNOTREADY          = 0x20000; //
const GSerror GSE_SUBSYSTEMFAILED         = 0x20001; //
const GSerror GSE_WINDOWSYSTEMFAILED      = 0x20002; //
const GSerror GSE_INCOMPATIBLE            = 0x20003; //
// general errors
const GSerror GSE_RENDERERNOTREADY        = 0x30000; // renderer should be created
const GSerror GSE_INVALIDOPERATION        = 0x30001; // some state wasn't valid
const GSerror GSE_INVALIDENUM             = 0x30002; // one of enum parameter is invalid
const GSerror GSE_INVALIDVALUE            = 0x30003; // one of value parameter is invalid
const GSerror GSE_INVALIDOBJECT           = 0x30004; // object is invalid (not allocated or already allocated)
const GSerror GSE_INVALIDSTATE            = 0x30005; // current state is invalid for operation
const GSerror GSE_OUTOFRESOURCES          = 0x30006; // some resources couldn't be allocated

const GSerror GSE_UNSUPPORTED             = 0x30100; // feature is not supported in current implementation
const GSerror GSE_UNIMPLEMENTED           = 0x30101; // feature is not implemented in current implementation


// -----------------------------------------------------------------------------
// xGS common values
// -----------------------------------------------------------------------------

// common values for anything
const GSenum GS_NONE           = 0;
const GSbool GS_FALSE          = false;
const GSbool GS_TRUE           = true;
const GSenum GS_ENABLE         = GS_TRUE;
const GSenum GS_DISABLE        = GS_FALSE;
const GSvalue GS_INVALID_VALUE = 0;
const GSenum GS_INVALID_ENUM   = 0;
const GSvalue GS_DEFAULT       = -1;

// common values for GetValue objects (geometry/buffers/textures etc.)
const GSenum GS_DETACHED       = 0x0001; // is object detached from device?
const GSenum GS_EMPTY          = 0x0201; // is object empty and haven't been allocated yet?
const GSenum GS_VALID          = 0x0202; // is object valid and ready to use?
const GSenum GS_LOCKED         = 0x0203; // is object locked (so unlock should be called)?
const GSenum GS_BOUND          = 0x0204; // is object bound somewhere (so its instance will be hold)?
const GSenum GS_OBJECT_FIRST   = 0x0300; // first custom get value for specific object type

// common limitation values
//      defines maximum possible limits for certain stuff
//      actual limit might be queried through caps, but can not be larger
//      than any limit defined here (even if implementation support larger values)
const GSvalue GS_MAX_FB_COLORTARGETS = 8; // maximum color targets for frame buffer
const GSvalue GS_MAX_INPUT_SLOTS     = 8; // maximum state input slots
const GSvalue GS_MAX_PARAMETER_SETS  = 8; // maximum state parameter slot sets
const GSvalue GS_MAX_PARAMETER_SLOTS = 8; // maximum state parameter slots per set

// -----------------------------------------------------------------------------
// xGS object types (for CreateObject)
// -----------------------------------------------------------------------------

const GSenum GS_OBJECTTYPE_GEOMTERY       = 1;
const GSenum GS_OBJECTTYPE_GEOMTERYBUFFER = 2;
const GSenum GS_OBJECTTYPE_UNIFORMBUFFER  = 3;
const GSenum GS_OBJECTTYPE_TEXTURE        = 4;
const GSenum GS_OBJECTTYPE_FRAMEBUFFER    = 5;
const GSenum GS_OBJECTTYPE_STATE          = 6;
const GSenum GS_OBJECTTYPE_INPUT          = 7;
const GSenum GS_OBJECTTYPE_PARAMETERS     = 8;
const GSenum GS_OBJECTTYPE_RENDERLIST     = 9;


// -----------------------------------------------------------------------------
// xGS RT & texture data formats
// -----------------------------------------------------------------------------

// color formats (used for RT and textures)
//  integer RGB* formats (8 bits per component) should be layed out as BGR*
//  floating point RGB* formats (from 16 bits per component) should be layed out as RGB*
const GSenum GS_COLOR_NONE           = GS_NONE;    // no color
const GSenum GS_COLOR_DEFAULT        = GS_DEFAULT; // default color format (for FB - from attachment)
const GSenum GS_COLOR_R              = 0x0500;     // R8
const GSenum GS_COLOR_R_HALFFLOAT    = 0x0501;     // R16
const GSenum GS_COLOR_R_FLOAT        = 0x0502;     // R32
const GSenum GS_COLOR_RG             = 0x0503;     // R8G8
const GSenum GS_COLOR_RG_HALFFLOAT   = 0x0504;     // R16G16
const GSenum GS_COLOR_RG_FLOAT       = 0x0505;     // R32G32
const GSenum GS_COLOR_RGBX           = 0x0506;     // R8G8B8X8     (32)
const GSenum GS_COLOR_RGBX_HALFFLOAT = 0x0507;     // R16G16B16X16 (64 float)
const GSenum GS_COLOR_RGBX_FLOAT     = 0x0508;     // R32G32B32X32 (128 float)
const GSenum GS_COLOR_RGBA           = 0x0509;     // R8G8B8A8     (32)
const GSenum GS_COLOR_RGBA_HALFFLOAT = 0x050A;     // R16G16B16A16 (64 float)
const GSenum GS_COLOR_RGBA_FLOAT     = 0x050B;     // R32G32B32A32 (128 float)

const GSenum GS_COLOR_S_RGBX         = 0x0516;     // R8G8B8X8     (32, sRGB)
const GSenum GS_COLOR_S_RGBA         = 0x0519;     // R8G8B8A8     (32, sRGB)

// depth/stencil formats (used for RT and textures)
const GSenum GS_DEPTH_NONE           = GS_NONE;    // no depth
const GSenum GS_DEPTH_DEFAULT        = GS_DEFAULT; // default depth format (for FB - from attachment)
const GSenum GS_DEPTH_16             = 0x0520;     // 16-bit depth, generaly integer
const GSenum GS_DEPTH_24             = 0x0521;     // 24-bit depth, generaly integer
const GSenum GS_DEPTH_32             = 0x0522;     // 32-bit depth, generaly integer
const GSenum GS_DEPTH_32_FLOAT       = 0x0523;     // 32-bit depth, force float

const GSenum GS_STENCIL_NONE         = GS_NONE;    // no stencil
const GSenum GS_STENCIL_DEFAULT      = GS_DEFAULT; // default stencil format
const GSenum GS_STENCIL_8            = 0x0530;     // 8-bit stencil

const GSenum GS_DEPTHSTENCIL_D24S8   = 0x0531;     // combined depth/stencil (24-bit depth, 8-bit stencil)


// -----------------------------------------------------------------------------
// xGS multisample values
// -----------------------------------------------------------------------------

const GSenum GS_MULTISAMPLE_NONE = GS_NONE;
const GSenum GS_MULTISAMPLE_2X   = 1;
const GSenum GS_MULTISAMPLE_4X   = 4;
const GSenum GS_MULTISAMPLE_8X   = 8;
const GSenum GS_MULTISAMPLE_16X  = 16;


// -----------------------------------------------------------------------------
// xGS index data formats
// -----------------------------------------------------------------------------

const GSenum GS_INDEX_NONE  = GS_NONE;        // no index data
const GSenum GS_INDEX_WORD  = 0x0600;         // 16-bit indices
const GSenum GS_INDEX_16    = GS_INDEX_WORD;  // 16-bit indices
const GSenum GS_INDEX_DWORD = 0x0601;         // 32-bit indices
const GSenum GS_INDEX_32    = GS_INDEX_DWORD; // 32-bit indices


// -----------------------------------------------------------------------------
// xGS vertex declaration data
// -----------------------------------------------------------------------------

const GSenum GSVD_END       = 0;              // end of vertex declaration

const GSenum GSVD_POSITION  = 1;              // 3-component position (xyz, float)
const GSenum GSVD_POS       = GSVD_POSITION;  // 3-component position (xyz, float)
const GSenum GSVD_POS3      = GSVD_POSITION;  // 3-component position (xyz, float)
const GSenum GSVD_POSITIONW = 2;              // 4-component position (xyzw, float)
const GSenum GSVD_POSW      = GSVD_POSITIONW; // 4-component position (xyzw, float)
const GSenum GSVD_POS4      = GSVD_POSITIONW; // 4-component position (xyzw, float)

const GSenum GSVD_VECTOR1   = 3;              // single float component
const GSenum GSVD_VEC1      = GSVD_VECTOR1;   // single float component
const GSenum GSVD_FLOAT     = GSVD_VECTOR1;   // single float component
const GSenum GSVD_VECTOR2   = 4;              // 2 float components
const GSenum GSVD_VEC2      = GSVD_VECTOR2;   // 2 float components
const GSenum GSVD_VECTOR3   = 5;              // 3 float components
const GSenum GSVD_VEC3      = GSVD_VECTOR3;   // 3 float components
const GSenum GSVD_VECTOR4   = 6;              // 4 float components
const GSenum GSVD_VEC4      = GSVD_VECTOR4;   // 4 float components

// TODO: integer components, double, packed components (32-bit 4ub)


// -----------------------------------------------------------------------------
// xGS lock access values
// -----------------------------------------------------------------------------

const GSdword GS_READ      = 1;                  // lock object data for reading only
const GSdword GS_WRITE     = 2;                  // lock object data for writing only
const GSdword GS_READWRITE = GS_READ | GS_WRITE; // lock object data for reading and writing


// -----------------------------------------------------------------------------
// xGS Sampler specific enums and values
// -----------------------------------------------------------------------------

// Sampler index constants
const GSenum GSS_0               = 0;
const GSenum GSS_1               = 1;
const GSenum GSS_2               = 2;
const GSenum GSS_3               = 3;
const GSenum GSS_4               = 4;
const GSenum GSS_5               = 5;
const GSenum GSS_6               = 6;
const GSenum GSS_7               = 7;

// wrap for texture sampling
const GSenum GS_WRAP_REPEAT      = 1;
const GSenum GS_WRAP_CLAMP       = 2;

// filter for texture sampling
const GSenum GS_FILTER_NEAREST   = 1;
const GSenum GS_FILTER_LINEAR    = 2;
const GSenum GS_FILTER_TRILINEAR = 3;

// depth compare values are same as fixed state depth test values


// -----------------------------------------------------------------------------
// xGS Geometry object specific enums and values
// -----------------------------------------------------------------------------

// Geometry object get values
const GSenum GS_GEOMETRY_TYPE          = GS_OBJECT_FIRST + 0;
const GSenum GS_GEOMETRY_VERTEXCOUNT   = GS_OBJECT_FIRST + 1;
const GSenum GS_GEOMETRY_INDEXCOUNT    = GS_OBJECT_FIRST + 2;
const GSenum GS_GEOMETRY_INDEXFORMAT   = GS_OBJECT_FIRST + 3;
const GSenum GS_GEOMETRY_VERTEXBYTES   = GS_OBJECT_FIRST + 4;
const GSenum GS_GEOMETRY_INDEXBYTES    = GS_OBJECT_FIRST + 5;
const GSenum GS_GEOMETRY_PATCHVERTICES = GS_OBJECT_FIRST + 6;
const GSenum GS_GEOMETRY_RESTART       = GS_OBJECT_FIRST + 7;
const GSenum GS_GEOMETRY_RESTARTINDEX  = GS_OBJECT_FIRST + 8;

// geometry data sharing
const GSenum GS_SHARE_ALL              = 1;
const GSenum GS_SHARE_VERTICESONLY     = 2;

// Geometry type (primitive type) values
const GSenum GS_PRIM_POINTS            = 0x0001;
const GSenum GS_PRIM_LINES             = 0x0002;
const GSenum GS_PRIM_LINESTRIP         = 0x0004;
const GSenum GS_PRIM_TRIANGLES         = 0x0008;
const GSenum GS_PRIM_TRIANGLESTRIP     = 0x0010;
const GSenum GS_PRIM_TRIANGLEFAN       = 0x0020;
const GSenum GS_PRIM_PATCHES           = 0x0040;

const GSenum GS_PRIM_ADJACENCY         = 0x1000; // FLAG value, should be ored with others


// -----------------------------------------------------------------------------
// xGS GeometryBuffer object specific enums and values
// -----------------------------------------------------------------------------

const GSenum GS_GB_TYPE              = GS_OBJECT_FIRST + 0;
const GSenum GS_GB_VERTEXCOUNT       = GS_OBJECT_FIRST + 1;
const GSenum GS_GB_VERTEXSIZE        = GS_OBJECT_FIRST + 2;
const GSenum GS_GB_INDEXCOUNT        = GS_OBJECT_FIRST + 3;
const GSenum GS_GB_INDEXFORMAT       = GS_OBJECT_FIRST + 4;
const GSenum GS_GB_VERTEXBYTES       = GS_OBJECT_FIRST + 5;
const GSenum GS_GB_INDEXBYTES        = GS_OBJECT_FIRST + 6;
const GSenum GS_GB_ACCESS            = GS_OBJECT_FIRST + 7;
const GSenum GS_GB_VERTICESALLOCATED = GS_OBJECT_FIRST + 8;
const GSenum GS_GB_INDICESALLOCATED  = GS_OBJECT_FIRST + 9;

const GSenum GS_GBTYPE_STATIC        = 1;
const GSenum GS_GBTYPE_GEOMETRYHEAP  = 2;
const GSenum GS_GBTYPE_IMMEDIATE     = 3;
// TODO: think about feedback buffer type

// TODO: add GB object values

// locktype for buffers
const GSenum GS_LOCK_VERTEXDATA      = 1;
const GSenum GS_LOCK_INDEXDATA       = 2;


// -----------------------------------------------------------------------------
// xGS UniformBuffer object specific enums and values
// -----------------------------------------------------------------------------

// TODO: add UB object values

// block type
const GSenum GSB_END    = GS_NONE;
const GSenum GSB_BLOCK  = 1;

// uniforms
const GSenum GSU_END    = GS_NONE;
const GSenum GSU_SCALAR = 1;
const GSenum GSU_VEC2   = 2;
const GSenum GSU_VEC3   = 3;
const GSenum GSU_VEC4   = 4;
const GSenum GSU_MAT2   = 5;
const GSenum GSU_MAT3   = 6;
const GSenum GSU_MAT4   = 7;


// -----------------------------------------------------------------------------
// xGS Texture object specific enums and values
// -----------------------------------------------------------------------------

// Texture object get values
const GSenum GS_TEX_TYPE        = GS_OBJECT_FIRST + 0;
const GSenum GS_TEX_FORMAT      = GS_OBJECT_FIRST + 1;
const GSenum GS_TEX_WIDTH       = GS_OBJECT_FIRST + 2;
const GSenum GS_TEX_HEIGHT      = GS_OBJECT_FIRST + 3;
const GSenum GS_TEX_DEPTH       = GS_OBJECT_FIRST + 4;
const GSenum GS_TEX_LAYERS      = GS_OBJECT_FIRST + 5;
const GSenum GS_TEX_MIN_LEVEL   = GS_OBJECT_FIRST + 6;
const GSenum GS_TEX_MAX_LEVEL   = GS_OBJECT_FIRST + 7;
const GSenum GS_TEX_MULTISAMPLE = GS_OBJECT_FIRST + 8;

// Texture types
const GSenum GS_TEXTYPE_EMPTY   = 0; // texture is empy (haven't been initialized yet)
const GSenum GS_TEXTYPE_1D      = 1; // 1D texture (width is valid)
const GSenum GS_TEXTYPE_2D      = 2; // 2D texture (width & height are valid)
const GSenum GS_TEXTYPE_RECT    = 3; // Rectangle texture (width & height are valid)
const GSenum GS_TEXTYPE_CUBEMAP = 4; // Cubemap texture (width & height are valid)
const GSenum GS_TEXTYPE_3D      = 5; // 3D texture (width & height & depth are valid)
const GSenum GS_TEXTYPE_BUFFER  = 6; // Buffer based texture (width is valid)

// locktype for textures
const GSenum GS_LOCK_TEXTURE    = 1;
const GSenum GS_LOCK_CUBEMAPPX  = 0x10000;
const GSenum GS_LOCK_CUBEMAPPY  = 0x20000;
const GSenum GS_LOCK_CUBEMAPPZ  = 0x30000;
const GSenum GS_LOCK_CUBEMAPNX  = 0x40000;
const GSenum GS_LOCK_CUBEMAPNY  = 0x50000;
const GSenum GS_LOCK_CUBEMAPNZ  = 0x60000;


// -----------------------------------------------------------------------------
// xGS FrameBuffer object specific enums and values
// -----------------------------------------------------------------------------

// FrameBuffer object get values
const GSenum GS_FB_TYPE          = GS_OBJECT_FIRST + 0;
const GSenum GS_FB_WIDTH         = GS_OBJECT_FIRST + 1;
const GSenum GS_FB_HEIGHT        = GS_OBJECT_FIRST + 2;
const GSenum GS_FB_DOUBLEBUFFER  = GS_OBJECT_FIRST + 3;
const GSenum GS_FB_SWAPMETHOD    = GS_OBJECT_FIRST + 4;
const GSenum GS_FB_COLORTARGETS  = GS_OBJECT_FIRST + 5;
const GSenum GS_FB_SHARECOLOR    = GS_OBJECT_FIRST + 6;
const GSenum GS_FB_DEPTHFORMAT   = GS_OBJECT_FIRST + 7;
const GSenum GS_FB_SHAREDEPTH    = GS_OBJECT_FIRST + 8;
const GSenum GS_FB_STENCILFORMAT = GS_OBJECT_FIRST + 9;
const GSenum GS_FB_SHARESTENCIL  = GS_OBJECT_FIRST + 10;
const GSenum GS_FB_MULTISAMPLE   = GS_OBJECT_FIRST + 11;
const GSenum GS_FB_COLORFORMAT   = GS_OBJECT_FIRST + 12;
const GSenum GS_FB_COLORFORMAT0  = GS_FB_COLORFORMAT;
const GSenum GS_FB_COLORFORMAT1  = GS_FB_COLORFORMAT0 + 1;
const GSenum GS_FB_COLORFORMAT2  = GS_FB_COLORFORMAT0 + 2;
const GSenum GS_FB_COLORFORMAT3  = GS_FB_COLORFORMAT0 + 3;
const GSenum GS_FB_COLORFORMAT4  = GS_FB_COLORFORMAT0 + 4;
const GSenum GS_FB_COLORFORMAT5  = GS_FB_COLORFORMAT0 + 5;
const GSenum GS_FB_COLORFORMAT6  = GS_FB_COLORFORMAT0 + 6;
const GSenum GS_FB_COLORFORMAT7  = GS_FB_COLORFORMAT0 + 7;

// FrameBuffer types
const GSenum GS_FBTYPE_DISPLAY   = 1;
const GSenum GS_FBTYPE_OFFSCREEN = 2;

// TODO: define swap methods

// FrameBuffer attachment points
const GSenum GSA_END             = 0;
const GSenum GSA_COLOR0          = 1;
const GSenum GSA_COLOR1          = 2;
const GSenum GSA_COLOR2          = 3;
const GSenum GSA_COLOR3          = 4;
const GSenum GSA_COLOR4          = 5;
const GSenum GSA_COLOR5          = 6;
const GSenum GSA_COLOR6          = 7;
const GSenum GSA_COLOR7          = 8;
const GSenum GSA_DEPTH           = 0x00010000;
const GSenum GSA_STENCIL         = 0x00010001;


// -----------------------------------------------------------------------------
// xGS State object specific enums and values
// -----------------------------------------------------------------------------

// input layout element type
const GSenum GSI_END                 = GS_NONE;
const GSenum GSI_DYNAMIC             = 1;
const GSenum GSI_STATIC              = 2;

// parameter slot type
const GSenum GSP_END                 = GS_NONE;
const GSenum GSP_DYNAMIC             = 1;
const GSenum GSP_STATIC              = 2;

// parameter type
const GSenum GSPD_END                = GS_NONE;
const GSenum GSPD_CONSTANT           = 1;
const GSenum GSPD_BLOCK              = 2;
const GSenum GSPD_TEXTURE            = 3;

// parameter slot within parameter set
const GSenum GSPS_END                = GS_NONE;
const GSenum GSPS_0                  = 1;
const GSenum GSPS_1                  = 2;
const GSenum GSPS_2                  = 3;
const GSenum GSPS_3                  = 4;
const GSenum GSPS_4                  = 5;
const GSenum GSPS_5                  = 6;
const GSenum GSPS_6                  = 7;
const GSenum GSPS_7                  = 8;


// depth test/compare values
const GSenum GS_DEPTHTEST_NONE       = 0;
const GSenum GS_DEPTHTEST_NEVER      = 1;
const GSenum GS_DEPTHTEST_LESS       = 2;
const GSenum GS_DEPTHTEST_LEQUAL     = 3;
const GSenum GS_DEPTHTEST_EQUAL      = 4;
const GSenum GS_DEPTHTEST_NOTEQUAL   = 5;
const GSenum GS_DEPTHTEST_GREATER    = 6;
const GSenum GS_DEPTHTEST_GEQUAL     = 7;
const GSenum GS_DEPTHTEST_ALWAYS     = 8;

// blend operations
const GSenum GS_BLEND_NONE           = 0;
const GSenum GS_BLEND_ADD            = 1;
const GSenum GS_BLEND_SUBTRACT       = 2;
const GSenum GS_BLEND_MIN            = 3;
const GSenum GS_BLEND_MAX            = 4;

// blend factors
const GSenum GSBF_ZERO               = 0;
const GSenum GSBF_ONE                = 1;
const GSenum GSBF_SRCCOLOR           = 2;
const GSenum GSBF_INVSRCCOLOR        = 3;
const GSenum GSBF_DSTCOLOR           = 4;
const GSenum GSBF_INVDSTCOLOR        = 5;
const GSenum GSBF_SRCALPHA           = 6;
const GSenum GSBF_INVSRCALPHA        = 7;
const GSenum GSBF_DSTALPHA           = 8;
const GSenum GSBF_INVDSTALPHA        = 9;
const GSenum GSBF_CONSTANTCOLOR      = 10;
const GSenum GSBF_INVCONSTANTCOLOR   = 11;
const GSenum GSBF_CONSTANTALPHA      = 12;
const GSenum GSBF_INVCONSTANTALPHA   = 13;
const GSenum GSBF_SRCALPHA_SAT       = 14;
const GSenum GSBF_SRC1COLOR          = 15;
const GSenum GSBF_INVSRC1COLOR       = 16;
const GSenum GSBF_SRC1ALPHA          = 17;
const GSenum GSBF_INVSRC1ALPHA       = 18;

// color write mask
const GSenum GS_CLRWR_R              = 0x1;
const GSenum GS_CLRWR_G              = 0x2;
const GSenum GS_CLRWR_B              = 0x4;
const GSenum GS_CLRWR_A              = 0x8;
const GSenum GS_CLRWR_ALL            = GS_CLRWR_R | GS_CLRWR_G | GS_CLRWR_B | GS_CLRWR_A;

// rasterizer fill values
const GSenum GS_FILL_POINT           = 1;
const GSenum GS_FILL_LINE            = 2;
const GSenum GS_FILL_POLYGON         = 3;

// rasterizer cull values
const GSenum GS_CULL_NONE            = 0;
const GSenum GS_CULL_CW              = 1;
const GSenum GS_CULL_CCW             = 2;

// output layout destinations
const GSenum GSD_END                 = GS_NONE;
const GSenum GSD_FRAMEBUFFER         = 1;
const GSenum GSD_FRAMEBUFFER_INDEXED = 2;
const GSenum GSD_FEEDBACK            = 3;


// -----------------------------------------------------------------------------
// xGS Input object specific enums and values
// -----------------------------------------------------------------------------

const GSenum GSIS_END  = GS_NONE;
const GSenum GSIS_0    = 1;
const GSenum GSIS_1    = 2;
const GSenum GSIS_2    = 3;
const GSenum GSIS_3    = 4;
const GSenum GSIS_4    = 5;
const GSenum GSIS_5    = 6;
const GSenum GSIS_6    = 7;
const GSenum GSIS_7    = 8;



class xGSSystem;

class xGSGeometry;
class xGSGeometryBuffer;
class xGSUniformBuffer;
class xGSTexture;
class xGSFrameBuffer;

class xGSState;
class xGSInput;
class xGSParameters;


// system object
typedef xGSSystem         *IxGS;

// resources
typedef xGSGeometry       *IxGSGeometry;
typedef xGSGeometryBuffer *IxGSGeometryBuffer;
typedef xGSUniformBuffer  *IxGSUniformBuffer;
typedef xGSTexture        *IxGSTexture;
typedef xGSFrameBuffer    *IxGSFrameBuffer;

// states
typedef xGSState          *IxGSState;
typedef xGSInput          *IxGSInput;
typedef xGSParameters     *IxGSParameters;

typedef InterfacePtr<IxGS>               IxGSRef;
typedef InterfacePtr<IxGSGeometry>       IxGSGeometryRef;
typedef InterfacePtr<IxGSGeometryBuffer> IxGSGeometryBufferRef;
typedef InterfacePtr<IxGSUniformBuffer>  IxGSUniformBufferRef;
typedef InterfacePtr<IxGSTexture>        IxGSTextureRef;
typedef InterfacePtr<IxGSFrameBuffer>    IxGSFrameBufferRef;
typedef InterfacePtr<IxGSState>          IxGSStateRef;
typedef InterfacePtr<IxGSInput>          IxGSInputRef;
typedef InterfacePtr<IxGSParameters>     IxGSParametersRef;


// Renderer description structure
//      GS_DEFAULT value for modeindex will left video mode unchanged
//      GS_DEFAULT value for format and multisample will choose best available values for
//      default render target. If some of buffer's surfaces not needed - format value should
//      be set to GS_NONE
//
struct GSrendererdescription
{
    GSwidget widget;        // windowing system handle

    GSint    modeindex;     // full screen video mode index (this mode will be set)
    GSbool   fullscreen;    // run in full screen mode

    GSbool   doublebuffer;  // use double buffer
    GSenum   colorformat;   // default color buffer format
    GSenum   depthformat;   // default depth buffer format
    GSenum   stencilformat; // default stencil buffer format
    GSenum   multisample;   // multisample for default buffer

    static GSrendererdescription construct(GSwidget _widget = 0)
    {
        GSrendererdescription result = {
            _widget,
            GS_DEFAULT, GS_FALSE, GS_TRUE,
            GS_DEFAULT, GS_DEFAULT, GS_DEFAULT, GS_DEFAULT
        };
        return result;
    }
};


// descriptors used by several object types

struct GSvertexcomponent
{
    GSdword     type;
    GSenum      semantic;
    GSint       index;
    const char *name;
};

struct GSuniformbinding
{
    GSenum            slot;
    IxGSUniformBuffer buffer;
    GSuint            block;
    GSuint            index;
};

struct GStexturebinding
{
    GSenum       slot;
    IxGSTexture  texture;
    GSenum       sampler;
};

struct GSconstantvalue
{
    GSenum       slot;
    GSenum       type;
    GSfloat     *value;
};


struct GSgeometrydescription
{
    GSenum             type;           // primitive type
    GSenum             indexformat;    // index format
    GSint              vertexcount;    // vertex count
    GSint              indexcount;     // index count
    GSuint             patchvertices;  // number of vertices in patch for patch primitive type
    GSbool             restart;        // index data has special restart index
    GSuint             restartindex;   // restart index value

    IxGSGeometryBuffer buffer;         // buffer where geometry vertex and index data stored
    IxGSGeometry       sharedgeometry; // geometry with which data should be shared
    GSenum             sharemode;      // data sharing mode

    static GSgeometrydescription construct()
    {
        GSgeometrydescription result = {
            GS_NONE,
            GS_INDEX_NONE,
            0, 0, 0, GS_FALSE, 0,
            nullptr, nullptr, GS_NONE
        };
        return result;
    }
};

struct GSgeometrybufferdescription
{
    GSenum                   type;
    GSuint                   vertexcount;
    GSuint                   indexcount;
    const GSvertexcomponent *vertexdecl;
    GSenum                   indexformat;
    GSdword                  flags;

    static GSgeometrybufferdescription construct()
    {
        GSgeometrybufferdescription result = {};
        return result;
    }
};

struct GSuniform
{
    GSenum type;
    GSuint count;
};

struct GSuniformblock
{
    GSenum           type;
    const GSuniform *uniforms;
    GSuint           count;
};

struct GSuniformbufferdescription
{
    const GSuniformblock *blocks;
    GSuint                flags;
};

struct GSuniformblockinfo
{
    GSuint offset; // offset to the 1st block in buffer
    GSuint size;   // aligned size of the block (single block, not multiplied by count)
    GSuint count;  // number of consecutive blocks in buffer, starting from offset
};

struct GSuniforminfo
{
    GSenum type;      // uniform type
    GSuint offset;    // offset relative to block start
    GSuint size;      // element size
    GSuint totalsize; // aligned total array size
    GSuint count;     // number of uniforms in array (1 - for single value)
};

struct GStexturedescription
{
    GSenum type;        //
    GSenum format;      //
    GSuint width;       //
    GSuint height;      //
    GSuint depth;       //
    GSuint layers;      //
    GSuint minlevel;    //
    GSuint maxlevel;    //
    GSenum multisample; //

    static GStexturedescription construct()
    {
        GStexturedescription result = {
            GS_NONE, GS_NONE,
            0, 0, 0, 0, 0, 1000,
            GS_NONE
        };
        return result;
    }
};

struct GSsamplerdescription
{
    GSenum  wrapu;        //
    GSenum  wrapv;        //
    GSenum  wrapw;        //
    GSenum  filter;       //
    GSint   minlod;       //
    GSint   maxlod;       //
    GSfloat bias;         //
    GSenum  depthcompare; //

    // TODO: anisotropy

    static GSsamplerdescription construct()
    {
        GSsamplerdescription result = {
            GS_WRAP_REPEAT, GS_WRAP_REPEAT, GS_WRAP_REPEAT,
            GS_FILTER_NEAREST, -1000, 1000, 0.0f,
            GS_NONE
        };
        return result;
    }
};

struct GSframebufferattachment
{
    GSenum      attachment;
    IxGSTexture texture;
    GSuint      level;
    GSuint      slice;
};

struct GSframebufferdescription
{
    GSuint                         width;
    GSuint                         height;
    GSuint                         colortargets;
    GSenum                         colorformat[GS_MAX_FB_COLORTARGETS];
    GSenum                         depthstencilformat;
    GSenum                         multisample;
    const GSframebufferattachment *attachments;

    static GSframebufferdescription construct()
    {
        GSframebufferdescription result = {};
        result.depthstencilformat = GS_DEPTH_DEFAULT;
        for (size_t n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
            result.colorformat[n] = GS_COLOR_DEFAULT;
        }
        return result;
    }
};

struct GSstencilparameters
{
    GSenum test;
    GSuint ref;
    GSuint mask;
    GSenum stencilfail;
    GSenum depthfail;
    GSenum depthpass;

    static GSstencilparameters construct()
    {
        GSstencilparameters result = {};
        return result;
    }
};

struct GSdepthstencilstate
{
    GSenum              depthtest;
    GSbool              depthmask;

    GSbool              stencilstest;
    GSstencilparameters front;
    GSstencilparameters back;

    static GSdepthstencilstate construct()
    {
        GSdepthstencilstate result = {
            GS_DEPTHTEST_LEQUAL, GS_TRUE
        };
        return result;
    }
};

struct GSblendparameters
{
    GSenum colorop;
    GSenum alphaop;
    GSenum src;
    GSenum dst;
    GSenum srcalpha;
    GSenum dstalpha;

    static GSblendparameters construct()
    {
        GSblendparameters result = {
            GS_BLEND_NONE, GS_BLEND_NONE,
            GSBF_SRCALPHA, GSBF_INVSRCALPHA,
            GSBF_SRCALPHA, GSBF_INVSRCALPHA
        };
        return result;
    }
};

struct GSblendsate
{
    GSbool            alphatocoverage;
    GSbool            separate;
    GSblendparameters parameters[GS_MAX_FB_COLORTARGETS];
    GSuint            writemask;

    static GSblendsate construct()
    {
        GSblendsate result = {};
        for (size_t n = 0; n < GS_MAX_FB_COLORTARGETS; ++n) {
            result.parameters[n] = GSblendparameters::construct();
        }
        result.writemask = GS_CLRWR_ALL;
        return result;
    }
};

struct GSrasterizerstate
{
    GSenum fill;
    GSenum cull;
    GSbool multisample;
    GSbool sampleshading;
    GSuint polygonoffset;
    GSbool discard;

    static GSrasterizerstate construct()
    {
        GSrasterizerstate result = {
            GS_FILL_POLYGON,
            GS_CULL_CCW
        };
        return result;
    }
};

struct GSinputlayout
{
    GSenum                   slottype; // type of input slot
    const GSvertexcomponent *decl;     // declaration for slot (can have more components)
    GSuint                   divisor;  // per-vertex/per-instance data?

    IxGSGeometryBuffer       buffer;   // staticaly bound input buffer
};

struct GSparameterdecl
{
    GSenum      type;     // texture slot, uniform constant, uniform block
    GSint       location; // block index or location
    GSuint      index;    // array index for array uniforms
    const char *name;     // name of parameter
};

struct GSparameterlayout
{
    GSenum                  settype;
    const GSparameterdecl  *parameters;

    const GSuniformbinding *uniforms;
    const GStexturebinding *textures;
};

struct GSoutputlayout
{
    GSenum      destination;
    GSenum      index;
    GSint       location;
    const char *name;
};

/*
    State object description struct

        INPUT
            Input description divided in three parts: attributes, streams and buffers

            Attribute is a input "unit" of input data. Attribute corresponds to vertex shader
            attribute. Individual attributes are defined with vertex declaration - array of
            GSvertexcomponent elements.

            Attribute can be sourced from buffer or be constant across draw call. If program
            contains attributes which weren't declared in any stream - these attributes become
            constant attributes.

            Attributes are grouped into streams and streams are defined by set of
            GSinputlayout structs. Each stream can be per-vertex or per-instance, this is
            indicated by divisor field of GSinputlayout.

            Streams are linked to buffers in order. State can has multiple sets of buffers and
            only one active set during draw call. Active buffers set can be changed with
            renderer command.

        SHADERS

        FIXED STATE

        OUTPUT
*/
struct GSstatedescription
{
    const GSinputlayout     *inputlayout;

    const char             **vs;
    const char             **cs;
    const char             **es;
    const char             **gs;
    const char             **ps;

    const GSparameterlayout *parameterlayout;

    GSdepthstencilstate      depthstencil;
    GSblendsate              blend;
    GSrasterizerstate        rasterizer;

    const GSoutputlayout    *output;

    GSenum                   colorformats[GS_MAX_FB_COLORTARGETS];
    GSenum                   depthstencilformat;

    static GSstatedescription construct()
    {
        GSstatedescription result = {
            nullptr,                                     // input
            nullptr, nullptr, nullptr, nullptr, nullptr, // shaders
            nullptr,                                     // parameters
            GSdepthstencilstate::construct(),
            GSblendsate::construct(),
            GSrasterizerstate::construct(),
            nullptr                                      // output
        };
        return result;
    }
};


struct GSinputbinding
{
    GSenum             slot;
    IxGSGeometryBuffer buffer;
};

struct GSinputdescription
{
    IxGSState             state;
    const GSinputbinding *bindings;
};


struct GSparametersdescription
{
    IxGSState               state;
    GSuint                  set;
    const GSuniformbinding *uniforms;
    const GStexturebinding *textures;
    const GSconstantvalue  *constants;
    // TODO: define behaviour for arrays
};


// structs for passing to functions
struct GSimmediateprimitive
{
    GSptr  vertexdata;
    GSuint vertexcount;
    GSuint stride;
    GSptr  indexdata;
    GSuint indexcount;
};


/*
 -------------------------------------------------------------------------------
 xGSObject
 -------------------------------------------------------------------------------
    Basic xGS object interface, all objects that need this functionality are
    inherited from xGSObject

        GetValue - get common or specific object value

    Other common stuff for xGS objects (not included here, every object has its own set
    of parameters for each function):

        Allocate
            After creation with CreateObject system function object is in EMPTY state and has
            no associated data. To finally create object and make it usable one must call
            Allocate function. In general it takes description structure with object
            parameters. Every type of object has its own description structure to pass to
            Allocate function.

        Lock/Unlock
            Objects which have mutable data (e.g. texture image data, uniform data and so on)
            have these methods to get pointer for data access. In general Lock has different
            parameters, but there're can be some common:
                locktype - determines which part (or data set) of object to lock
                access   - determines access to data, read or write or both
                lockdata - structure which will receive detailed lock parameters

    All objects are treated as immutable objects, that means that object parameters can not
    be changed after its allocation. Some objects have mutable data, e.g. textures have
    associated image data which is mutable (but its format and parameters are not).

    For efficiency, buffer objects can be created with immutable data as well - the object
    can be locked only once for writing.
*/
class xGSObject : public IUnknownStub
{
public:
    virtual GSvalue xGSAPI GetValue(GSenum valuetype) = 0;
};


/*
 -------------------------------------------------------------------------------
 xGSGeometry
 -------------------------------------------------------------------------------
    Geometry xGS object interface

    This object holds geometry description data: which primitive type to draw, where
    primitive data located inside buffer, how many vertices/indices it has and other
    parameters.

        Following specific values defined for this object type (can be queried with GetValue):
            GS_GEOMETRY_VERTEXCOUNT   - count of vertices in geometry
            GS_GEOMETRY_INDEXCOUNT    - count of indices in geometry (if geometry is indexed)
            GS_GEOMETRY_INDEXFORMAT   - format of index data (should be NONE or match geometry buffer index format)
            GS_GEOMETRY_VERTEXBYTES   - amount of bytes of memory needed to store vertex data
            GS_GEOMETRY_INDEXBYTES    - amount of bytes of memory needed to store index data
            GS_GEOMETRY_PATCHVERTICES - vertex count in patch, applicable only for GS_PRIM_PATCHES
            GS_GEOMETRY_RESTART       - index data has special primitive restart index value
            GS_GEOMETRY_RESTARTINDEX  - special value for primitive restart

        Allocate - initialize geometry object and allocate its data based on
                   GSgeometrydescription structure

        Lock     - lock part of geometry buffer specific to this geometry object
                   following data can be locked for access from CPU side:
                        GS_LOCK_VERTEXDATA - lock vertex data
                        GS_LOCK_INDEXDATA  - lock index data

                   only part of geometry buffer is locked. It is more efficient to lock entire
                   geometry buffer for data upload, and lock geometry object for small data
                   updates

        Unlock   - unlock part of geometry buffer
*/
class xGSGeometry : public xGSObject
{
public:
    virtual GSptr  xGSAPI Lock(GSenum locktype, GSdword access, void *lockdata) = 0;
    virtual GSbool xGSAPI Unlock() = 0;
};


/*
 -------------------------------------------------------------------------------
 xGSGeometryBuffer
 -------------------------------------------------------------------------------
    GeometryBuffer xGS object interface

    This object holds geometry data: vertex and index data.

        Following specific values defined for this object type (can be queried with GetValue):
            GS_GB_TYPE              - type of geometry buffer (geometry allocation behavior)
            GS_GB_VERTEXCOUNT       - total vertex count
            GS_GB_VERTEXSIZE        - size of one vertex in bytes
            GS_GB_INDEXCOUNT        - total index count
            GS_GB_INDEXFORMAT       - index format
            GS_GB_VERTEXBYTES       - amount of bytes of memory needed to store all vertex data
            GS_GB_INDEXBYTES        - amount of bytes of memory needed to store all index data
            GS_GB_ACCESS            - buffer acces type
            GS_GB_VERTICESALLOCATED - amount of allocated vertices
            GS_GB_INDICESALLOCATED  - amount of allocated indices

        Allocate - initialize geometry buffer object and allocate its data based on
                   GSgeometrybufferdescription structure

        Lock     - lock entire geometry buffer
                   following data can be locked for access from CPU side:
                        GS_LOCK_VERTEXDATA - lock vertex data
                        GS_LOCK_INDEXDATA  - lock index data

        Unlock   - unlock geometry buffer
*/
class xGSGeometryBuffer : public xGSObject
{
public:
    virtual GSptr  xGSAPI Lock(GSenum locktype, GSdword access, void *lockdata) = 0;
    virtual GSbool xGSAPI Unlock() = 0;
};


/*
 -------------------------------------------------------------------------------
 xGSUniformBuffer
 -------------------------------------------------------------------------------
    UniformBuffer xGS object interface

    This object holds uniform data for shader programs.

    Following specific values defined for this object type (can be queried with GetValue):
        TODO
*/
class xGSUniformBuffer : public xGSObject
{
public:
    virtual GSbool xGSAPI Update(GSuint offset, GSuint size, const GSptr data) = 0;
    virtual GSbool xGSAPI UpdateBlock(GSuint block, GSuint index, const GSptr data) = 0;
    virtual GSbool xGSAPI UpdateValue(GSuint block, GSuint index, GSuint uniform, GSuint uniformindex, GSuint count, const GSptr data) = 0;

    virtual GSptr  xGSAPI Lock(GSdword access, void *lockdata) = 0;
    virtual GSbool xGSAPI Unlock() = 0;
};


/*
 -------------------------------------------------------------------------------
 xGSTexture
 -------------------------------------------------------------------------------
    Texture xGS object interface

    This object holds texture image data. Image data is mutable.

        Following specific values defined for this object type (can be queried with GetValue):
            GS_TEX_TYPE        - texture type (1D, 2D, 3D, Cubemap, etc.)
            GS_TEX_FORMAT      - image data format (one of GS_COLOR/GS_DEPTH values)
            GS_TEX_WIDTH       - texture base level width (or size for BUFFER texture)
            GS_TEX_HEIGHT      - texture base level height
            GS_TEX_DEPTH       - texture base level depth (only for 3D textures)
            GS_TEX_LAYERS      - texture layers (if greater than 0 - array texture is created)
            GS_TEX_MIN_LEVEL   - texture base MIP level
            GS_TEX_MAX_LEVEL   - texture maximum MIP level
            GS_TEX_MULTISAMPLE - texture multisampling

        Allocate - initialize texture object and allocate its data based on
                   GStexturedescription structure

        Lock     - lock texture image
                   following data can be locked for access from CPU side:
                        GS_LOCK_TEXTURE   - texture image layer of given MIP level for non cubemap texture
                        GS_LOCK_CUBEMAPPX - cubemap positive X face
                        GS_LOCK_CUBEMAPPY - cubemap positive Y face
                        GS_LOCK_CUBEMAPPZ - cubemap positive Z face
                        GS_LOCK_CUBEMAPNX - cubemap negative X face
                        GS_LOCK_CUBEMAPNY - cubemap negative Y face
                        GS_LOCK_CUBEMAPNZ - cubemap negative Z face

                   level - indicates which MIP level to lock
                   layer - indicates which array layer to lock

        Unlock   - unlock texture image
*/
class xGSTexture : public xGSObject
{
public:
    virtual GSptr  xGSAPI Lock(GSenum locktype, GSdword access, GSint level, GSint layer, void *lockdata) = 0;
    virtual GSbool xGSAPI Unlock() = 0;
};


/*
 -------------------------------------------------------------------------------
 xGSFrameBuffer
 -------------------------------------------------------------------------------
    FrameBuffer xGS object interface

    This object holds frame buffer configuration and data for static parts.

        Following specific values defined for this object type (can be queried with GetValue):
            GS_FB_TYPE          - type of framebuffer* (default screen fb or offscreen)
            GS_FB_WIDTH         - width of the frame buffer
            GS_FB_HEIGHT        - height of the frame buffer
            GS_FB_DOUBLEBUFFER  - is double buffering enabled*
            GS_FB_SWAPMETHOD    - swap method for double buffer configuration*
            GS_FB_COLORTARGETS  - number of color targets
            GS_FB_DEPTHFORMAT   - format for depth attachment
            GS_FB_STENCILFORMAT - format for stencil attachment
            GS_FB_MULTISAMPLE   - multisampling
            GS_FB_COLORFORMATn  - format for n-th color target

        Allocate - initialize frame buffer object based on GSframebufferdescription structure

        * these values are applicable only for default onscreen frame buffer and always
        return constant values for xGSFrameBuffer object

        Frame buffer consists of several compontents which are called render targets. Every
        render target can hold color data or depth/stencil data. The format value for render
        target determines its behavior. Render target can be internal (held by frame buffer
        object) or external (texture object attached to FB as render target).

        For every render target two special values can be set:
            GS_NONE - indicates that frame buffer doesn't have this render target and nothing
            can be attached to this attachment point

            GS_DEFAULT - indicates that this render target will be external and
            texture can be attached as render target for this attachment point

            If specific format is set for render target - frame buffer object internally
            creates storage for this RT of desired format

        If frame buffer is multisampled - all render targets should be without multisample or
        should be multisampled targets with same multisample value. In case FB is multisampled
        and render targets are not multisampled - MS resolve operation is automatically done on
        frame buffer unbind.

*/
class xGSFrameBuffer : public xGSObject
{};


/*
 -------------------------------------------------------------------------------
 xGSState
 -------------------------------------------------------------------------------
    State xGS object interface

    This object holds allmost all graphics pipeline state.
*/
class xGSState : public IUnknownStub
{};


/*
 -------------------------------------------------------------------------------
 xGSInput
 -------------------------------------------------------------------------------
    Input xGS object interface

    This object holds geometry buffer bindings to state input slots.
*/
class xGSInput : public IUnknownStub
{};


/*
 -------------------------------------------------------------------------------
 xGSParameters
 -------------------------------------------------------------------------------
    Parameters xGS object interface

    This object holds parameters for state object (mostly - resource bindings)
*/
class xGSParameters : public IUnknownStub
{};


class xGSRenderList : public IUnknownStub
{};


class xGSSystem : public IUnknownStub
{
public:
    // RENDERER
    virtual GSbool xGSAPI CreateRenderer(const GSrendererdescription &desc) = 0;
    virtual GSbool xGSAPI DestroyRenderer(GSbool restorevideomode) = 0;

    // OBJECTS
    virtual GSbool xGSAPI CreateObject(GSenum type, const void *desc, void **result) = 0;
    virtual GSbool xGSAPI CreateSamplers(const GSsamplerdescription *samplers, GSuint count) = 0;

    // QUERY API
    virtual GSbool xGSAPI GetRenderTargetSize(GSsize &size) = 0;

    // RENDERING
    virtual GSbool xGSAPI Clear(GSbool color, GSbool depth, GSbool stencil, const GScolor &colorvalue, float depthvalue, GSdword stencilvalue) = 0;
    virtual GSbool xGSAPI Display() = 0;

    virtual GSbool xGSAPI SetRenderTarget(IxGSFrameBuffer rendertarget) = 0;

    virtual GSbool xGSAPI SetState(IxGSState state) = 0;
    virtual GSbool xGSAPI SetInput(IxGSInput input) = 0;
    virtual GSbool xGSAPI SetParameters(IxGSParameters parameters) = 0;

    virtual GSbool xGSAPI SetViewport(const GSrect &viewport) = 0;
    virtual GSbool xGSAPI SetStencilReference(GSuint ref) = 0;
    virtual GSbool xGSAPI SetBlendColor(const GScolor &color) = 0;
    virtual GSbool xGSAPI SetUniformValue(GSenum set, GSenum slot, GSenum type, const void *value) = 0;

    virtual GSbool xGSAPI DrawGeometry(IxGSGeometry geometry) = 0;
    virtual GSbool xGSAPI DrawGeometryInstanced(IxGSGeometry geometry, GSuint count) = 0;
    virtual GSbool xGSAPI DrawGeometries(IxGSGeometry *geometries, GSuint count) = 0;
    virtual GSbool xGSAPI DrawGeometriesInstanced(IxGSGeometry *geometries, GSuint count, GSuint instancecount) = 0;

    virtual GSbool xGSAPI BeginCapture(GSenum mode, IxGSGeometryBuffer buffer) = 0;
    virtual GSbool xGSAPI EndCapture(GSuint *elementcount) = 0;

    virtual GSbool xGSAPI BeginImmediateDrawing(IxGSGeometryBuffer buffer, GSuint flags) = 0;
    virtual GSbool xGSAPI ImmediatePrimitive(GSenum type, GSuint vertexcount, GSuint indexcount, GSuint flags, GSimmediateprimitive *primitive) = 0;
    virtual GSbool xGSAPI EndImmediateDrawing() = 0;

    virtual GSbool xGSAPI BuildMIPs(IxGSTexture texture) = 0;
};


#ifdef GS_STATIC_LINK
GSbool Create(IxGS *xgs);
#else
extern "C" {
    GSbool xGSAPI xGSCreate(IxGS *xgs);
}
#endif

template <typename T>
static inline void Release(T &ref)
{
    if (ref) {
        ref->Release();
        ref = nullptr;
    }
}

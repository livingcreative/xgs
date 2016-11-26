# xgs
3D Graphics API low-level abstraction layer

[![Build status](https://ci.appveyor.com/api/projects/status/hcv94urqio08trux?svg=true)](https://ci.appveyor.com/project/livingcreative/xgs)

## What is it
xgs is easy to use C++ API for rendering 3D graphics. Its main purpose is to make 3D graphics
programming easier and hide nasty details of platform specific graphics APIs. It's crossplatform
and has native implementation for supported platforms (currently OpenGL 4+ implementation available).
It's suitable for rendering modern 3D graphics. It can be used as a low-level layer for 3D
visualization and interactive game rendering.   

Please note that xgs is not a full graphics or game engine - it's only a thin layer which simplifies
interaction with 3D hardware, it does not provide any pre-made rendering techniques, shaders or
other data. However, most of modern rendering techniques are easier to implement with xgs.

> This project is under development now, so not all of its features are implemented and not all   
> platforms are fully supported

## What it does
xgs API provides an easy way to set-up your 3D rendering pipeline and render modern 3D graphics. 
However, mastering of your 3D content, shaders and rendering techniques is up to you.

Screenshot of xgs demo application (eventually more pleasant demo will be released with more implemented features):   
![xgs demo image](https://raw.githubusercontent.com/livingcreative/xgs/master/demo.jpg)

**Features**
* GCN friendly abstraction
* Modern shader model (with geometry and tesselation shaders support)
* Storage for arbitrary 3D geometry data via static and dynamic buffers
* Different texture types (2D, 3D, cube, arrays and multisampled)
* Dynamic immediate rendering (specify geometry data inplace instead of buffers)
* Floating point and sRGB rendering

**Upcoming features**
* Unified shader language model
* Compute support
* Multithreading and rendering lists (command lists)
* Sparse (tiled) textures and buffers
* GPU - host, GPU - GPU synchronization
* Precise timing

## How it works
Most of the API tied to main xgs object accessed via `xGS` interface. `xGS` object serves for
API initialization, resource and state objects construction and accepts rendering commands.   
> As of current version there's no rendering list and multithreading support and all rendering   
> commands tied to xGS system object. This will be changed in near future.   

There are two groups of xgs objects: resource and state objects. All objects are immutable and
their properties can not be changed after object has been created, however data inside resource
objects can be changed (but not its format or layout).

Resource objects provide data to 3D pipeline and describe data format and layout.
* Geometry object (accessed via `xGSGeometry` interface) stores geometry properties (primitive type and data location inside buffer).
* Geometry buffer object (accessed via `xGSGeometryBuffer` interface) stores vertex and index data.
* Data buffer object (accessed via `xGSDataBuffer` interface) stores arbitrary data (shader parameters).
* Texture object (accessed via `xGSTexture`) stores texture image data.

State objects provide information about 3D pipeline configuration.
* Frame buffer object (accessed via `xGSFrameBuffer` interface) holds configuration for render target.
* State object (accessed via `xGSState` interface) holds most configuration for programmable and fixed pipeline.
* Input object (accessed via `xGSInput` interface) holds configuration for fetching vertex data to shaders from geometry buffers.
* Parameters object (accessed via `xGSParameters` interface) holds values and bindings of data buffers to programmable pipeline.

All xgs objects are constructed via passing their description struct with all required parameters.

In general, before issuing rendering command (most of the time this is geometry rendering command)
you ensure that correct state is set up view few state objects (frame buffer, state, input and parameters). Small amount of
setup (due to few state objects) allows better performance and reduces possible errors.

## How to use
Having proper setup of your project for using xgs library (correct path to xgs include directory and xgs library
linked into project) you only need to include `xGS/xGS.h` header in your source to get all xgs API stuff.

In general 3D API is a bit complex compared to other stuff, so even simple example requires
several steps (so, it can't be called "quick" example).

To be able to use xgs system you need to obtain xgs system object:
```c++
IxGS gs = nullptr;
xGSCreate(&gs);
```

Having correct pointer to xgs object, you need to create renderer:
```c++
// set-up renderer description struct
GSrendererdescription renderdesc = GSrendererdescription::construct();
renderdesc.widget = GSwidget(WindowHandle);
// here you can set specific properties for default RT
// but it should work fine with default ones

// create renderer
gs->CreateRenderer(renderdesc);
```
Renderer is a part of xgs system and there's no separate object for it. It can be treaded as a
internal state - renderer exists after successful call to `CreateRenderer` up until call to `DestroyRenderer`. 
After renderer setup all xgs stuff is available, you can create other xgs objects and issue rendering
commands.

To render even simple geometry you need a place to store geometry data. That's geometry buffer
object is needed for. Here's example of simple geometry buffer allocation:
```c++
// declare vertex data layout
GSvertexcomponent vertex_decl[] = {
	GSVD_POS, 0, GS_DEFAULT, "pos", // vertex position
    GSVD_END                        // end of declaration
};

// set-up geometry buffer description struct
GSgeometrybufferdescription gbdesc = {
    GS_GBTYPE_STATIC, // buffer type - static
	3, 0,             // 3 vertices and 0 indices
    vertex_decl,      // vertex data layout declaration
    GS_INDEX_NONE,    // no indices
    0                 // no flags
};

// create geometry buffer object
IxGSGeometryBuffer buffer = nullptr;
gs->CreateObject(GS_OBJECTTYPE_GEOMETRYBUFFER, &gbdesc, reinterpret_cast<void**>(&buffer));

// vertex data
float vertices[] = {
    -1, -1, 0,
     0,  1, 0,
     1, -1, 0
};

// lock buffer and fill in vertex data
GSptr p = buffer->Lock(GS_LOCK_VERTEXDATA, 0, nullptr));
memcpy(p, vertices, sizeof(vertices));
buffer->Unlock();
```

After that you need geometry object, which holds information about primitive type and
location inside buffer. For static buffers geometry allocated contiguously in order of
geometry object construction.
```c++
// set-up geometry description struct
GSgeometrydescription gdesc = GSgeometrydescription::construct();
gdesc.type = GS_PRIM_TRIANGLES;    // render triangles
gdesc.indexformat = GS_INDEX_NONE; // no indexing
gdesc.vertexcount = 3;             // 3 vertices
gdesc.indexcount = 0;              // no indices
gdesc.buffer = buffer;             // buffer in which geometry will be allocated

// create geometry object
IxGSGeometry geometry = nullptr;
gs->CreateObject(GS_OBJECTTYPE_GEOMETRY, &gdesc, reinterpret_cast<void**>(&geometry));
```

The final and most important object for proper xgs work is state object. State object
defines most of the 3D pipeline configuration.
> Currently shader source should be in GLSL because there's only OpenGL implementation available.   
> Eventually unified shader language will be developed specially for xgs   

```c++
// state object needs input configuration, geometry buffer object
// can be statically bound to state and this feature is used here for simplicity
GSinputlayout input[] = {
    GSI_STATIC, vertex_decl, 0, buffer,
    GSI_END
};

// specify shader source, vertex and pixel shaders is a minimum requirement for
// rendering to work
const char *vs[] = {
    "#version 400 core\n"
    "in vec3 pos;\n"
    "void main() {\n"
    "    gl_Position = vec4(pos, 1);\n"
    "}\n",
    nullptr
};

const char *ps[] = {
    "#version 400 core\n"
    "out vec4 result;"
    "void main() {\n"
    "    result = vec4(0, 1, 0, 1);\n"
    "}\n",
    nullptr
};

// set-up shader parameters layout
GSparameterlayout parameters[] = {
    GSP_END // end of declaration, no parameters
};

// set-up state object description struct
GSstatedescription statedesc = GSstatedescription::construct();
statedesc.inputlayout = input;
statedesc.vs = vs;
statedesc.ps = ps;
statedesc.parameterlayout = parameters;
statedesc.colorformats[0] = GS_COLOR_DEFAULT;
statedesc.depthstencilformat = GS_DEPTH_DEFAULT;

// create state object
IxGSState state = nullptr;
gs->CreateObject(GS_OBJECTTYPE_STATE, &statedesc, reinterpret_cast<void**>(&state));
```

Finally everything is set-up and xgs system now ready for rendering, here's example for
simple rendering loop:
```c++
// get current RT size
GSsize sz;
gs->GetRenderTargetSize(sz);

// clear RT
gs->Clear(true, true, false, GScolor::construct(0.4f, 0.7f, 1.0f), 1.0f, 0);

// define viewport to cover full RT area
GSrect vp = { 0, 0, sz.width, sz.height };
gs->SetViewport(vp);

// set state
gs->SetState(state);

// render triangle geometry
gs->DrawGeometry(geometry);

// display result
gs->Display();
```

To proper shutdown xgs you need to clean-up all objects, destroy renderer and release xgs
system object:
```c++
state->Release();
geometry->Release();
buffer->Release();

gs->DestroyRenderer();
gs->Release();
```

## How to build
> Build instructions will be provided soon

## Copyright and licensing
xgs 3D Graphics API layer

Copyright (C) 2015 – 2016, livingcreative (https://github.com/livingcreative)   
All rights reserved.

Redistribution and use in source and/or binary forms, with or without 
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* The name of the author may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"   
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE   
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE   
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR   
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF   
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS   
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN   
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)   
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF   
THE POSSIBILITY OF SUCH DAMAGE.

[License file](https://raw.githubusercontent.com/livingcreative/xgs/master/license.txt)

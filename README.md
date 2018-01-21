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

See [wiki example](https://github.com/livingcreative/xgs/wiki/Examples#1-triangle) of triangle rendering

## How to build
> Build instructions will be provided soon

## Copyright and licensing
xgs 3D Graphics API layer

Copyright (C) 2015 – 2018, livingcreative (https://github.com/livingcreative)   
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

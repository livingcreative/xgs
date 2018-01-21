/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/mac/glplatform.h
        OpenGL platform header for Mac OS X
*/

#pragma once

#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>

#define GS_CONFIG_TEXTURE_STORAGE
//#define GS_CONFIG_TEXTURE_STORAGE_MULTISAMPLE // not supported
#define GS_CONFIG_MAP_BUFFER_RANGE
//#define GS_CONFIG_FRAMEBUFFER_EXT // not supported
//#define GS_CONFIG_SEPARATE_VERTEX_FORMAT // not supported

#define GS_CAPS_MULTI_BIND           false
#define GS_CAPS_MULTI_BLEND          true // core
#define GS_CAPS_VERTEX_FORMAT        false
#define GS_CAPS_TEXTURE_SRGB         true // core
#define GS_CAPS_TEXTURE_FLOAT        true // core
#define GS_CAPS_TEXTURE_DEPTH        true // core
#define GS_CAPS_TEXTURE_DEPTHSTENCIL true // core

// TODO: think about this
#define glBindTextures(...)
#define glBindSamplers(...)


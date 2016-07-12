/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    win/glplatform.h
        OpenGL platform header for Windows
*/

#pragma once

#include "GL/glew.h"

#define GS_CONFIG_TEXTURE_STORAGE
#define GS_CONFIG_TEXTURE_STORAGE_MULTISAMPLE
#define GS_CONFIG_MAP_BUFFER_RANGE
#define GS_CONFIG_FRAMEBUFFER_EXT
#define GS_CONFIG_SEPARATE_VERTEX_FORMAT

#define GS_CAPS_MULTI_BIND           (GLEW_ARB_multi_bind != 0)
#define GS_CAPS_MULTI_BLEND          true // core
#define GS_CAPS_VERTEX_FORMAT        (GLEW_ARB_vertex_attrib_binding != 0)
#define GS_CAPS_TEXTURE_SRGB         true // core
#define GS_CAPS_TEXTURE_FLOAT        true // core
#define GS_CAPS_TEXTURE_DEPTH        true // core
#define GS_CAPS_TEXTURE_DEPTHSTENCIL true // core

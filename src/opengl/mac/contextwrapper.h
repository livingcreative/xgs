/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/mac/contextwrapper.h
        NSOpenGLContext plain C wrapper functions
*/

#pragma once

struct NSContextObject;
struct NSWidget;

#ifdef __cplusplus
extern "C" {
#endif

struct NSContextObject* cw_create(struct NSWidget *widget);
void cw_destroy(struct NSContextObject *context);
void cw_display(struct NSContextObject *context);
void cw_getsize(struct NSContextObject *context, int *width, int *height);

#ifdef __cplusplus
}
#endif

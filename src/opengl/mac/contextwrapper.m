/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    contextwrapper.m
        NSOpenGLContext plain C wrapper implementation
*/

#import "contextwrapper.h"
#import <Cocoa/Cocoa.h>


@interface NSOpenGLContextEx : NSOpenGLContext

@end

@implementation NSOpenGLContextEx

- (void)reshape
{
    [self update];
}

@end


struct NSContextObject* cw_create(struct NSWidget *widget)
{
    NSOpenGLContextEx *context;

    NSOpenGLPixelFormatAttribute attribs[] = {
        NSOpenGLPFAAccelerated,
        NSOpenGLPFAClosestPolicy,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
        NSOpenGLPFAColorSize, 32,
        NSOpenGLPFADepthSize, 32,
        0
    };

    NSOpenGLPixelFormat *format;
    format = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];

    context = [[NSOpenGLContextEx alloc] initWithFormat: format shareContext: NULL];
    [context makeCurrentContext];

    NSView *view = (__bridge NSView*)widget;
    [context setView: view];

    [[NSNotificationCenter defaultCenter] addObserver: context
                                             selector: @selector(reshape)
                                                 name: NSViewGlobalFrameDidChangeNotification
                                               object: view];

    return (__bridge struct NSContextObject*)context;
}

void cw_destroy(struct NSContextObject *context)
{
    NSOpenGLContext *ctx = (__bridge NSOpenGLContext*)context;

    [[NSNotificationCenter defaultCenter] removeObserver: ctx
                                                    name: NSViewGlobalFrameDidChangeNotification
                                                  object: ctx.view];

    [ctx clearDrawable];
    [NSOpenGLContext clearCurrentContext];
    [ctx release];
}

void cw_display(struct NSContextObject *context)
{
    NSOpenGLContext *ctx = (__bridge NSOpenGLContext*)context;
    [ctx flushBuffer];
}

void cw_getsize(struct NSContextObject *context, int *width, int *height)
{
    NSOpenGLContext *ctx = (__bridge NSOpenGLContext*)context;
    if (ctx.view) {
        *width = (int)ctx.view.bounds.size.width;
        *height = (int)ctx.view.bounds.size.height;
    } else {
        *width = 0;
        *height = 0;
    }
}

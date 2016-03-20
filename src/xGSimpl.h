#pragma once

#include "xGS/xGS.h"
#include <Windows.h>
#include "GL/glew.h"


class xGSimpl : public xGS
{
public:
    xGSimpl();
    ~xGSimpl();

    // xGS interface implementation
    bool CreateRenderer(const GSrendererdesc &desc) override;
    bool DestroyRenderer() override;

    bool GetRenderTargetSize(/* out */ GSsize &size) override;

    bool CreateObject(GSobjecttype type, const void *desc, void **object) override;

    bool Clear(bool clearcolor, bool cleardepth, bool clearstencil, const GScolor &color, float depth, unsigned int stencil) override;

    bool SetViewport(const GSviewport &viewport) override;
    bool SetState(xGSstate *state) override;

    bool DrawGeometry(xGSgeometry *geometry) override;

    bool Display() override;

private:
    void TrackGLError();

    void RenderTargetSize(/* out */ GSsize &size);

private:
    HWND  p_window;
    HDC   p_windowdc;
    HGLRC p_glcontext;
};

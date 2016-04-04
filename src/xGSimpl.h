#pragma once

#include "xGS/xGS.h"
#include "xGSrefcounted.h"
#include <unordered_set>
#include <Windows.h>
#include "GL/glew.h"


class xGSimpl : public xGSrefcountedImpl<xGS>
{
public:
    xGSimpl();
    ~xGSimpl();

    // xGS interface implementation
    bool CreateRenderer(const GSrendererdesc &desc) override;
    bool DestroyRenderer() override;

    bool GetRenderTargetSize(/* out */ GSsize &size) override;

    bool CreateSamplers(GSsamplerdesc *samplers, unsigned int count) override;
    bool CreateObject(GSobjecttype type, const void *desc, void **object) override;

    bool Clear(bool clearcolor, bool cleardepth, bool clearstencil, const GScolor &color, float depth, unsigned int stencil) override;

    bool SetViewport(const GSviewport &viewport) override;
    bool SetState(xGSstate *state) override;

    bool DrawGeometry(xGSgeometry *geometry) override;
    bool BuildMIPs(xGStexture *texture) override;

    bool Display() override;

public:
#define DECLARE_ADD_REMOVE_OBJECT(T) \
    void AddObject(T *object); \
    void RemoveObject(T *object);

    DECLARE_ADD_REMOVE_OBJECT(xGSgeometrybuffer)
    DECLARE_ADD_REMOVE_OBJECT(xGSgeometry)
    DECLARE_ADD_REMOVE_OBJECT(xGSdatabuffer)
    DECLARE_ADD_REMOVE_OBJECT(xGStexture)
    DECLARE_ADD_REMOVE_OBJECT(xGSstate)

#undef DECLARE_ADD_REMOVE_OBJECT

    GLuint samplerId(unsigned int index) const { return p_samplers[index]; }

private:
    void TrackGLError();

    void RenderTargetSize(/* out */ GSsize &size);

    template <typename T>
    void CheckObjectList(const T &list, const char *type);

private:
    typedef std::unordered_set<xGSgeometrybuffer*> GeometryBufferList;
    typedef std::unordered_set<xGSgeometry*> GeometryList;
    typedef std::unordered_set<xGSdatabuffer*> DataBufferList;
    typedef std::unordered_set<xGStexture*> GeometryTextureList;
    typedef std::unordered_set<xGSstate*> StateList;

    HWND   p_window;
    HDC    p_windowdc;
    HGLRC  p_glcontext;

    GLuint p_samplers[8];
    GLuint p_samplerscount;

    GeometryBufferList  p_geometrybufferlist;
    GeometryList        p_geometrylist;
    DataBufferList      p_databufferlist;
    GeometryTextureList p_texturelist;
    StateList           p_statelist;
};

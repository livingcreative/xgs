/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015

    https://github.com/livingcreative/xgs

    mac/xGScontextplatform.h
        xGScontextOSX class - Mac OS X OpenGL context implementation
*/

#pragma once

#include "../xGScontext.h"
#include "contextwrapper.h"


namespace xGS
{

    class xGScontextOSX : public xGScontext
    {
    public:
        xGScontextOSX();
        ~xGScontextOSX() override;

        GSerror Initialize() override;
        GSerror CreateRenderer(const GSrendererdescription &desc) override;
        GSbool DestroyRenderer() override;

        GSbool Display() override;

        GSsize RenderTargetSize() const override;

    private:
        void setBitsSupport(GSuint &bitflags, int bits)
        {
            switch (bits) {
                case 2: bitflags |= b2; break;
                case 4: bitflags |= b4; break;
                case 8: bitflags |= b8; break;
                case 16: bitflags |= b16; break;
                case 24: bitflags |= b24; break;
                case 32: bitflags |= b32; break;
                case 64: bitflags |= b64; break;
                case 128: bitflags |= b128; break;
            }
        }

        void CleanUp();

    private:
        NSContextObject *p_context;
    };


    class xGScontextCreatorPlatform : public xGScontextCreator
    {
    public:
        xGScontext* create() override
        {
            return new xGScontextOSX();
        }
    };

} // namespace xGS

/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    IxGSgeometrybufferimpl.h
        GeometryBuffer object public interface implementation
*/

#pragma once

#include "xGSgeometrybuffer.h"


namespace xGS
{

    class IxGSGeometryBufferImpl : public xGSObjectImpl<xGSGeometryBufferImpl, IxGSGeometryBufferImpl>
    {
    public:
        IxGSGeometryBufferImpl(xGSImpl *owner);
        ~IxGSGeometryBufferImpl() override;

        GSvalue xGSAPI GetValue(GSenum valuetype) override;

        GSptr   xGSAPI Lock(GSenum locktype, GSdword access, void *lockdata) override;
        GSbool  xGSAPI Unlock() override;

    public:
        void BeginImmediateDrawing();
        void EndImmediateDrawing();
    };

} // namespace xGS

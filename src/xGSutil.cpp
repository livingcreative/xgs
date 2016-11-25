/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    xGSutil.cpp
        Common utility functions and classes for implementation
*/

#include "xGSutil.h"


using namespace xGS;


GSvertexdecl::GSvertexdecl() :
    p_decl(),
    p_dynamic(false)
{}

GSvertexdecl::GSvertexdecl(const GSvertexcomponent *decl) :
    p_decl(),
    p_dynamic(false)
{
    initialize(decl);
}

GSuint GSvertexdecl::buffer_size(int count) const
{
    GSuint result = 0;
    for (auto const &i : p_decl) {
        result += vertex_component_size(i);
    }
    return result * count;
}

void GSvertexdecl::initialize(const GSvertexcomponent *decl)
{
    p_decl.clear();
    while (decl->type != GSVD_END) {
        if (decl->index == GS_DEFAULT) {
            p_dynamic = true;
        }

        p_decl.push_back(*decl);
        ++decl;
    }
}

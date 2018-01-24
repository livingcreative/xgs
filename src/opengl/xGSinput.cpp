/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2018

    https://github.com/livingcreative/xgs

    opengl/xGSinput.cpp
        Input object implementation class
*/

#include "xGSinput.h"
#include <algorithm>


using namespace std;
using namespace xGS;


xGSInputImpl::xGSInputImpl(xGSImpl *owner) :
    xGSObjectBase(owner),
    p_vertexbuffers(),
    p_indexbuffer(0),
    p_vertexarray(0)
{}

xGSInputImpl::~xGSInputImpl()
{}

GSbool xGSInputImpl::AllocateImpl(GSuint elementbuffers)
{
    if (elementbuffers) {
        // TODO: find_if MUST find something, think of any other means passing index buffer here
        auto ib = *find_if(
            p_buffers.begin(), p_buffers.end(),
            [](auto buffer) { return buffer->indexbuffer() != 0; }
        );
        p_indexbuffer = ib->indexbuffer();
    }

    if (p_owner->caps().vertex_format) {
        // store only variable buffers
        GSuint slot = 0;
        for (auto b : p_buffers) {
            if (b) {
                b->AddRef();
                p_vertexbuffers.push_back(b->vertexbuffer());
                p_strides.push_back(b->vertexDecl().buffer_size());
                p_slots.push_back(slot++);
            }
        }
    } else {
        glGenVertexArrays(1, &p_vertexarray);
        glBindVertexArray(p_vertexarray);

        // for own VAO path all buffers needed, including static state
        // buffers
        for (GSuint n = 0; n < p_state->inputCount(); ++n) {
            const xGSStateImpl::InputSlot &slot = p_state->input(n);

            xGSGeometryBufferImpl *buffer;
            if (slot.buffer) {
                buffer = slot.buffer;
            } else {
                buffer = p_buffers[n];
                buffer->AddRef();
            }
            glBindBuffer(GL_ARRAY_BUFFER, buffer->vertexbuffer());
            p_state->setarrays(slot.decl, slot.divisor, nullptr);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_indexbuffer);

#ifdef _DEBUG
        glBindVertexArray(0);
#endif
    }

    return p_owner->error(GS_OK);
}

void xGSInputImpl::apply(const GScaps &caps)
{
#ifdef GS_CONFIG_SEPARATE_VERTEX_FORMAT
    if (p_vertexarray) {
        // VAO available, but no separate format
        // bind whole VAO
        glBindVertexArray(p_vertexarray);
    } else {
        // separate format available, vertex pointers
        // assigned with VAO of state object
        // just buffers need binding

        // TODO: glBindVertexBuffers

        for (size_t b = 0; b < p_vertexbuffers.size(); ++b) {
            glBindVertexBuffer(p_slots[b], p_vertexbuffers[b], 0, p_strides[b]);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_indexbuffer);
    }
#else
    glBindVertexArray(p_vertexarray);
#endif
}

void xGSInputImpl::ReleaseRendererResources()
{
    if (p_vertexarray) {
        glDeleteVertexArrays(1, &p_vertexarray);
    }

    if (p_state) {
        p_state->Release();
    }

    for (auto b : p_buffers) {
        b->Release();
    }
}

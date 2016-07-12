/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    opengl/xGSinput.cpp
        Input object implementation class
*/

#include "xGSinput.h"
#include "xGSstate.h"
#include "xGSgeometry.h"
#include "xGSgeometrybuffer.h"


using namespace xGS;


xGSInputImpl::xGSInputImpl(xGSImpl *owner) :
    xGSObjectImpl(owner),
    p_allocated(false),
    p_state(nullptr),
    p_primarybuffer(nullptr),
    p_buffers(),
    p_vertexbuffers(),
    p_indexbuffer(0),
    p_vertexarray(0)
{
    p_owner->debug(DebugMessageLevel::Information, "Input object created\n");
}

xGSInputImpl::~xGSInputImpl()
{
    ReleaseRendererResources();
    p_owner->debug(DebugMessageLevel::Information, "Input object destroyed\n");
}

GSbool xGSInputImpl::allocate(const GSinputdescription &desc)
{
    if (!desc.bindings) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    xGSStateImpl *state = static_cast<xGSStateImpl*>(desc.state);

    if (!state || !state->allocated()) {
        return p_owner->error(GSE_INVALIDOBJECT);
    }

    GSuint elementbuffers = 0;
    p_buffers.resize(state->inputCount());
    for (auto &b : p_buffers) {
        b = nullptr;
    }

    const GSinputbinding *binding = desc.bindings;
    while (binding->slot != GSIS_END) {
        if (GSuint(binding->slot - GSIS_0) >= state->inputCount()) {
            // invalid slot index
            return p_owner->error(GSE_INVALIDVALUE);
        }

        if (state->input(binding->slot - GSIS_0).buffer) {
            // slot already bound to static buffer
            return p_owner->error(GSE_INVALIDVALUE);
        }

        xGSGeometryBufferImpl *buffer = static_cast<xGSGeometryBufferImpl*>(binding->buffer);
        if (!buffer->allocated()) {
            return p_owner->error(GSE_INVALIDOBJECT);
        }

        // TODO: check layout match

        if (GLuint ib = buffer->getIndexBufferID()) {
            p_indexbuffer = ib;
            ++elementbuffers;
        }

        p_buffers[binding->slot - GSIS_0] = buffer;
        ++binding;
    }

    if (p_buffers.size() != state->inputAvailable() || elementbuffers > 1) {
        // TODO: reconsider error code
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    p_state = state;
    p_state->AddRef();

    p_primarybuffer = p_buffers[p_state->inputPrimarySlot()];

    if (p_owner->caps().vertex_format) {
        // store only variable buffers
        GSuint slot = 0;
        for (auto b : p_buffers) {
            if (b) {
                b->AddRef();
                p_vertexbuffers.push_back(b->getVertexBufferID());
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
            glBindBuffer(GL_ARRAY_BUFFER, buffer->getVertexBufferID());
            p_state->setarrays(slot.decl, slot.divisor, nullptr);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_indexbuffer);

#ifdef _DEBUG
        glBindVertexArray(0);
#endif
    }

    p_allocated = true;

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

/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2017

    https://github.com/livingcreative/xgs

    dx12/xGSdatabuffer.cpp
        DataBuffer object implementation class
*/

#include "xGSdatabuffer.h"
#include "xGSstate.h"


using namespace xGS;


xGSDataBufferImpl::xGSDataBufferImpl(xGSImpl *owner) :
    xGSObjectImpl(owner),
    p_buffer(nullptr),
    p_size(0),
    p_locktype(GS_NONE)
{
    p_owner->debug(DebugMessageLevel::Information, "DataBuffer object created\n");
}

xGSDataBufferImpl::~xGSDataBufferImpl()
{
    ReleaseRendererResources();
    p_owner->debug(DebugMessageLevel::Information, "DataBuffer object destroyed\n");
}

GSvalue xGSDataBufferImpl::GetValue(GSenum valuetype)
{
    p_owner->error(GSE_UNIMPLEMENTED);
    return 0;
}

GSbool xGSDataBufferImpl::allocate(const GSdatabufferdescription &desc)
{
    switch (desc.type) {
        case GSDT_UNIFORM: break;
        default:
            return p_owner->error(GSE_INVALIDENUM);
    }

    p_size = 0;

    if (!desc.blocks) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    GSuint alignment = 0;

    const GSuniformblock *block = desc.blocks;
    while (block->type != GSB_END) {
        UniformBlock uniformblock = {
            p_size,                    // offset
            0,                         // size
            0,                         // actual size
            block->count,              // count
            GSuint(p_uniforms.size()), // firstuniform
            GSuint(p_uniforms.size())  // onepastlastuniform
        };

        const GSuniform *uniform = block->uniforms;
        GSuint uniformoffset = 0;
        while (uniform->type != GSU_END) {
            Uniform u = {
                uniform->type, // type
                uniformoffset, // offset
                0,             // size
                0,             // stride
                0,             // totalsize
                uniform->count // count
            };

            // TODO: check stride values
            switch (uniform->type) {
                case GSU_SCALAR: u.size = 4; u.stride = 16; break;
                case GSU_VEC2:   u.size = 8; u.stride = 16; break;
                case GSU_VEC3:   u.size = 16; u.stride = 16; break;
                case GSU_VEC4:   u.size = 16; u.stride = 16; break;
                case GSU_MAT2:   u.size = 32; u.stride = 32; break;
                case GSU_MAT3:   u.size = 48; u.stride = 48; break;
                case GSU_MAT4:   u.size = 64; u.stride = 64; break;
                default:
                    return p_owner->error(GSE_INVALIDENUM);
            }

            // TODO: adjust array paddings
            u.totalsize = u.stride * uniform->count;
            uniformoffset += u.totalsize;

            uniformblock.size += u.totalsize;
            ++uniformblock.onepastlastuniform;

            p_uniforms.push_back(u);

            ++uniform;
        }

        uniformblock.actualsize = uniformblock.size;
        uniformblock.size = (uniformblock.size + alignment) & ~alignment;
        GSuint blocksize = uniformblock.size * block->count;

        p_blocks.push_back(uniformblock);

        p_size += blocksize;

        ++block;
    }

    if (p_size == 0) {
        return GS_FALSE;
    }

    D3D12_HEAP_PROPERTIES heapprops = {
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
        D3D12_MEMORY_POOL_UNKNOWN,
        1, 1
    };

    D3D12_RESOURCE_DESC lockbufferdesc = {
        D3D12_RESOURCE_DIMENSION_BUFFER,
        0, p_size, 1, 1, 1,
        DXGI_FORMAT_UNKNOWN
    };
    lockbufferdesc.SampleDesc.Count = 1;
    lockbufferdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    p_owner->device()->CreateCommittedResource(
        &heapprops, D3D12_HEAP_FLAG_NONE, &lockbufferdesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr, IID_PPV_ARGS(&p_buffer)
    );

    return p_owner->error(GS_OK);
}

GSbool xGSDataBufferImpl::Update(GSuint offset, GSuint size, const GSptr data)
{
    if (p_size == 0) {
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    Update(data, offset, size);

    return p_owner->error(GS_OK);
}

GSbool xGSDataBufferImpl::UpdateBlock(GSuint block, GSuint index, const GSptr data)
{
    if (p_size == 0) {
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    if (block >= p_blocks.size()) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    const UniformBlock &ub = p_blocks[block];

    if (index >= ub.count) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    Update(data, ub.offset, ub.size);

    return p_owner->error(GS_OK);
}

GSbool xGSDataBufferImpl::UpdateValue(GSuint block, GSuint index, GSuint uniform, GSuint uniformindex, GSuint count, const GSptr data)
{
    if (p_size == 0) {
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    if (block >= p_blocks.size()) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    const UniformBlock &ub = p_blocks[block];

    if (index >= ub.count) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    if ((uniform + ub.firstuniform) >= ub.onepastlastuniform) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    const Uniform &u = p_uniforms[ub.firstuniform + uniform];

    if (uniformindex >= u.count) {
        return p_owner->error(GSE_INVALIDVALUE);
    }

    size_t offset = ub.offset + ub.size * index + u.offset + u.stride * uniformindex;

    Update(data, offset, u.size);

    return p_owner->error(GS_OK);
}

GSptr xGSDataBufferImpl::Lock(GSdword access, void *lockdata)
{
    if (p_locktype) {
        p_owner->error(GSE_INVALIDOPERATION);
        return nullptr;
    }

    p_locktype = GS_LOCKED;
    p_owner->error(GS_OK);

    // TODO: lock DX12 data buffer
    void *mem = nullptr;
    D3D12_RANGE range = { 0, p_size };
    p_buffer->Map(0, &range, &mem);

    return mem;
}

GSbool xGSDataBufferImpl::Unlock()
{
    if (p_locktype == GS_NONE) {
        return p_owner->error(GSE_INVALIDOPERATION);
    }

    p_locktype = GS_NONE;

    p_buffer->Unmap(0, nullptr);

    return p_owner->error(GS_OK);
}

void xGSDataBufferImpl::ReleaseRendererResources()
{
    ::Release(p_buffer);
}

void xGSDataBufferImpl::Update(void *source, size_t offset, size_t size)
{
    D3D12_RANGE range = { offset, offset + size };
    void *mem = nullptr;
    p_buffer->Map(0, &range, &mem);
    memcpy(mem, source, size);
    p_buffer->Unmap(0, nullptr);
}

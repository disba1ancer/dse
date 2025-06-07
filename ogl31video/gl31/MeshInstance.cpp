/*
 * MeshInstance.cpp
 *
 *  Created on: 29 февр. 2020 г.
 *      Author: disba1ancer
 */

#include <algorithm>
#include <vector>
#include <glbinding/gl31/gl.h>
#include "MeshInstance.h"
#include "binds.h"

using namespace gl31;

namespace {

enum {
    Ready,
    Pending,
    UploadReady
};

}

namespace dse::ogl31rbe::gl31 {

MeshInstance::MeshInstance(core::IMesh2 *mesh) :
    mesh(mesh),
    vao(0),
    vbo(0),
    ibo(0),
    readyStatus(0)
{
    if (!mesh) {
        throw std::runtime_error("Mesh can not be nullptr");
    }
}

auto MeshInstance::GetMesh() const -> core::IMesh2*
{
    return mesh;
}

bool MeshInstance::IsReady()
{
    switch (readyStatus.load(std::memory_order_acquire)) {
        case Ready: {
            if (vao && valid) {
                return true;
            }
            BeginLoad();
            if (readyStatus.load(std::memory_order_acquire) == Pending) {
                return false;
            }
        } [[fallthrough]];
        case UploadReady: {
            UploadBuffers();
            return readyStatus.load(std::memory_order_acquire) == Ready;
        } break;
    }
    return false;
}

auto MeshInstance::GetVAO() -> glwrp::VAO&
{
    return vao;
}

auto MeshInstance::GetVBO() -> glwrp::VertexBuffer&
{
    return vbo;
}

auto MeshInstance::GetIBO() -> glwrp::ElementBuffer&
{
    return ibo;
}

auto MeshInstance::GetSubmeshCount() -> std::size_t
{
    return submeshRanges.size();
}

auto MeshInstance::GetSubmeshRange(size_t n) -> core::IMesh2::SubmeshRange
{
    return submeshRanges[n];
}

void MeshInstance::Invalidate()
{
    valid = false;
}

void MeshInstance::BeginLoad()
{
    readyStatus.store(Pending, std::memory_order_relaxed);
    util::function_ptr f{*this, util::fn_tag<&MeshInstance::LoadRanges>};
    auto result = mesh->LoadMeshParameters(&meshParameters, f);
    if (result != core::status::Code::PendingOperation) {
        f(result);
    }
}

void MeshInstance::LoadRanges(core::Status status)
{
    vertexData.resize(meshParameters.verticesCount);
    elementData.resize(meshParameters.elementsCount);
    submeshRanges.resize(meshParameters.submeshCount);
    util::function_ptr f{*this, util::fn_tag<&MeshInstance::LoadVertices>};
    status = mesh->LoadSubmeshRanges(submeshRanges.data(), f);
    if (status != core::status::Code::PendingOperation) {
        f(status);
    }
}

void MeshInstance::LoadVertices(core::Status status)
{
    util::function_ptr f{*this, util::fn_tag<&MeshInstance::LoadElements>};
    status = mesh->LoadVertices(vertexData.data(), f);
    if (status != core::status::Code::PendingOperation) {
        f(status);
    }
}

void MeshInstance::LoadElements(core::Status status)
{
    util::function_ptr f{*this, util::fn_tag<&MeshInstance::BuffersReady>};
    status = mesh->LoadElements(elementData.data(), f);
    if (status != core::status::Code::PendingOperation) {
        f(status);
    }
}

void MeshInstance::BuffersReady(core::Status)
{
    readyStatus.store(UploadReady, std::memory_order_release);
}

void MeshInstance::UploadBuffers()
{
    vao = {};
    vbo = {};
    ibo = {};
    glBufferData(vbo.target, vertexData.size() * sizeof(core::IMesh2::Vertex), vertexData.data(), GL_STATIC_DRAW);
    glBufferData(ibo.target, elementData.size() * sizeof(std::uint32_t), elementData.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(InputParams::Position);
    glEnableVertexAttribArray(InputParams::Normal);
    glEnableVertexAttribArray(InputParams::Tangent);
    glEnableVertexAttribArray(InputParams::UV);
    glEnableVertexAttribArray(InputParams::BTangSign);
    glVertexAttribPointer(InputParams::Position, 3, GL_FLOAT, GL_FALSE, sizeof(core::IMesh2::Vertex), reinterpret_cast<void*>(offsetof(core::IMesh2::Vertex, pos)));
    glVertexAttribPointer(InputParams::Normal, 3, GL_FLOAT, GL_FALSE, sizeof(core::IMesh2::Vertex), reinterpret_cast<void*>(offsetof(core::IMesh2::Vertex, norm)));
    glVertexAttribPointer(InputParams::Tangent, 3, GL_FLOAT, GL_FALSE, sizeof(core::IMesh2::Vertex), reinterpret_cast<void*>(offsetof(core::IMesh2::Vertex, tang)));
    glVertexAttribPointer(InputParams::UV, 2, GL_FLOAT, GL_FALSE, sizeof(core::IMesh2::Vertex), reinterpret_cast<void*>(offsetof(core::IMesh2::Vertex, uv)));
    glVertexAttribPointer(InputParams::BTangSign, 1, GL_FLOAT, GL_FALSE, sizeof(core::IMesh2::Vertex), reinterpret_cast<void*>(offsetof(core::IMesh2::Vertex, bTangSign)));
    vertexData.clear();
    elementData.clear();
    valid = true;
    readyStatus.store(Ready, std::memory_order_release);
}

void MeshInstance::Deleter::operator()(MeshInstance* inst) const
{
    inst->Release();
}

} /* namespace dse::ogl31rbe::gl31 */

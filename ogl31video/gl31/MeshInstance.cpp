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

MeshInstance::MeshInstance(core::IMesh* mesh) :
	mesh(mesh),
	lastVersion(0),
	vao(0),
	vbo(0),
	ibo(0),
	readyStatus(0)
{
	if (!mesh) {
		throw std::runtime_error("Mesh can not be nullptr");
	}
}

auto MeshInstance::GetMesh() const -> core::IMesh*
{
	return mesh;
}

bool MeshInstance::IsReady()
{
	switch (readyStatus.load(std::memory_order_acquire)) {
		case Ready: {
			if (vao && lastVersion == mesh->GetVersion()) {
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

auto MeshInstance::GetSubmeshRange(size_t n) -> core::IMesh::submesh_range
{
	return submeshRanges[n];
}

void MeshInstance::BeginLoad()
{
	readyStatus.store(Pending, std::memory_order_relaxed);
	mesh->LoadMeshParameters(
		&meshParameters,
		util::StaticMemFn<&MeshInstance::LoadRanges>(*this)
	);
}

void MeshInstance::LoadRanges()
{
	vertexData.resize(meshParameters.verticesCount);
	elementData.resize(meshParameters.elementsCount);
	submeshRanges.resize(meshParameters.submeshCount);
	mesh->LoadSubmeshRanges(
		submeshRanges.data(),
		util::StaticMemFn<&MeshInstance::LoadVertices>(*this)
	);
}

void MeshInstance::LoadVertices()
{
	mesh->LoadVertices(
		vertexData.data(),
		util::StaticMemFn<&MeshInstance::LoadElements>(*this)
	);
}

void MeshInstance::LoadElements()
{
	mesh->LoadElements(
		elementData.data(),
		util::StaticMemFn<&MeshInstance::BuffersReady>(*this)
	);
}

void MeshInstance::BuffersReady()
{
	readyStatus.store(UploadReady, std::memory_order_release);
}

void MeshInstance::UploadBuffers()
{
	vao = {};
	vbo = {};
	ibo = {};
	glBufferData(vbo.target, vertexData.size() * sizeof(core::IMesh::vertex), vertexData.data(), GL_STATIC_DRAW);
	glBufferData(ibo.target, elementData.size() * sizeof(std::uint32_t), elementData.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(InputParams::Position);
	glEnableVertexAttribArray(InputParams::Normal);
	glEnableVertexAttribArray(InputParams::Tangent);
	glEnableVertexAttribArray(InputParams::UV);
	glVertexAttribPointer(InputParams::Position, 3, GL_FLOAT, GL_FALSE, sizeof(core::IMesh::vertex), reinterpret_cast<void*>(offsetof(core::IMesh::vertex, pos)));
	glVertexAttribPointer(InputParams::Normal, 3, GL_FLOAT, GL_FALSE, sizeof(core::IMesh::vertex), reinterpret_cast<void*>(offsetof(core::IMesh::vertex, norm)));
	glVertexAttribPointer(InputParams::Tangent, 3, GL_FLOAT, GL_FALSE, sizeof(core::IMesh::vertex), reinterpret_cast<void*>(offsetof(core::IMesh::vertex, tang)));
	glVertexAttribPointer(InputParams::UV, 2, GL_FLOAT, GL_FALSE, sizeof(core::IMesh::vertex), reinterpret_cast<void*>(offsetof(core::IMesh::vertex, uv)));
	vertexData.clear();
	elementData.clear();
	lastVersion = mesh->GetVersion();
	readyStatus.store(Ready, std::memory_order_release);
}

void MeshInstance::Deleter::operator()(MeshInstance* inst) const
{
	inst->Release();
}

} /* namespace dse::ogl31rbe::gl31 */

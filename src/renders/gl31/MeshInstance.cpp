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

namespace dse::renders::gl31 {

MeshInstance::MeshInstance(scn::IMesh* mesh) : mesh(mesh), lastVersion(0), vao(0), vbo(0), ibo(0), refcount(0) {
	if (!mesh) {
		std::runtime_error("Mesh can not be nullptr");
	}
}

auto MeshInstance::getMesh() const -> scn::IMesh* {
	return mesh;
}

bool MeshInstance::isReady() {
	if (!vao || lastVersion != mesh->getVersion()) {
		auto [vertCount, elemCount, submCount] = mesh->getMeshParameters();
		std::vector<scn::IMesh::vertex> vertexData(vertCount);
		std::vector<std::uint32_t> elementData(elemCount);
		submCount = std::max(submCount, std::uint32_t(1));
		submeshRanges.resize(submCount);
		if (submCount == 1) {
			submeshRanges[0] = { 0, elemCount };
		} else {
			for (unsigned i = 0; i < submCount; ++i) {
				submeshRanges[i] = mesh->getSubmeshRange(i);
			}
		}
		vao = glwrp::VAO();
		vbo = glwrp::VertexBuffer();
		ibo = glwrp::ElementBuffer();
		mesh->loadVerticesRange(vertexData.data(), 0, vertCount);
		mesh->loadElementsRange(elementData.data(), 0, elemCount);
		glBufferData(GL_ARRAY_BUFFER, vertCount * sizeof(scn::IMesh::vertex), vertexData.data(), GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, elemCount * sizeof(std::uint32_t), elementData.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(InputParams::Position);
		glEnableVertexAttribArray(InputParams::Normal);
		glEnableVertexAttribArray(InputParams::Tangent);
		glEnableVertexAttribArray(InputParams::UV);
		glVertexAttribPointer(InputParams::Position, 3, GL_FLOAT, GL_FALSE, sizeof(scn::IMesh::vertex), reinterpret_cast<void*>(offsetof(scn::IMesh::vertex, pos)));
		glVertexAttribPointer(InputParams::Normal, 3, GL_FLOAT, GL_FALSE, sizeof(scn::IMesh::vertex), reinterpret_cast<void*>(offsetof(scn::IMesh::vertex, norm)));
		glVertexAttribPointer(InputParams::Tangent, 3, GL_FLOAT, GL_FALSE, sizeof(scn::IMesh::vertex), reinterpret_cast<void*>(offsetof(scn::IMesh::vertex, tang)));
		glVertexAttribPointer(InputParams::UV, 2, GL_FLOAT, GL_FALSE, sizeof(scn::IMesh::vertex), reinterpret_cast<void*>(offsetof(scn::IMesh::vertex, uv)));
		lastVersion = mesh->getVersion();
	}
	return true;
}

auto MeshInstance::getVAO() -> glwrp::VAO& {
	return vao;
}

auto MeshInstance::getVBO() -> glwrp::VertexBuffer& {
	return vbo;
}

auto MeshInstance::getIBO() -> glwrp::ElementBuffer& {
	return ibo;
}

auto MeshInstance::getSubmeshCount() -> std::size_t {
	return submeshRanges.size();
}

auto MeshInstance::getSubmeshRange(size_t n) -> scn::IMesh::submesh_range {
	return submeshRanges[n];
}

void MeshInstance::AddRef() {
	++refcount;
}

void MeshInstance::Release() {
	if (refcount) {
		--refcount;
	}
}

bool MeshInstance::isNoRefs() {
	return refcount == 0 ;
}

void MeshInstance::Deleter::operator()(MeshInstance* inst) const {
	inst->Release();
}

} /* namespace dse::renders::gl31 */

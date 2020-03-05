/*
 * MeshInstance.cpp
 *
 *  Created on: 29 февр. 2020 г.
 *      Author: disba1ancer
 */

#include "MeshInstance.h"
#include <memory>
#include <algorithm>
#include "binds.h"

namespace dse {
namespace subsys {
namespace gl31_impl {

MeshInstance::MeshInstance(scn::IMesh* mesh) : mesh(mesh), lastVersion(0), vao(true), vbo(true), ibo(true) {
	if (!mesh) {
		std::runtime_error("Mesh can not be nullptr");
	}
}

bool MeshInstance::Comparator::operator()(const MeshInstance& lm, const MeshInstance& rm) const {
	return std::less<scn::IMesh*>::operator ()(lm.getMesh(), rm.getMesh());
}

scn::IMesh* MeshInstance::getMesh() const {
	return mesh;
}

bool MeshInstance::isReady() {
	// задействовать общий vbo и ibo на субмеши с требованием размещать элементы ibo одного субмеша друг за другом без разрывов
	if (!vao || lastVersion != mesh->getVersion()) {
		auto [vertCount, elemCount, submCount] = mesh->getMeshParameters();
		std::unique_ptr<scn::IMesh::vertex[]> vertexData(new scn::IMesh::vertex[vertCount]);
		std::unique_ptr<std::uint32_t[]> elementData(new std::uint32_t[elemCount]);
		submCount = std::max(submCount, std::uint32_t(1));
		submeshRanges.resize(submCount);
		if (submCount == 1) {
			submeshRanges[0] = { 0, elemCount };
		} else {
			for (unsigned i = 0; i < submCount; ++i) {
				submeshRanges[i] = mesh->getSubmeshRange(i);
			}
		}
		vao = gl::VAO();
		vbo = gl::VertexBuffer();
		ibo = gl::ElementBuffer();
		mesh->loadVerticesRange(vertexData.get(), 0, vertCount);
		mesh->loadElementsRange(elementData.get(), 0, elemCount);
		glBufferData(GL_ARRAY_BUFFER, vertCount * sizeof(scn::IMesh::vertex), vertexData.get(), GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, elemCount * sizeof(std::uint32_t), elementData.get(), GL_STATIC_DRAW);
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

gl::VAO& MeshInstance::getVAO() {
	return vao;
}

gl::VertexBuffer& MeshInstance::getVBO() {
	return vbo;
}

gl::ElementBuffer& MeshInstance::getIBO() {
	return ibo;
}

std::size_t MeshInstance::getSubmeshCount() {
	return submeshRanges.size();
}

scn::IMesh::submesh_range MeshInstance::getSubmeshRange(size_t n) {
	return submeshRanges[n];
}

} /* namespace gl31_impl */
} /* namespace subsys */
} /* namespace dse */

/*
 * MeshInstance.h
 *
 *  Created on: 29 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL31_IMPL_MESHINSTANCE_H_
#define SUBSYS_GL31_IMPL_MESHINSTANCE_H_

#include "scn/IMesh.h"
#include <functional>
#include "renders/glwrp/VAO.h"
#include "renders/glwrp/Buffer.h"
#include <vector>

namespace dse::renders::gl31 {

class MeshInstance {
	scn::IMesh* mesh;
	unsigned lastVersion;
	std::vector<scn::IMesh::submesh_range> submeshRanges;
	glwrp::VAO vao;
	glwrp::VertexBuffer vbo;
	glwrp::ElementBuffer ibo;
	std::size_t refcount;
public:
	MeshInstance(scn::IMesh* mesh);
	auto getMesh() const -> scn::IMesh*;
	bool isReady();
	auto getVAO() -> glwrp::VAO&;
	auto getVBO() -> glwrp::VertexBuffer&;
	auto getIBO() -> glwrp::ElementBuffer&;
	auto getSubmeshCount() -> std::size_t;
	auto getSubmeshRange(size_t n) -> scn::IMesh::submesh_range;
	void AddRef();
	void Release();
	bool isNoRefs();
	struct Deleter {
		void operator()(MeshInstance* inst) const;
	};
};

} /* namespace dse::renders::gl31 */

#endif /* SUBSYS_GL31_IMPL_MESHINSTANCE_H_ */

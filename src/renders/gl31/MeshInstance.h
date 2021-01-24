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
public:
	MeshInstance(scn::IMesh* mesh);
	~MeshInstance() = default;
	MeshInstance(const MeshInstance &other) = default;
	MeshInstance(MeshInstance &&other) = default;
	MeshInstance& operator=(const MeshInstance &other) = default;
	MeshInstance& operator=(MeshInstance &&other) = default;
	scn::IMesh* getMesh() const;
	bool isReady();
	glwrp::VAO& getVAO();
	glwrp::VertexBuffer& getVBO();
	glwrp::ElementBuffer& getIBO();
	std::size_t getSubmeshCount();
	scn::IMesh::submesh_range getSubmeshRange(size_t n);
	class Comparator : std::less<scn::IMesh*> {
	public:
		bool operator()(const MeshInstance& lm, const MeshInstance& rm) const;
	};
};

} /* namespace dse::renders::gl31 */

#endif /* SUBSYS_GL31_IMPL_MESHINSTANCE_H_ */

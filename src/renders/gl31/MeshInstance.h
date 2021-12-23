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
#include "RefCounted.h"
#include <atomic>

namespace dse::renders::gl31 {

class MeshInstance : public RefCounted {
	scn::IMesh* mesh;
	unsigned lastVersion;
	std::vector<scn::IMesh::submesh_range> submeshRanges;
	glwrp::VAO vao;
	glwrp::VertexBuffer vbo;
	glwrp::ElementBuffer ibo;
	std::atomic_int readyStatus;
	scn::IMesh::mesh_parameters meshParameters;
	std::vector<scn::IMesh::vertex> vertexData;
	std::vector<std::uint32_t> elementData;
public:
	MeshInstance(scn::IMesh* mesh);
	auto GetMesh() const -> scn::IMesh*;
	bool IsReady();
	auto GetVAO() -> glwrp::VAO&;
	auto GetVBO() -> glwrp::VertexBuffer&;
	auto GetIBO() -> glwrp::ElementBuffer&;
	auto GetSubmeshCount() -> std::size_t;
	auto GetSubmeshRange(size_t n) -> scn::IMesh::submesh_range;
	struct Deleter {
		void operator()(MeshInstance* inst) const;
	};
private:
	void BeginLoad();
	void LoadRanges();
	void LoadVertices();
	void LoadElements();
	void BuffersReady();
	void UploadBuffers();
};

} /* namespace dse::renders::gl31 */

#endif /* SUBSYS_GL31_IMPL_MESHINSTANCE_H_ */

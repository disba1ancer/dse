/*
 * MeshInstance.h
 *
 *  Created on: 29 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL31_IMPL_MESHINSTANCE_H_
#define SUBSYS_GL31_IMPL_MESHINSTANCE_H_

#include <dse/core/scene2.h>
#include <functional>
#include "../glwrp/VAO.h"
#include "../glwrp/Buffer.h"
#include <vector>
#include "RefCounted.h"
#include <atomic>

namespace dse::ogl31rbe::gl31 {

class MeshInstance : public RefCounted {
    core::IMesh2* mesh;
    bool valid = false;
    std::vector<core::IMesh2::SubmeshRange> submeshRanges;
	glwrp::VAO vao;
	glwrp::VertexBuffer vbo;
	glwrp::ElementBuffer ibo;
	std::atomic_int readyStatus;
    core::IMesh2::MeshParameters meshParameters;
    std::vector<core::IMesh2::Vertex> vertexData;
	std::vector<std::uint32_t> elementData;
public:
    MeshInstance(core::IMesh2* mesh);
    auto GetMesh() const -> core::IMesh2*;
	bool IsReady();
	auto GetVAO() -> glwrp::VAO&;
	auto GetVBO() -> glwrp::VertexBuffer&;
	auto GetIBO() -> glwrp::ElementBuffer&;
	auto GetSubmeshCount() -> std::size_t;
    auto GetSubmeshRange(size_t n) -> core::IMesh2::SubmeshRange;
    void Invalidate();
	struct Deleter {
		void operator()(MeshInstance* inst) const;
	};
private:
	void BeginLoad();
    void LoadRanges(core::Status);
    void LoadVertices(core::Status);
    void LoadElements(core::Status);
    void BuffersReady(core::Status);
	void UploadBuffers();
};

} /* namespace dse::ogl31rbe::gl31 */

#endif /* SUBSYS_GL31_IMPL_MESHINSTANCE_H_ */

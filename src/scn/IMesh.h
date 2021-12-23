/*
 * IMesh.h
 *
 *  Created on: 9 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SCN_IMESH_H_
#define SCN_IMESH_H_

#include "math/vec.h"
#include <cstdint>
#include "util/functional.h"

namespace dse {
namespace scn {

class IMesh {
public:
	struct vertex {
		math::vec3 pos;
		math::vec3 norm;
		math::vec3 tang;
		math::vec2 uv;
		float reserve;
	};
	struct mesh_parameters {
		std::uint32_t verticesCount;
		std::uint32_t elementsCount;
		std::uint32_t submeshCount;
	};
	struct submesh_range {
		std::uint32_t start;
		std::uint32_t end;
	};
	virtual void LoadMeshParameters(mesh_parameters* parameters, util::FunctionPtr<void()> callback) = 0;
	virtual void LoadVertices(vertex* vertexBuffer, util::FunctionPtr<void()> callback) = 0;
	virtual void LoadElements(std::uint32_t* elementBuffer, util::FunctionPtr<void()> callback) = 0;
	virtual void LoadSubmeshRanges(submesh_range* ranges, util::FunctionPtr<void()> callback) = 0;
	virtual auto GetVersion() -> unsigned = 0;
protected:
	~IMesh() = default;
};

} /* namespace scn */
} /* namespace dse */

#endif /* SCN_IMESH_H_ */

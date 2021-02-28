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
	virtual mesh_parameters getMeshParameters() = 0;
	virtual void loadVerticesRange(vertex* vertexBuffer, std::uint32_t startVertex, std::uint32_t vertexCount) = 0;
	virtual void loadElementsRange(std::uint32_t* elementBuffer, std::uint32_t startElement, std::uint32_t elementCount) = 0;
	virtual submesh_range getSubmeshRange(std::uint32_t submeshIndex) = 0;
	virtual unsigned getVersion() = 0;
protected:
	~IMesh() = default;
};

} /* namespace scn */
} /* namespace dse */

#endif /* SCN_IMESH_H_ */

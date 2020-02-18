/*
 * IMesh.h
 *
 *  Created on: 9 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SCN_IMESH_H_
#define SCN_IMESH_H_

#include "../math/vec.h"
#include <cinttypes>

namespace dse {
namespace scn {

class Material;

class IMesh {
public:
	struct vertex {
		math::vec3 pos;
		math::vec3 norm;
		math::vec3 tang;
		math::vec2 uv;
		float reserve;
	};
	struct submesh_size {
		std::uint32_t verticesCount;
		std::uint32_t elementsCount;
	};
	virtual std::uint32_t getSubmeshCount() = 0;
	virtual submesh_size getSubmeshSize(std::uint32_t submeshIndex) = 0;
	virtual void fillSubmeshBuffers(std::uint32_t submeshIndex, vertex* vertexBuffer, std::uint32_t* elementBuffer) = 0;
protected:
	~IMesh() = default;
};

} /* namespace scn */
} /* namespace dse */

#endif /* SCN_IMESH_H_ */

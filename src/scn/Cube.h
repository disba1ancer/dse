/*
 * Cube.h
 *
 *  Created on: 16 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SCN_CUBE_H_
#define SCN_CUBE_H_

#include "IMesh.h"

namespace dse {
namespace scn {

class Cube: public IMesh {
public:
	Cube() = default;
	~Cube() = default;
	Cube(const Cube &other) = default;
	Cube(Cube &&other) = default;
	Cube& operator=(Cube &&other) = default;
	Cube& operator=(const Cube &other) = default;

	virtual void fillSubmeshBuffers(uint32_t submeshIndex, vertex *vertexBuffer, uint32_t *elementBuffer) override;
	virtual uint32_t getSubmeshCount() override;
	virtual dse::scn::IMesh::submesh_size getSubmeshSize(uint32_t submeshIndex) override;
};

} /* namespace scn */
} /* namespace dse */

#endif /* SCN_CUBE_H_ */

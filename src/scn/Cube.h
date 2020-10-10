/*
 * Cube.h
 *
 *  Created on: 16 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SCN_CUBE_H_
#define SCN_CUBE_H_

#include "IMeshStore.h"

namespace dse {
namespace scn {

class Cube final : public IMeshStore {
public:
	Cube() = default;
	~Cube() = default;
	Cube(const Cube &other) = default;
	Cube(Cube &&other) = default;
	Cube& operator=(Cube &&other) = default;
	Cube& operator=(const Cube &other) = default;

	virtual dse::scn::IMesh::mesh_parameters getMeshParameters() override;
	virtual unsigned getVersion() override;
	virtual void loadVerticesRange(dse::scn::IMesh::vertex *vertexBuffer,
			std::uint32_t startVertex, std::uint32_t vertexCount) override;
	virtual void loadElementsRange(std::uint32_t *elementBuffer, std::uint32_t startElement,
			std::uint32_t elementCount) override;
	virtual dse::scn::IMesh::submesh_range getSubmeshRange(
			std::uint32_t submeshIndex) override;
};

} /* namespace scn */
} /* namespace dse */

#endif /* SCN_CUBE_H_ */

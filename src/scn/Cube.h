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

class Cube final : public IMesh {
public:
	Cube() = default;
	~Cube() = default;
	Cube(const Cube &other) = default;
	Cube(Cube &&other) = default;
	Cube& operator=(Cube &&other) = default;
	Cube& operator=(const Cube &other) = default;

	// IMesh interface
public:
	virtual void LoadMeshParameters(mesh_parameters *parameters, util::FunctionPtr<void ()> callback) override;
	virtual void LoadSubmeshRanges(submesh_range *ranges, util::FunctionPtr<void ()> callback) override;
	virtual void LoadVertices(vertex* vertexBuffer, util::FunctionPtr<void ()> callback) override;
	virtual void LoadElements(uint32_t* elementBuffer, util::FunctionPtr<void ()> callback) override;
	virtual auto GetVersion() -> unsigned override;
};

} /* namespace scn */
} /* namespace dse */

#endif /* SCN_CUBE_H_ */

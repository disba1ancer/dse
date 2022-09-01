/*
 * Cube.h
 *
 *  Created on: 16 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_CUBE_H_
#define DSE_CORE_CUBE_H_

#include "IMesh.h"
#include "detail/impexp.h"

namespace dse::core {

class API_DSE_CORE Cube final : public IMesh {
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

} /* namespace dse::core */

#endif /* DSE_CORE_CUBE_H_ */

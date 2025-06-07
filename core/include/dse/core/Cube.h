/*
 * Cube.h
 *
 *  Created on: 16 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_CUBE_H_
#define DSE_CORE_CUBE_H_

#include "scene2.h"
#include "detail/impexp.h"

namespace dse::core {

class API_DSE_CORE Cube final : public IMesh2 {
public:
	Cube() = default;
	~Cube() = default;
	Cube(const Cube &other) = default;
	Cube(Cube &&other) = default;
	Cube& operator=(Cube &&other) = default;
	Cube& operator=(const Cube &other) = default;

    // IMesh2 interface
public:
    auto LoadMeshParameters(MeshParameters *parameters, util::function_ptr<void(Status)> callback) -> Status override;
    auto LoadSubmeshRanges(SubmeshRange *ranges, util::function_ptr<void(Status)> callback) -> Status override;
    auto LoadVertices(Vertex* vertexBuffer, util::function_ptr<void(Status)> callback) -> Status override;
    auto LoadElements(uint32_t* elementBuffer, util::function_ptr<void(Status)> callback) -> Status override;
    auto GetResourceManager() -> IResourceManager& override;
};

} /* namespace dse::core */

#endif /* DSE_CORE_CUBE_H_ */

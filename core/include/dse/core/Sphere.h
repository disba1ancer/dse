#ifndef DSE_CORE_SPHERE_H
#define DSE_CORE_SPHERE_H

#include "scene2.h"
#include <vector>

namespace dse::core {

class Sphere : public IMesh2
{
public:
    Sphere(int hseg, int vseg);

    // IMesh interface
public:
    auto LoadMeshParameters(MeshParameters *parameters, util::function_ptr<void (Status)> callback) -> Status override;
    auto LoadVertices(Vertex *vertexBuffer, util::function_ptr<void (Status)> callback) -> Status override;
    auto LoadElements(std::uint32_t *elementBuffer, util::function_ptr<void (Status)> callback) -> Status override;
    auto LoadSubmeshRanges(SubmeshRange *ranges, util::function_ptr<void (Status)> callback) -> Status override;
    auto GetResourceManager() -> IResourceManager& override;
private:
    std::vector<IMesh2::Vertex> vertices;
    std::vector<std::uint32_t> elements;
};

} // namespace dse::core

#endif // DSE_CORE_SPHERE_H

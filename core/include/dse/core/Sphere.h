#ifndef DSE_CORE_SPHERE_H
#define DSE_CORE_SPHERE_H

#include "IMesh.h"
#include <vector>

namespace dse::core {

class Sphere : public IMesh
{
public:
    Sphere(int hseg, int vseg);

    // IMesh interface
public:
    void LoadMeshParameters(mesh_parameters *parameters, util::FunctionPtr<void ()> callback) override;
    void LoadVertices(vertex *vertexBuffer, util::FunctionPtr<void ()> callback) override;
    void LoadElements(std::uint32_t *elementBuffer, util::FunctionPtr<void ()> callback) override;
    void LoadSubmeshRanges(submesh_range *ranges, util::FunctionPtr<void ()> callback) override;
    unsigned int GetVersion() override;
private:
    std::vector<IMesh::vertex> vertices;
    std::vector<std::uint32_t> elements;
};

} // namespace dse::core

#endif // DSE_CORE_SPHERE_H

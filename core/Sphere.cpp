#include "dse/math/mat.h"
#include <dse/core/Sphere.h>
#include <dse/math/constants.h>
#include <dse/math/vec.h>
#include <cmath>

using dse::math::PI;

namespace dse::core {

Sphere::Sphere(int hseg, int vseg) :
    vertices(vseg * (hseg + 1) + hseg - 1),
    elements(2 * hseg * vseg)
{
    math::vec3 baseVec = {0, 0, 1};
    math::vec3 baseTan = {-1, 0, 0};
    float bTangSign = -1.f;
    math::mat3 mrot = {
        1, 0, 0,
        0, std::cos(PI / hseg), -std::sin(PI / hseg),
        0, std::sin(PI / hseg), std::cos(PI / hseg)
    };
    float huvOff = 1.f / hseg;
    float huvBase = 0.f;
    vertices[0].pos = baseVec;
    vertices[0].norm = baseVec;
    vertices[0].tang = baseTan;
    vertices[0].uv = { huvOff / 2, huvBase };
    vertices[0].bTangSign = bTangSign;
    baseVec = mrot * baseVec;
    huvBase += huvOff;
    for (int i = 1; i < hseg; ++i, baseVec = mrot * baseVec, huvBase += huvOff) {
        vertices[i].pos = baseVec;
        vertices[i].norm = baseVec;
        vertices[i].tang = baseTan;
        vertices[i].uv = { 0, huvBase };
        vertices[i].bTangSign = bTangSign;
    }
    baseVec = {0, 0, -1};
    vertices[hseg].pos = baseVec;
    vertices[hseg].norm = baseVec;
    vertices[hseg].tang = baseTan;
    vertices[hseg].uv = { huvOff / 2, 1.f };
    vertices[hseg].bTangSign = bTangSign;
    float vuvOff = 1.f / vseg;
    mrot = {
        std::cos(2 * PI / vseg), std::sin(2 * PI / vseg), 0,
        -std::sin(2 * PI / vseg), std::cos(2 * PI / vseg), 0,
        0, 0, 1
    };
    auto end = vseg * (hseg + 1);
    for (int i = hseg + 1; i < end; ++i) {
        vertices[i].pos = mrot * vertices[i - hseg - 1].pos;
        vertices[i].norm = mrot * vertices[i - hseg - 1].norm;
        vertices[i].tang = mrot * vertices[i - hseg - 1].tang;
        vertices[i].uv = vertices[i - hseg - 1].uv + math::vec2{ vuvOff, 0.f };
        vertices[i].bTangSign = bTangSign;
    }
    for (int i = 0; i < hseg - 1; ++i) {
        vertices[end + i].pos = vertices[i + 1].pos;
        vertices[end + i].norm = vertices[i + 1].norm;
        vertices[end + i].tang = vertices[i + 1].tang;
        vertices[end + i].uv = vertices[i + 1].uv + math::vec2{ 1.f, 0.f };
        vertices[end + i].bTangSign = bTangSign;
    }
    int index = 0;
    for (int j = 0; j < vseg - 1; ++j) {
        elements[index++] = j * (hseg + 1);
        for (int i = 0; i < hseg - 1; ++i) {
            elements[index++] = j * (hseg + 1) + i + 1;
            elements[index++] = (j + 1) * (hseg + 1) + i + 1;
        }
        elements[index++] = j++ * (hseg + 1) + hseg;
        if (j == vseg - 1) {
            elements[index++] = j * (hseg + 1) + hseg;
            for (int i = hseg - 1; i > 0; --i) {
                elements[index++] = (j + 1) * (hseg + 1) + i - 1;
                elements[index++] = j * (hseg + 1) + i;
            }
            elements[index++] = j * (hseg + 1);
            return;
        }
        elements[index++] = j * (hseg + 1) + hseg;
        for (int i = hseg - 1; i > 0; --i) {
            elements[index++] = (j + 1) * (hseg + 1) + i;
            elements[index++] = j * (hseg + 1) + i;
        }
        elements[index++] = j * (hseg + 1);
    }
    int j = vseg - 1;
    elements[index++] = j * (hseg + 1);
    for (int i = 0; i < hseg - 1; ++i) {
        elements[index++] = j * (hseg + 1) + i + 1;
        elements[index++] = (j + 1) * (hseg + 1) + i;
    }
    elements[index++] = j * (hseg + 1) + hseg;
}

void Sphere::LoadMeshParameters(mesh_parameters *parameters, util::FunctionPtr<void ()> callback)
{
    parameters->submeshCount = 1;
    parameters->elementsCount = elements.size();
    parameters->verticesCount = vertices.size();
    callback();
}

void Sphere::LoadVertices(vertex *vertexBuffer, util::FunctionPtr<void ()> callback)
{
    std::copy(std::begin(vertices), std::end(vertices), vertexBuffer);
    callback();
}

void Sphere::LoadElements(uint32_t *elementBuffer, util::FunctionPtr<void ()> callback)
{
    std::copy(std::begin(elements), std::end(elements), elementBuffer);
    callback();
}

void Sphere::LoadSubmeshRanges(submesh_range *ranges, util::FunctionPtr<void ()> callback)
{
    ranges->drawType = Draw::Stripes;
    ranges->start = 0;
    ranges->end = elements.size();
    callback();
}

unsigned int Sphere::GetVersion()
{
    return 1;
}

}

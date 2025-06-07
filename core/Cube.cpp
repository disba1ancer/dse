/*
 * Cube.cpp
 *
 *  Created on: 16 февр. 2020 г.
 *      Author: disba1ancer
 */

#include <dse/core/Cube.h>
#include <iterator>
#include <array>

namespace dse::core {

namespace {
static const auto vertices = std::to_array<IMesh2::Vertex>({
        {{ 1, -1, -1}, { 1, 0, 0}, {0, 1, 0}, {0, 0}, 1},//+x
        {{ 1, -1,  1}, { 1, 0, 0}, {0, 1, 0}, {0, 1}, 1},//1
        {{ 1,  1, -1}, { 1, 0, 0}, {0, 1, 0}, {1, 0}, 1},//2
        {{ 1,  1,  1}, { 1, 0, 0}, {0, 1, 0}, {1, 1}, 1},//3

        {{-1, -1, -1}, {-1, 0, 0}, {0, 1, 0}, {0, 0}, 1},//-x
        {{-1, -1,  1}, {-1, 0, 0}, {0, 1, 0}, {0, 1}, 1},//5
        {{-1,  1, -1}, {-1, 0, 0}, {0, 1, 0}, {1, 0}, 1},//6
        {{-1,  1,  1}, {-1, 0, 0}, {0, 1, 0}, {1, 1}, 1},//7

        {{-1,  1, -1}, {0,  1, 0}, {1, 0, 0}, {0, 0}, 1},//+y
        {{ 1,  1, -1}, {0,  1, 0}, {1, 0, 0}, {0, 1}, 1},//9
        {{-1,  1,  1}, {0,  1, 0}, {1, 0, 0}, {1, 0}, 1},//10
        {{ 1,  1,  1}, {0,  1, 0}, {1, 0, 0}, {1, 1}, 1},//11

        {{-1, -1, -1}, {0, -1, 0}, {1, 0, 0}, {0, 0}, 1},//-y
        {{ 1, -1, -1}, {0, -1, 0}, {1, 0, 0}, {0, 1}, 1},//13
        {{-1, -1,  1}, {0, -1, 0}, {1, 0, 0}, {1, 0}, 1},//14
        {{ 1, -1,  1}, {0, -1, 0}, {1, 0, 0}, {1, 1}, 1},//15

        {{-1, -1,  1}, {0, 0,  1}, {1, 0, 0}, {0, 0}, 1},//+z
        {{-1,  1,  1}, {0, 0,  1}, {1, 0, 0}, {0, 1}, 1},//17
        {{ 1, -1,  1}, {0, 0,  1}, {1, 0, 0}, {1, 0}, 1},//18
        {{ 1,  1,  1}, {0, 0,  1}, {1, 0, 0}, {1, 1}, 1},//19

        {{-1, -1, -1}, {0, 0, -1}, {1, 0, 0}, {0, 0}, 1},//-z
        {{-1,  1, -1}, {0, 0, -1}, {1, 0, 0}, {0, 1}, 1},//21
        {{ 1, -1, -1}, {0, 0, -1}, {1, 0, 0}, {1, 0}, 1},//22
        {{ 1,  1, -1}, {0, 0, -1}, {1, 0, 0}, {1, 1}, 1} //23
});
static const auto elements = std::to_array<std::uint32_t>({
         2,  1,  0,  1,  2,  3,//+x
         4,  5,  6,  7,  6,  5,//-x
        10,  9,  8,  9, 10, 11,//+y
        12, 13, 14, 15, 14, 13,//-y
        18, 17, 16, 17, 18, 19,//+z
        20, 21, 22, 23, 22, 21,//-z
});

using DT = IMesh2::Draw;

static const auto subranges = std::to_array<IMesh2::SubmeshRange>({
        {0, 36, DT::Triangles},//+x
//        {6, 12},//-x
//        {12, 18},//+y
//        {18, 24},//-y
//        {24, 30},//+z
//        {30, 36},//-z
});
}

auto Cube::LoadMeshParameters(MeshParameters* parameters, util::function_ptr<void(Status)> callback) -> Status
{
    *parameters = { std::size(vertices), std::size(elements), std::size(subranges) };
    return Make(status::Code::Success);
}

auto Cube::LoadSubmeshRanges(SubmeshRange* ranges, util::function_ptr<void(Status)> callback) -> Status
{
    std::copy(std::begin(subranges), std::end(subranges), ranges);
    return Make(status::Code::Success);
}

auto Cube::LoadVertices(Vertex* vertexBuffer, util::function_ptr<void(Status)> callback) -> Status
{
    std::copy(std::begin(vertices), std::end(vertices), vertexBuffer);
    return Make(status::Code::Success);
}

auto Cube::LoadElements(uint32_t* elementBuffer, util::function_ptr<void(Status)> callback) -> Status
{
    std::copy(std::begin(elements), std::end(elements), elementBuffer);
    return Make(status::Code::Success);
}

auto Cube::GetResourceManager() -> IResourceManager&
{
    return StaticResourceManager::instance;
}

} /* namespace dse::core */

/*
 * Cube.cpp
 *
 *  Created on: 16 февр. 2020 г.
 *      Author: disba1ancer
 */

#include <dse/core/Cube.h>
#include <cstring>
#include <iterator>
#include <array>

namespace dse::core {

namespace {
static const auto vertices = std::to_array<IMesh::vertex>({
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

using DT = IMesh::Draw;

static const auto subranges = std::to_array<IMesh::submesh_range>({
        {0, 36, DT::Triangles},//+x
//        {6, 12},//-x
//        {12, 18},//+y
//        {18, 24},//-y
//        {24, 30},//+z
//        {30, 36},//-z
});
}

void Cube::LoadMeshParameters(mesh_parameters* parameters, util::function_ptr<void ()> callback)
{
    *parameters = { std::size(vertices), std::size(elements), std::size(subranges) };
    callback();
}

void Cube::LoadSubmeshRanges(submesh_range* ranges, util::function_ptr<void ()> callback)
{
    std::copy(std::begin(subranges), std::end(subranges), ranges);
    callback();
}

void Cube::LoadVertices(vertex* vertexBuffer, util::function_ptr<void ()> callback)
{
    std::copy(std::begin(vertices), std::end(vertices), vertexBuffer);
    callback();
}

void Cube::LoadElements(uint32_t* elementBuffer, util::function_ptr<void ()> callback)
{
    std::copy(std::begin(elements), std::end(elements), elementBuffer);
    callback();
}

unsigned Cube::GetVersion()
{
    return 1;
}

} /* namespace dse::core */

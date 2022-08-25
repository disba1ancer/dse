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
		{{ 1, -1, -1}, { 1, 0, 0}, {0, 1, 0}, {0, 0}, 0},//+x
		{{ 1, -1,  1}, { 1, 0, 0}, {0, 1, 0}, {0, 1}, 0},//1
		{{ 1,  1, -1}, { 1, 0, 0}, {0, 1, 0}, {1, 0}, 0},//2
		{{ 1,  1,  1}, { 1, 0, 0}, {0, 1, 0}, {1, 1}, 0},//3

		{{-1, -1, -1}, {-1, 0, 0}, {0, 1, 0}, {0, 0}, 0},//-x
		{{-1, -1,  1}, {-1, 0, 0}, {0, 1, 0}, {0, 1}, 0},//5
		{{-1,  1, -1}, {-1, 0, 0}, {0, 1, 0}, {1, 0}, 0},//6
		{{-1,  1,  1}, {-1, 0, 0}, {0, 1, 0}, {1, 1}, 0},//7

		{{-1,  1, -1}, {0,  1, 0}, {1, 0, 0}, {0, 0}, 0},//+y
		{{ 1,  1, -1}, {0,  1, 0}, {1, 0, 0}, {0, 1}, 0},//9
		{{-1,  1,  1}, {0,  1, 0}, {1, 0, 0}, {1, 0}, 0},//10
		{{ 1,  1,  1}, {0,  1, 0}, {1, 0, 0}, {1, 1}, 0},//11

		{{-1, -1, -1}, {0, -1, 0}, {1, 0, 0}, {0, 0}, 0},//-y
		{{ 1, -1, -1}, {0, -1, 0}, {1, 0, 0}, {0, 1}, 0},//13
		{{-1, -1,  1}, {0, -1, 0}, {1, 0, 0}, {1, 0}, 0},//14
		{{ 1, -1,  1}, {0, -1, 0}, {1, 0, 0}, {1, 1}, 0},//15

		{{-1, -1,  1}, {0, 0,  1}, {1, 0, 0}, {0, 0}, 0},//+z
		{{-1,  1,  1}, {0, 0,  1}, {1, 0, 0}, {0, 1}, 0},//17
		{{ 1, -1,  1}, {0, 0,  1}, {1, 0, 0}, {1, 0}, 0},//18
		{{ 1,  1,  1}, {0, 0,  1}, {1, 0, 0}, {1, 1}, 0},//19

		{{-1, -1, -1}, {0, 0, -1}, {1, 0, 0}, {0, 0}, 0},//-z
		{{-1,  1, -1}, {0, 0, -1}, {1, 0, 0}, {0, 1}, 0},//21
		{{ 1, -1, -1}, {0, 0, -1}, {1, 0, 0}, {1, 0}, 0},//22
		{{ 1,  1, -1}, {0, 0, -1}, {1, 0, 0}, {1, 1}, 0} //23
});
static const auto elements = std::to_array<std::uint32_t>({
		 2,  1,  0,  1,  2,  3,//+x
		 4,  5,  6,  7,  6,  5,//-x
		10,  9,  8,  9, 10, 11,//+y
		12, 13, 14, 15, 14, 13,//-y
		18, 17, 16, 17, 18, 19,//+z
		20, 21, 22, 23, 22, 21,//-z
});
static const auto subranges = std::to_array<IMesh::submesh_range>({
		{0, 36},//+x
//		{6, 12},//-x
//		{12, 18},//+y
//		{18, 24},//-y
//		{24, 30},//+z
//		{30, 36},//-z
});
}

void Cube::LoadMeshParameters(mesh_parameters* parameters, util::FunctionPtr<void ()> callback)
{
	*parameters = { std::size(vertices), std::size(elements), std::size(subranges) };
	callback();
}

void Cube::LoadSubmeshRanges(submesh_range* ranges, util::FunctionPtr<void ()> callback)
{
	std::copy(std::begin(subranges), std::end(subranges), ranges);
	callback();
}

void Cube::LoadVertices(vertex* vertexBuffer, util::FunctionPtr<void ()> callback)
{
	std::copy(std::begin(vertices), std::end(vertices), vertexBuffer);
	callback();
}

void Cube::LoadElements(uint32_t* elementBuffer, util::FunctionPtr<void ()> callback)
{
	std::copy(std::begin(elements), std::end(elements), elementBuffer);
	callback();
}

unsigned Cube::GetVersion()
{
	return 1;
}

} /* namespace dse::core */

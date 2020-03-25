/*
 * Cube.cpp
 *
 *  Created on: 16 февр. 2020 г.
 *      Author: disba1ancer
 */

#include "Cube.h"
#include <cstring>
#include <iterator>

namespace {
static const dse::scn::IMesh::vertex vertices[24] = {
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
};
static const std::uint32_t elements[36] = {
		 2,  1,  0,  1,  2,  3,//+x
		 4,  5,  6,  7,  6,  5,//-x
		10,  9,  8,  9, 10, 11,//+y
		12, 13, 14, 15, 14, 13,//-y
		18, 17, 16, 17, 18, 19,//+z
		20, 21, 22, 23, 22, 21,//-z
};
}

namespace dse {
namespace scn {

IMesh::mesh_parameters Cube::getMeshParameters() {
	return {std::size(vertices), std::size(elements), 1};
}

unsigned dse::scn::Cube::getVersion() {
	return 0;
}

void Cube::loadVerticesRange(IMesh::vertex *vertexBuffer,
		uint32_t startVertex, uint32_t vertexCount) {
	std::memcpy(vertexBuffer, vertices + startVertex, sizeof(IMesh::vertex) * vertexCount);
}

void Cube::loadElementsRange(uint32_t *elementBuffer, uint32_t startElement,
		uint32_t elementCount) {
	std::memcpy(elementBuffer, elements + startElement, sizeof(std::uint32_t) * elementCount);
}

dse::scn::IMesh::submesh_range Cube::getSubmeshRange(uint32_t submeshIndex) {
	return {0, std::size(elements)};
}

} /* namespace scn */
} /* namespace dse */

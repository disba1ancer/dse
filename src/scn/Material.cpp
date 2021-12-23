/*
 * Material.cpp
 *
 *  Created on: 27 июн. 2020 г.
 *      Author: disba1ancer
 */

#include "Material.h"

namespace dse {
namespace scn {

Material::Material(ITextureDataProvider* texture, math::vec4 color) :
	color(color),
	texture(texture),
	version(1)
{}

Material::Material(ITextureDataProvider* texture)  :
	Material(texture, {1.f, 0.f, 1.f})
{}

Material::Material(math::vec4 color) :
	Material(nullptr, color)
{}

Material::Material() :
	Material(nullptr, {1.f, 0.f, 1.f})
{}

auto Material::GetColor() const -> math::vec4
{
	return color;
}

void Material::SetColor(math::vec4 color)
{
	this->color = color;
	IncrementVersion();
}

auto Material::GetTexture() const -> ITextureDataProvider*
{
	return texture;
}

void Material::SetTexture(ITextureDataProvider* texture)
{
	this->texture = texture;
	IncrementVersion();
}

uint32_t Material::GetVersion()
{
	return version;
}

void Material::IncrementVersion()
{
	bool t = !++version;
	version += t;
}

} /* namespace scn */
} /* namespace dse */

/*
 * Material.cpp
 *
 *  Created on: 27 июн. 2020 г.
 *      Author: disba1ancer
 */

#include "Material.h"

namespace dse {
namespace scn {

Material::Material(math::vec4 color) : color(color) {
}

math::vec4 Material::getColor() const {
	return color;
}

void Material::setColor(math::vec4 color) {
	this->color = color;
}

} /* namespace scn */
} /* namespace dse */

/*
 * Material.h
 *
 *  Created on: 27 июн. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_MATERIAL_H_
#define DSE_CORE_MATERIAL_H_

#include <dse/math/vec.h>
#include "ITextureDataProvider.h"
#include <cstdint>

namespace dse::core {

class Material {
	math::vec4 color;
	ITextureDataProvider* texture;
	std::uint32_t version;
public:
	Material(ITextureDataProvider* texture, math::vec4 color);
	Material(ITextureDataProvider* texture);
	Material(math::vec4 color);
	Material();
	auto GetColor() const -> math::vec4;
	void SetColor(math::vec4 color);
	auto GetTexture() const -> ITextureDataProvider*;
	void SetTexture(ITextureDataProvider* texture);
	auto GetVersion() -> std::uint32_t;
private:
	void IncrementVersion();
};

} /* namespace dse::core */

#endif /* DSE_CORE_MATERIAL_H_ */

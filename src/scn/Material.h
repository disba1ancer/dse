/*
 * Material.h
 *
 *  Created on: 27 июн. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SCN_MATERIAL_H_
#define SCN_MATERIAL_H_

#include "math/vec.h"

namespace dse {
namespace scn {

class Material {
public:
	Material(math::vec4 color = {1.f, 0.f, 1.f});
	math::vec4 getColor() const;
	void setColor(math::vec4 color);

private:
	math::vec4 color;
};

} /* namespace scn */
} /* namespace dse */

#endif /* SCN_MATERIAL_H_ */

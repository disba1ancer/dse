/*
 * Camera.h
 *
 *  Created on: 8 мар. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_CAMERA_H_
#define DSE_CORE_CAMERA_H_

#include <dse/math/vec.h>
#include "detail/impexp.h"

namespace dse::core {

class API_DSE_CORE Camera {
	math::vec3 pos;
	float aspect;
	math::vec4 qRot;
	float focalLength;
	float zNear;
	float zFar;
public:
	Camera();
	~Camera() = default;
	Camera(const Camera &other) = default;
	Camera(Camera &&other) = default;
	Camera& operator=(const Camera &other) = default;
	Camera& operator=(Camera &&other) = default;

	float getAspect() const;
	void setAspect(float aspect);
	float getFocalLength() const;
	void setFocalLength(float focalLength);
	math::vec3 getPos() const;
	void setPos(const math::vec3 &pos);
	math::vec4 getRot() const;
	void setRot(const math::vec4 &rot);
	float getFar() const;
	void setFar(float far);
	float getNear() const;
	void setNear(float near);
};

} /* namespace dse::core */

#endif /* DSE_CORE_CAMERA_H_ */

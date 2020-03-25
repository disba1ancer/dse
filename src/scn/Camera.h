/*
 * Camera.h
 *
 *  Created on: 8 мар. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SCN_CAMERA_H_
#define SCN_CAMERA_H_

#include "../math/vec.h"

namespace dse {
namespace scn {

class Camera {
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

} /* namespace scn */
} /* namespace dse */

#endif /* SCN_CAMERA_H_ */

/*
 * Camera.cpp
 *
 *  Created on: 8 мар. 2020 г.
 *      Author: disba1ancer
 */

#include "Camera.h"
#include "math/constants.h"

namespace dse {
namespace scn {

float Camera::getAspect() const {
	return aspect;
}

void Camera::setAspect(float aspect) {
	this->aspect = aspect;
}

float Camera::getFocalLength() const {
	return focalLength;
}

void Camera::setFocalLength(float focalLength) {
	this->focalLength = focalLength;
}

math::vec3 Camera::getPos() const {
	return pos;
}

void Camera::setPos(const math::vec3 &pos) {
	this->pos = pos;
}

math::vec4 Camera::getRot() const {
	return qRot;
}

void Camera::setRot(const math::vec4 &rot) {
	qRot = rot;
}

float Camera::getFar() const {
	return zFar;
}

void Camera::setFar(float far) {
	zFar = far;
}

float Camera::getNear() const {
	return zNear;
}

void Camera::setNear(float near) {
	zNear = near;
}

Camera::Camera() :
		pos {0.f, 0.f, 0.f},
		aspect (0.f),
		qRot {0.f, 0.f, 0.f, 1.f},
		focalLength (1.0),
		zNear (0.f),
		zFar (0.f)
{}

} /* namespace scn */
} /* namespace dse */

/*
 * Texture.h
 *
 *  Created on: 2 июл. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_TEXTURE_H_
#define SUBSYS_GL_TEXTURE_H_

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include "gl.h"
#include "TrvMvOnlyRes.h"

namespace dse::ogl31rbe::glwrp {

class Texture : TrvMvOnlyRes<GLuint, true> {
protected:
	Texture(bool nonempty = true) noexcept : TrvMvOnlyRes(0) {
		if (nonempty) {
			glGenTextures(1, &resource);
		}
	}
public:
	~Texture() {
		if (resource) {
			glDeleteTextures(1, &resource);
		}
	}
	Texture(Texture &&other) = default;
	Texture& operator=(Texture &&other) = default;
	operator GLuint() noexcept {
		return resource;
	}
};

template <gl::GLenum _target>
class TargetTexture: public Texture {
public:
	TargetTexture(bool nonempty = true) noexcept : Texture(nonempty) {
		if (nonempty) {
			bind();
		}
	}
	void bind() noexcept {
		glBindTexture(target, *this);
	}
	static void unbind() noexcept {
		glBindTexture(target, 0);
	}
	static constexpr gl::GLenum target = _target;
};

typedef TargetTexture<GL_TEXTURE_2D> Texture2D;
typedef TargetTexture<GL_TEXTURE_2D_ARRAY> Texture2DArray;
typedef TargetTexture<GL_TEXTURE_RECTANGLE> TextureRectangle;

} /* dse::ogl31rbe::glwrp */

#endif /* SUBSYS_GL_BUFFER_H_ */

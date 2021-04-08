/*
 * Buffer.h
 *
 *  Created on: 28 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_BUFFER_H_
#define SUBSYS_GL_BUFFER_H_

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include "gl.h"
#include "TrvMvOnlyRes.h"

namespace dse::renders::glwrp {

class Buffer : TrvMvOnlyRes<GLuint, true> {
protected:
	Buffer(bool nonempty = true) noexcept : TrvMvOnlyRes(0) {
		if (nonempty) {
			glGenBuffers(1, &resource);
		}
	}
public:
	~Buffer() {
		if (resource) {
			glDeleteBuffers(1, &resource);
		}
	}
	Buffer(Buffer &&other) = default;
	Buffer& operator=(Buffer &&other) = default;
	operator GLuint() noexcept {
		return resource;
	}
};

template <gl::GLenum target>
class TargetBuffer: public Buffer {
public:
	TargetBuffer(bool nonempty = true) noexcept : Buffer(nonempty) {
		if (nonempty) {
			bind();
		}
	}
	void bind() {
		glBindBuffer(target, *this);
	}
};

typedef TargetBuffer<GL_ELEMENT_ARRAY_BUFFER> ElementBuffer;
typedef TargetBuffer<GL_ARRAY_BUFFER> VertexBuffer;
typedef TargetBuffer<GL_UNIFORM_BUFFER> UniformBuffer;

} /* namespace dse::renders::glwrp */

#endif /* SUBSYS_GL_BUFFER_H_ */

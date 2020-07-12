/*
 * Buffer.h
 *
 *  Created on: 28 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_BUFFER_H_
#define SUBSYS_GL_BUFFER_H_

#include "gl.h"
#include "TrvMvOnlyRes.h"

namespace dse {
namespace subsys {
namespace gl {

class Buffer : TrvMvOnlyRes<GLuint, true> {
protected:
	Buffer(bool nonempty = true) noexcept : TrvMvOnlyRes(0) {
		if (nonempty) {
			glGenBuffers(1, &resource);
		}
	}
public:
	~Buffer() noexcept {
		if (resource) {
			glDeleteBuffers(1, &resource);
		}
	}
	Buffer(Buffer &&other) noexcept = default;
	Buffer& operator=(Buffer &&other) noexcept = default;
	operator GLuint() noexcept {
		return resource;
	}
};

template <GLenum target>
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

} /* namespace gl */
} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_GL_BUFFER_H_ */

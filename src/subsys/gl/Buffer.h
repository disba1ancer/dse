/*
 * Buffer.h
 *
 *  Created on: 28 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_BUFFER_H_
#define SUBSYS_GL_BUFFER_H_

#include "gl.h"
#include <utility>

namespace dse {
namespace subsys {
namespace gl {

class Buffer {
	GLuint buffer;
protected:
	Buffer(bool isEmpty = false) noexcept : buffer(0) {
		if (!isEmpty) {
			glGenBuffers(1, &buffer);
		}
	}
public:
	~Buffer() noexcept {
		if (buffer) {
			glDeleteBuffers(1, &buffer);
		}
	}
	Buffer(const Buffer &other) = delete;
	Buffer(Buffer &&other) noexcept : buffer(other.buffer) {
		other.buffer = 0;
	}
	Buffer& operator=(const Buffer &other) = delete;
	Buffer& operator=(Buffer &&other) noexcept {
		std::swap(buffer, other.buffer);
		return *this;
	}
	operator GLuint() noexcept {
		return buffer;
	}
};

template <GLenum target>
class TargetBuffer: public Buffer {
public:
	TargetBuffer(bool isEmpty = false) noexcept : Buffer(isEmpty) {
		if (!isEmpty) {
			glBindBuffer(target, *this);
		}
	}
	~TargetBuffer() = default;
	TargetBuffer(const TargetBuffer &other) = default;
	TargetBuffer(TargetBuffer &&other) = default;
	TargetBuffer& operator=(const TargetBuffer &other) = default;
	TargetBuffer& operator=(TargetBuffer &&other) = default;
};

typedef TargetBuffer<GL_ELEMENT_ARRAY_BUFFER> ElementBuffer;
typedef TargetBuffer<GL_ARRAY_BUFFER> VertexBuffer;

} /* namespace gl */
} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_GL_BUFFER_H_ */

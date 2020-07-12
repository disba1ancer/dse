/*
 * Framebuffer.h
 *
 *  Created on: 1 июл. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_FRAMEBUFFER_H_
#define SUBSYS_GL_FRAMEBUFFER_H_

#include "gl.h"
#include "TrvMvOnlyRes.h"

namespace dse {
namespace subsys {
namespace gl {

class FrameBuffer : TrvMvOnlyRes<GLuint, true> {
public:
	FrameBuffer(bool nonempty = true) noexcept : TrvMvOnlyRes(0) {
		if (nonempty) {
			glGenFramebuffers(1, &resource);
		}
	}
	~FrameBuffer() noexcept {
		if (resource) {
			glDeleteFramebuffers(1, &resource);
		}
	}
	FrameBuffer(FrameBuffer &&other) noexcept = default;
	FrameBuffer& operator=(FrameBuffer &&other) noexcept = default;
	operator GLuint() noexcept {
		return resource;
	}
};

} /* namespace gl */
} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_GL_FRAMEBUFFER_H_ */

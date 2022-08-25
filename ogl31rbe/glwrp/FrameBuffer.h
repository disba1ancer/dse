/*
 * Framebuffer.h
 *
 *  Created on: 1 июл. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_FRAMEBUFFER_H_
#define SUBSYS_GL_FRAMEBUFFER_H_

#include <glbinding/gl/functions.h>
#include "gl.h"
#include "TrvMvOnlyRes.h"

namespace dse::ogl31rbe::glwrp {

class FrameBuffer : TrvMvOnlyRes<GLuint, true> {
public:
	FrameBuffer(bool nonempty = true) noexcept : TrvMvOnlyRes(0) {
		if (nonempty) {
			glGenFramebuffers(1, &resource);
		}
	}
	~FrameBuffer() {
		if (resource) {
			glDeleteFramebuffers(1, &resource);
		}
	}
	FrameBuffer(FrameBuffer &&other) = default;
	FrameBuffer& operator=(FrameBuffer &&other) = default;
	operator GLuint() noexcept {
		return resource;
	}
};

} /* namespace dse::ogl31rbe::glwrp */

#endif /* SUBSYS_GL_FRAMEBUFFER_H_ */

/*
 * RenderBuffer.h
 *
 *  Created on: 4 июл. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_RENDERBUFFER_H_
#define SUBSYS_GL_RENDERBUFFER_H_

#include "gl.h"
#include "TrvMvOnlyRes.h"

namespace dse::renders::glwrp {

class RenderBuffer : TrvMvOnlyRes<GLuint, true> {
public:
	RenderBuffer(bool nonempty = true) : TrvMvOnlyRes(0) {
		if (nonempty) {
			glGenRenderbuffers(1, &resource);
		}
	}
	~RenderBuffer() {
		if (resource) {
			glDeleteRenderbuffers(1, &resource);
		}
	}
	RenderBuffer(RenderBuffer&&) = default;
	RenderBuffer& operator=(RenderBuffer&&) = default;
	operator GLuint() {
		return resource;
	}
};

} // dse::renders::glwrp

#endif /* SUBSYS_GL_RENDERBUFFER_H_ */

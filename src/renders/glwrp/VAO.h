/*
 * VAO.h
 *
 *  Created on: 23 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_VAO_H_
#define SUBSYS_GL_VAO_H_

#include <glbinding/gl/functions.h>
#include "gl.h"
#include "TrvMvOnlyRes.h"

namespace dse::renders::glwrp {

class VAO : TrvMvOnlyRes<GLuint, true> {
public:
	VAO(bool nonempty = true) noexcept : TrvMvOnlyRes(0) {
		if (nonempty) {
			glGenVertexArrays(1, &resource);
			glBindVertexArray(resource);
		}
	}
	~VAO() {
		if (resource) {
			glDeleteVertexArrays(1, &resource);
		}
	}
	VAO(VAO &&other) = default;
	VAO& operator=(VAO &&other) = default;
	operator GLuint() noexcept {
		return resource;
	}
};

} /* dse::renders::glwrp */

#endif /* SUBSYS_GL_VAO_H_ */

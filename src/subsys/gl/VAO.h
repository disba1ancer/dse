/*
 * VAO.h
 *
 *  Created on: 23 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_VAO_H_
#define SUBSYS_GL_VAO_H_

#include "gl.h"
#include "TrvMvOnlyRes.h"

namespace dse {
namespace subsys {
namespace gl {

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

} /* namespace gl31_impl */
} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_GL_VAO_H_ */

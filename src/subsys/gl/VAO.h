/*
 * VAO.h
 *
 *  Created on: 23 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_VAO_H_
#define SUBSYS_GL_VAO_H_

#include "gl.h"
#include <utility>

namespace dse {
namespace subsys {
namespace gl {

class VAO {
	GLuint vao;
public:
	VAO(bool isEmpty = false) : vao(0) {
		if (!isEmpty) {
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);
		}
	}
	~VAO() {
		if (vao) {
			glDeleteVertexArrays(1, &vao);
		}
	}
	VAO(const VAO &other) = delete;
	VAO(VAO &&other) : vao(other.vao) {
		other.vao = 0;
	}
	VAO& operator=(const VAO &other) = delete;
	VAO& operator=(VAO &&other) {
		std::swap(vao, other.vao);
		return *this;
	}
	operator GLuint() {
		return vao;
	}
};

} /* namespace gl31_impl */
} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_GL_VAO_H_ */

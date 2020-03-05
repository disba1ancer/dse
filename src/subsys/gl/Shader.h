/*
 * Shader.h
 *
 *  Created on: 29 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_SHADER_H_
#define SUBSYS_GL_SHADER_H_

#include "gl.h"
#include <utility>
#include <string>
#include <exception>

namespace dse {
namespace subsys {
namespace gl {

class Shader {
	GLuint shader;
protected:
	Shader(GLuint shader) : shader(shader) {
	}
public:
	~Shader() {
		glDeleteShader(shader);
	}
	Shader(const Shader &other) = delete;
	Shader(Shader &&other) : shader(other.shader) {
		other.shader = 0;
	}
	Shader& operator=(const Shader &other) = delete;
	Shader& operator=(Shader &&other) {
		std::swap(shader, other.shader);
		return *this;
	}
	void loadSource(const char *source) {
		const char *src[] = {
				"#version 140",
				source
		};
		glShaderSource(shader, sizeof(src) / sizeof(src[0]), src, nullptr);
	}
	void compile() {
		glCompileShader(shader);
		GLint status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (!status) {
			GLsizei logSize;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
			std::string log;
			log.resize(logSize);
			glGetShaderInfoLog(shader, logSize, &logSize, log.data());
			throw std::runtime_error(std::move(log));
		}
	}
	operator GLuint() {
		return shader;
	}
};

} /* namespace gl */
} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_GL_SHADER_H_ */

/*
 * Program.h
 *
 *  Created on: 29 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_PROGRAM_H_
#define SUBSYS_GL_PROGRAM_H_

#include "gl.h"
#include <utility>
#include "Shader.h"

namespace dse {
namespace subsys {
namespace gl {

class Program {
	GLuint program;
public:
	Program() : program(glCreateProgram()) {
	}
	~Program() {
		glDeleteProgram(program);
	}
	Program(const Program &other) = delete;
	Program(Program &&other) : program(other.program) {
		other.program = 0;
	}
	Program& operator=(const Program &other) = delete;
	Program& operator=(Program &&other) {
		std::swap(program, other.program);
		return *this;
	}
	void attachShader(Shader& shader) {
		glAttachShader(program, shader);
	}
	void link() {
		glLinkProgram(program);
		GLint status;
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if (!status) {
			GLsizei logSize;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
			std::string log;
			log.resize(logSize);
			glGetProgramInfoLog(program, logSize, &logSize, log.data());
			throw std::runtime_error(std::move(log));
		}
	}
	operator GLuint() {
		return program;
	}
};

} /* namespace gl */
} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_GL_PROGRAM_H_ */

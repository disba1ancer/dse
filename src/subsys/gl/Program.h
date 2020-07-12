/*
 * Program.h
 *
 *  Created on: 29 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_PROGRAM_H_
#define SUBSYS_GL_PROGRAM_H_

#include "Shader.h"

namespace dse {
namespace subsys {
namespace gl {

class Program : TrvMvOnlyRes<GLuint, true> {
public:
	Program() noexcept : TrvMvOnlyRes(glCreateProgram()) {
	}
	~Program() {
		glDeleteProgram(resource);
	}
	Program(Program &&other) noexcept = default;
	Program& operator=(Program &&other) noexcept = default;
	void attachShader(Shader& shader) noexcept {
		glAttachShader(resource, shader);
	}
	void link() {
		glLinkProgram(resource);
		GLint status;
		glGetProgramiv(resource, GL_LINK_STATUS, &status);
		if (!status) {
			GLsizei logSize;
			glGetProgramiv(resource, GL_INFO_LOG_LENGTH, &logSize);
			std::string log;
			log.resize(logSize);
			glGetProgramInfoLog(resource, logSize, &logSize, log.data());
			throw std::runtime_error(std::move(log));
		}
	}
	operator GLuint() noexcept {
		return resource;
	}
};

} /* namespace gl */
} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_GL_PROGRAM_H_ */

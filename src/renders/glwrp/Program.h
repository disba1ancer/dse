/*
 * Program.h
 *
 *  Created on: 29 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_PROGRAM_H_
#define SUBSYS_GL_PROGRAM_H_

#include "Shader.h"

namespace dse::renders::glwrp {

class Program : TrvMvOnlyRes<GLuint, true> {
public:
	Program() noexcept : TrvMvOnlyRes(glCreateProgram()) {
	}
	~Program() {
		glDeleteProgram(resource);
	}
	Program(Program &&other) = default;
	Program& operator=(Program &&other) = default;
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

} /* namespace dse::renders::glwrp */

#endif /* SUBSYS_GL_PROGRAM_H_ */

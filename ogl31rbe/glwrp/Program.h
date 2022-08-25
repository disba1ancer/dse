/*
 * Program.h
 *
 *  Created on: 29 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_PROGRAM_H_
#define SUBSYS_GL_PROGRAM_H_

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include "Shader.h"

namespace dse::ogl31rbe::glwrp {

class Program : TrvMvOnlyRes<GLuint, true> {
public:
	Program() : TrvMvOnlyRes(glCreateProgram()) {
	}
	~Program() {
		glDeleteProgram(resource);
	}
	Program(Program &&other) = default;
	Program& operator=(Program &&other) = default;
	void attachShader(Shader& shader) {
		glAttachShader(resource, shader);
	}
	GLuint getUniformBlockIndex(const GLchar* uniformBlockName) {
		return glGetUniformBlockIndex(resource, uniformBlockName);
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

} /* namespace dse::ogl31rbe::glwrp */

#endif /* SUBSYS_GL_PROGRAM_H_ */

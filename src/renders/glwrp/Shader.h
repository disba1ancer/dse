/*
 * Shader.h
 *
 *  Created on: 29 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_SHADER_H_
#define SUBSYS_GL_SHADER_H_

#include <string>
#include <exception>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include "gl.h"
#include "TrvMvOnlyRes.h"

namespace dse::renders::glwrp {

class Shader : TrvMvOnlyRes<GLuint, true> {
protected:
	Shader(GLuint shader) noexcept : TrvMvOnlyRes(shader) {
	}
public:
	~Shader() {
		glDeleteShader(resource);
	}
	Shader(Shader&&) = default;
	Shader& operator=(Shader&&) = default;
	void loadSource(const char *source) noexcept {
		const char *src[] = {
				"#version 140",
				source
		};
		glShaderSource(resource, sizeof(src) / sizeof(src[0]), src, nullptr);
	}
	void compile() {
		glCompileShader(resource);
		GLint status;
		glGetShaderiv(resource, GL_COMPILE_STATUS, &status);
		if (!status) {
			GLsizei logSize;
			glGetShaderiv(resource, GL_INFO_LOG_LENGTH, &logSize);
			std::string log;
			log.resize(logSize);
			glGetShaderInfoLog(resource, logSize, &logSize, log.data());
			throw std::runtime_error(std::move(log));
		}
	}
	operator GLuint() noexcept {
		return resource;
	}
};

template <gl::GLenum type>
class ConcreteShader : public Shader {
public:
	ConcreteShader() noexcept : Shader(glCreateShader(type)) {}
};

typedef ConcreteShader<GL_VERTEX_SHADER> VertexShader;
typedef ConcreteShader<GL_FRAGMENT_SHADER> FragmentShader;

} /* dse::renders::glwrp */

#endif /* SUBSYS_GL_SHADER_H_ */

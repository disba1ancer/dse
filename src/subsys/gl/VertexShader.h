/*
 * VertexShader.h
 *
 *  Created on: 29 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_VERTEXSHADER_H_
#define SUBSYS_GL_VERTEXSHADER_H_

#include "Shader.h"

namespace dse {
namespace subsys {
namespace gl {

class VertexShader: public Shader {
public:
	VertexShader() : Shader(glCreateShader(GL_VERTEX_SHADER)) {}
	~VertexShader() = default;
	VertexShader(const VertexShader &other) = default;
	VertexShader(VertexShader &&other) = default;
	VertexShader& operator=(const VertexShader &other) = default;
	VertexShader& operator=(VertexShader &&other) = default;
};

} /* namespace gl */
} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_GL_VERTEXSHADER_H_ */

/*
 * FragmentShader.h
 *
 *  Created on: 29 февр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_FRAGMENTSHADER_H_
#define SUBSYS_GL_FRAGMENTSHADER_H_

#include "Shader.h"

namespace dse {
namespace subsys {
namespace gl {

class FragmentShader: public Shader {
public:
	FragmentShader() : Shader(glCreateShader(GL_FRAGMENT_SHADER)) {}
	~FragmentShader() = default;
	FragmentShader(const FragmentShader &other) = default;
	FragmentShader(FragmentShader &&other) = default;
	FragmentShader& operator=(const FragmentShader &other) = default;
	FragmentShader& operator=(FragmentShader &&other) = default;
};

} /* namespace gl */
} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_GL_FRAGMENTSHADER_H_ */

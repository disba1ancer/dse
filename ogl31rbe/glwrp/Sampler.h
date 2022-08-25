#ifndef SAMPLER_H
#define SAMPLER_H

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include "gl.h"
#include "TrvMvOnlyRes.h"

namespace dse::ogl31rbe::glwrp {

class Sampler : TrvMvOnlyRes<GLuint, true> {
public:
	Sampler(bool nonempty = true) noexcept : TrvMvOnlyRes(0) {
		if (nonempty) {
			glGenSamplers(1, &resource);
		}
	}
	~Sampler() {
		if (resource) {
			glDeleteSamplers(1, &resource);
		}
	}
	Sampler(Sampler &&other) = default;
	Sampler& operator=(Sampler &&other) = default;
	operator GLuint() noexcept {
		return resource;
	}
};

}

#endif // SAMPLER_H

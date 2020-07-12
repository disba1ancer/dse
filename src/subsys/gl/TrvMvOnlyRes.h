/*
 * TrivialMoveResource.h
 *
 *  Created on: 1 июл. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_TRVMVONLYRES_H_
#define SUBSYS_GL_TRVMVONLYRES_H_

#include <utility>

namespace dse {
namespace subsys {
namespace gl {

template <typename Rs, bool inherit>
class TrvMvOnlyRes;

template <typename Rs>
class TrvMvOnlyRes<Rs, true> {
public:
	operator Rs() {
		return resource;
	}
protected:
	TrvMvOnlyRes(Rs res) : resource(res){}
	TrvMvOnlyRes(const TrvMvOnlyRes&) = delete;
	TrvMvOnlyRes(TrvMvOnlyRes&& other) : resource(other.resource) {
		other.resource = 0;
	}
	TrvMvOnlyRes& operator=(const TrvMvOnlyRes&) = delete;
	TrvMvOnlyRes& operator=(TrvMvOnlyRes&& other) {
		std::swap(resource, other.resource);
		return *this;
	}
	Rs resource;
};

template <typename Rs>
class TrvMvOnlyRes<Rs, false> {
public:
	TrvMvOnlyRes(Rs res) : resource(res){}
	TrvMvOnlyRes(const TrvMvOnlyRes&) = delete;
	TrvMvOnlyRes(TrvMvOnlyRes&& other) : resource(other.resource) {
		other.resource = 0;
	}
	TrvMvOnlyRes& operator=(const TrvMvOnlyRes&) = delete;
	TrvMvOnlyRes& operator=(TrvMvOnlyRes&& other) {
		std::swap(resource, other.resource);
		return *this;
	}
	Rs resource;
};

} /* namespace gl */
} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_GL_TRVMVONLYRES_H_ */

/*
 * binds.h
 *
 *  Created on: 3 мар. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL31_IMPL_BINDS_H_
#define SUBSYS_GL31_IMPL_BINDS_H_

namespace dse {
namespace subsys {
namespace gl31_impl {

enum InputParams {
	Position, Normal, Tangent, UV
};

enum OutputParams {
	FragmentColor
};

} /* namespace gl31_impl */
} /* namespace subsys */
} /* namespace dse */

#endif /* SUBSYS_GL31_IMPL_BINDS_H_ */

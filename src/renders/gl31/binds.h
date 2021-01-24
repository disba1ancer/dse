/*
 * binds.h
 *
 *  Created on: 3 мар. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL31_IMPL_BINDS_H_
#define SUBSYS_GL31_IMPL_BINDS_H_

namespace dse::renders::gl31 {

enum InputParams {
	Position, Normal, Tangent, UV
};

enum OutputParams {
	FragmentColor
};

} /* namespace dse::renders::gl31 */

#endif /* SUBSYS_GL31_IMPL_BINDS_H_ */

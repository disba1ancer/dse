/*
 * FinalStep.h
 *
 *  Created on: 16 мая 2020 г.
 *      Author: disba1ancer
 */

#ifndef UTIL_FINALSTEP_H_
#define UTIL_FINALSTEP_H_

#include <algorithm>

namespace dse {
namespace util {

template <typename T>
class FinalStep {
	T final;
public:
	FinalStep(T& onFinal) : final(onFinal) {}
	FinalStep(T&& onFinal) : final(std::move(onFinal)) {}
	~FinalStep() noexcept { final(); }
};

} // namespace util
} // namespace dse

#endif /* UTIL_FINALSTEP_H_ */

/*
 * ProxyContainer.h
 *
 *  Created on: 19 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef UTIL_PROXYCONTAINER_H_
#define UTIL_PROXYCONTAINER_H_

namespace dse {
namespace util {

template <auto, auto>
class ProxyContainer;

template <typename Owner, typename It, It(Owner::*a_begin)(), It(Owner::*a_end)()>
class ProxyContainer<a_begin, a_end> {
	Owner* owner;
	friend Owner;
	ProxyContainer(Owner* owner) : owner(owner) {}
public:
	It begin() { return (owner->*a_begin)(); }
	It end() { return (owner->*a_end)(); }
};

} /* namespace util */
} /* namespace dse */

#endif /* UTIL_PROXYCONTAINER_H_ */

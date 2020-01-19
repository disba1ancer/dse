/*
 * ProxyIterator.h
 *
 *  Created on: 19 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef UTIL_PROXYITERATOR_H_
#define UTIL_PROXYITERATOR_H_

namespace dse {
namespace util {

template <typename Cont, typename Owner>
class ProxyIterator {
	typename Cont::iterator it;
	friend Owner;
	ProxyIterator(const typename Cont::iterator& it) : it(it) {}
public:
	ProxyIterator& operator++() { ++it; return *this; };
	typename Cont::value_type& operator*() { return *it; }
	typename Cont::value_type& operator->() { return *it; }
	bool operator!=(const ProxyIterator& other) const { return it != other.it; }
};

} /* namespace util */
} /* namespace dse */

#endif /* UTIL_PROXYITERATOR_H_ */

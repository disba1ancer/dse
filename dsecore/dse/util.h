/*
 * dse_util.h
 *
 *  Created on: 9 мар. 2019 г.
 *      Author: Anton
 */

#ifndef DSE_UTIL_H_
#define DSE_UTIL_H_

#include <cinttypes>
#include <type_traits>

namespace dse {
namespace util {

template <typename Owner, typename Child>
class OwnerStore {
	std::ptrdiff_t offset;
public:
	OwnerStore(Owner* owner) : offset(reinterpret_cast<char*>(static_cast<Child*>(this)) - reinterpret_cast<char*>(owner)) {}
	void setOwner(Owner* owner) { offset = reinterpret_cast<char*>(static_cast<Child*>(this)) - reinterpret_cast<char*>(owner); }
	Owner* getOwner() { return reinterpret_cast<Owner*>(reinterpret_cast<char*>(static_cast<Child*>(this)) - offset); }
	const Owner* getOwner() const { return (reinterpret_cast<const char*>(static_cast<const Child*>(this)) - offset); }
};

template <typename T>
struct reference_to_pointer;

template <typename T>
struct reference_to_pointer {
	typedef T type;
};

template <typename T>
struct reference_to_pointer<T&> {
	typedef T* type;
};

/*template <typename T>
struct reference_to_pointer<T&&> {
	typedef T* type;
};*/

template <typename A>
A as_reference(A a) {
	return a;
}

template <typename A>
A& as_reference(typename std::remove_reference<A>::type* a) {
	return *a;
}

/*template <bool conv, typename A>
typename std::conditional<conv, A&, A*>::type  {
	return *a;
}

template <bool conv, typename A>
A as_reference(A a) {
	return a;
}*/

template <typename M>
struct wrap_method;

template <typename Ret, typename Obj, typename ... Args>
struct wrap_method<Ret (Obj::*)(Args...)> {
	template <Ret(Obj::*method)(Args...)>
	static Ret wrapper(Obj* obj, typename reference_to_pointer<Args>::type ... args) {
		return (obj->*method)(as_reference<Args>(args)...);
	}
};

template <typename Ret, typename Obj, typename ... Args>
struct wrap_method<Ret (Obj::*)(Args ...) const> {
	template <Ret(Obj::*method)(Args...) const>
	static Ret wrapper(const Obj* obj, typename reference_to_pointer<Args>::type ... args) {
		return (obj->*method)(as_reference<Args>(args)...);
	}
};

#define DSE_WRAP_METHOD(method) (&::dse::util::wrap_method<decltype(method)>::template wrapper<method>)

template <typename M>
class wrap_static;

template <typename Ret, typename ... Args>
struct wrap_static<Ret (*)(Args ...)> {
	template <Ret(*function)(Args...), typename Ret2, typename ... Args2>
	static Ret2 wrapper(Args2 ... args) {
		return static_cast<Ret2>(function(static_cast<Args>(args)...));
	}
};

#define DSE_WRAP_STATIC(function, ...) (&::dse::util::wrap_static<decltype(function)>::template wrapper<function, __VA_ARGS__>)

#define DSE_HANDLER(method) DSE_WRAP_STATIC(DSE_WRAP_METHOD(method), void, void*, Event*)

#define GET_PRIVATE(_type)  *static_cast<std::conditional<std::is_const<std::remove_pointer<decltype(this)>::type>::value, const _type, _type>::type*>(this->priv)
#define PRIVATE GET_PRIVATE(PRIVATE_CLASSNAME)

}
}

#endif /* DSE_UTIL_H_ */

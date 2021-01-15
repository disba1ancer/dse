#ifndef FUNCTIONAL_H
#define FUNCTIONAL_H

#include <utility>
#include <type_traits>

namespace dse::util {

template <typename T>
struct class_from_member_ptr;

template <typename M, typename C>
struct class_from_member_ptr<M C::*>{
	using type = C;
};

template <typename T>
using class_from_member_ptr_t = typename class_from_member_ptr<T>::type;

template <typename T>
struct type_from_member_ptr;

template <typename M, typename C>
struct type_from_member_ptr<M C::*>{
	using type = M;
};

template <typename T>
using type_from_member_ptr_t = typename type_from_member_ptr<T>::type;

template <typename T>
struct remove_method_cvref;

#define GENERATE_VARIANT(cv) template <typename R, typename C, typename ... A>\
struct remove_method_cvref<R(C::*)(A...) cv> {\
	using type = R(C::*)(A...);\
}
GENERATE_VARIANT();
GENERATE_VARIANT(const);
GENERATE_VARIANT(volatile);
GENERATE_VARIANT(const volatile);
GENERATE_VARIANT(&);
GENERATE_VARIANT(const&);
GENERATE_VARIANT(volatile&);
GENERATE_VARIANT(const volatile&);
GENERATE_VARIANT(&&);
GENERATE_VARIANT(const&&);
GENERATE_VARIANT(volatile&&);
GENERATE_VARIANT(const volatile&&);
#undef GENERATE_VARIANT

template <typename T>
using remove_method_cvref_t = typename remove_method_cvref<T>::type;

enum cvref {
	none_v,
	const_v,
	volatile_v,
	lref_v = 4,
	rref_v = 8
};

template <typename ... Args, typename A>
constexpr auto select(A(*p)(Args...)) -> A(*)(Args...) {
	return p;
}

template <typename ... Args, typename A, typename B>
constexpr auto select(A(B::*p)(Args...)) -> A(B::*)(Args...) {
	return p;
}
// -> A(B::*)(Args...)cv
#define GENERATE_VARIANT(v, cv) template <int qualifier, typename ... Args, typename A, typename B>\
constexpr auto select(A(B::*p)(Args...)cv) requires(qualifier == (v)) {\
	return p;\
}
GENERATE_VARIANT(none_v,)
GENERATE_VARIANT(const_v, const)
GENERATE_VARIANT(volatile_v, volatile)
GENERATE_VARIANT(const_v | volatile_v, const volatile)
GENERATE_VARIANT(lref_v, &)
GENERATE_VARIANT(lref_v | const_v, const&)
GENERATE_VARIANT(lref_v | volatile_v, volatile&)
GENERATE_VARIANT(lref_v | const_v | volatile_v, const volatile&)
GENERATE_VARIANT(rref_v, &&)
GENERATE_VARIANT(rref_v | const_v, const&&)
GENERATE_VARIANT(rref_v | volatile_v, volatile&&)
GENERATE_VARIANT(rref_v | const_v | volatile_v, const volatile&&)
#undef GENERATE_VARIANT

template <auto>
struct static_wrapper;

#define GENERATE_VARIANT(cv) template <\
	typename R, typename C, typename ... Args,\
	R(C::*m)(Args...)cv>\
struct static_wrapper<m> {\
	using cls_type = C cv;\
	static R function(std::remove_reference_t<cls_type>& cls, Args ... args)\
		requires(!std::is_rvalue_reference_v<cls_type>)\
	{\
		return (cls.*m)(std::forward<Args>(args)...);\
	}\
	static R function(std::remove_reference_t<cls_type>&& cls, Args ... args)\
		requires(!std::is_lvalue_reference_v<cls_type>)\
	{\
		return (cls.*m)(std::forward<Args>(args)...);\
	}\
}

GENERATE_VARIANT();
GENERATE_VARIANT(const);
GENERATE_VARIANT(volatile);
GENERATE_VARIANT(const volatile);
GENERATE_VARIANT(&);
GENERATE_VARIANT(const&);
GENERATE_VARIANT(volatile&);
GENERATE_VARIANT(const volatile&);
GENERATE_VARIANT(&&);
GENERATE_VARIANT(const&&);
GENERATE_VARIANT(volatile&&);
GENERATE_VARIANT(const volatile&&);
#undef GENERATE_VARIANT

template <typename C>
struct operator_wrapper {
	template <typename ... A>
	static auto function(C c, A...a) requires(std::is_reference_v<C>) {
		return c(std::forward<A>(a)...);
	}
};

template <auto>
class from_method;

#define GENERATE_VARIANT(cv) template <\
	typename Ret, typename Obj, typename ... Args,\
	Ret(Obj::*method)(Args...) cv\
>\
class from_method<method> {\
public:\
	using cls_ref_type = Obj cv;\
	using cls_type = std::remove_reference_t<cls_ref_type>;\
	using compatible_type = Ret(Args...);\
private:\
	cls_type * obj;\
public:\
	constexpr from_method(cls_type& obj) noexcept : obj(&obj) {}\
	constexpr from_method(cls_type&& obj) noexcept = delete;\
	Ret operator()(Args ... args) const { return (obj->*method)(std::forward<Args>(args)...); }\
	constexpr cls_type * get() const noexcept { return obj; }\
}
GENERATE_VARIANT();
GENERATE_VARIANT(const);
GENERATE_VARIANT(volatile);
GENERATE_VARIANT(const volatile);
GENERATE_VARIANT(&);
GENERATE_VARIANT(const&);
GENERATE_VARIANT(volatile&);
GENERATE_VARIANT(const volatile&);
// specializations for rvalue references is not allowed
#undef GENERATE_VARIANT

template <typename T>
struct is_from_method {
	static constexpr bool value = false;
};

#define GENERATE_VARIANT(cv) template <\
	typename R, typename C, typename ... A, R(C::*m)(A...) cv\
>\
struct is_from_method<from_method<m>> {\
	static constexpr bool value = true;\
}
GENERATE_VARIANT();
GENERATE_VARIANT(const);
GENERATE_VARIANT(volatile);
GENERATE_VARIANT(const volatile);
GENERATE_VARIANT(&);
GENERATE_VARIANT(const&);
GENERATE_VARIANT(volatile&);
GENERATE_VARIANT(const volatile&);
#undef DECARE_is_from_method

template <typename T>
inline constexpr auto is_from_method_v = is_from_method<T>::value;

template <typename F>
class function_view;

template <typename R, typename ... Args>
class function_view<R(Args...)> {
public:
	using fptr = R(*)(Args...);
	using sfptr = R(*)(void*, Args...);
private:
	sfptr stateful;
	union {
		void* state;
		fptr stateless;
	};
public:
	constexpr function_view(fptr f) noexcept : stateful(nullptr), stateless(f) {}
	constexpr function_view(void* s, sfptr f) noexcept : stateful(f), state(s) {}
	constexpr function_view() noexcept : function_view(static_cast<fptr>(nullptr)) {}
	constexpr function_view(std::nullptr_t) noexcept : function_view(static_cast<fptr>(nullptr)) {}
	template <typename C>
	constexpr function_view(C&& c) requires (
		!is_from_method_v<std::remove_cvref_t<C>> &&
		requires(C c){static_cast<fptr>(c);} &&
		!std::same_as<std::remove_cvref_t<C>, function_view<R(Args...)>>
	) : function_view(static_cast<fptr>(c)) {}
	template <typename C>
	constexpr function_view(C& c) noexcept requires (
		!is_from_method_v<std::remove_cvref_t<C>> &&
		!requires(C c){static_cast<fptr>(c);} &&
		!std::same_as<std::remove_cvref_t<C>, function_view<R(Args...)>>
	) :
		function_view(
			const_cast<typename std::remove_const_t<C>*>(&c),
			[](void* obj, Args...args) -> R {
				using wrp = operator_wrapper<C&>;
				return wrp::function(*static_cast<C*>(obj), std::forward<Args>(args)...);
			}
		)
	{}
	template <typename C>
	constexpr function_view(C&& c) requires (
		!is_from_method_v<std::remove_cvref_t<C>> &&
		!requires(C c){static_cast<fptr>(c);} &&
		!std::same_as<std::remove_cvref_t<C>, function_view<R(Args...)>>
	) = delete;
	template <auto m>
	constexpr function_view(from_method<m> c) requires(
		std::same_as<
			remove_method_cvref_t<decltype(m)>,
			R(class_from_member_ptr_t<decltype(m)>::*)(Args...)
		>
	) :
		function_view(
			const_cast<void*>(static_cast<const volatile void*>(c.get())),
			[](void* obj, Args...args) -> R {
				return static_wrapper<m>::function(
					*static_cast<typename from_method<m>::cls_type*>(obj),
					std::forward<Args>(args)...
				);
			}
		)
	{}
	R operator()(Args ... args) const {
		if (stateful) {
			return stateful(state, std::forward<Args>(args)...);
		} else {
			return stateless(std::forward<Args>(args)...);
		}
	}
	bool is_stateful() const noexcept { return stateful; }
	sfptr get_stateful_function() const noexcept { return stateful; }
	void* get_state_object_ptr() const noexcept { return state; }
	fptr get_stateless_function() const noexcept { return stateless; }
	explicit operator bool() const noexcept {
		return stateful || stateless;
	}
};

} // namespace dse::util

#endif // FUNCTIONAL_H

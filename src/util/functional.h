#ifndef FUNCTIONAL_H
#define FUNCTIONAL_H

#include <utility>
#include <type_traits>

namespace dse::util {

template <auto m>
struct StaticMemFn;

namespace func_impl {

template <typename T>
struct StaticMemFnAddRef {
    using Type = T&;
};

template <typename T>
struct StaticMemFnAddRef<T&> {
    using Type = T&;
};

template <typename T>
struct StaticMemFnAddRef<T&&> {
    using Type = T&&;
};

template <typename T>
using StaticMemFnAddRefT = typename StaticMemFnAddRef<T>::Type;

template <typename Sig, typename Cal>
struct ExtractCallable;

template <typename Ret, typename ... Args, typename Cal>
struct ExtractCallable<Ret(Args...), Cal> {
    static Ret Function(void* callable, Args ... args)
    noexcept(std::is_nothrow_invocable_v<Cal, Args...>)
    {
        return static_cast<StaticMemFnAddRefT<Cal>>(*(std::remove_reference_t<Cal>*)(callable))(args...);
    }
};

template <typename Sig, auto m>
struct ExtractMember;

#define GENERATE(cvr, nxcpt)\
template <typename Ret, typename Obj, typename ... Args, Ret(Obj::*m)(Args...) cvr nxcpt>\
struct ExtractMember<Ret(Args...), m> {\
    static Ret Function(void* obj, Args ... args) nxcpt\
    {\
        return (static_cast<StaticMemFnAddRefT<Obj cvr>>(*static_cast<Obj*>(obj)).*m)(args...);\
    }\
};

GENERATE(,)
GENERATE(const,)
GENERATE(volatile,)
GENERATE(const volatile,)
GENERATE(&,)
GENERATE(const&,)
GENERATE(volatile&,)
GENERATE(const volatile&,)
GENERATE(&&,)
GENERATE(const&&,)
GENERATE(volatile&&,)
GENERATE(const volatile&&,)
GENERATE(, noexcept)
GENERATE(const, noexcept)
GENERATE(volatile, noexcept)
GENERATE(const volatile, noexcept)
GENERATE(&, noexcept)
GENERATE(const&, noexcept)
GENERATE(volatile&, noexcept)
GENERATE(const volatile&, noexcept)
GENERATE(&&, noexcept)
GENERATE(const&&, noexcept)
GENERATE(volatile&&, noexcept)
GENERATE(const volatile&&, noexcept)
#undef GENERATE

template <typename T>
struct IsMemFnToFunc : std::false_type {};

template <auto m>
struct IsMemFnToFunc<StaticMemFn<m>> : std::true_type {};

template <typename T>
constexpr auto& IsMemFnToFuncV = IsMemFnToFunc<T>::value;

}

#define GENERATE(cvr, nxcpt)\
template <typename Ret, typename Obj, typename ... Args, Ret (Obj::*m)(Args...) cvr nxcpt>\
struct StaticMemFn<m> {\
    using ObjType = Obj cvr;\
    ObjType& self;\
    StaticMemFn(func_impl::StaticMemFnAddRefT<ObjType> obj) : self(obj) {}\
    Ret operator ()(Args ... args) const nxcpt {\
        return (static_cast<func_impl::StaticMemFnAddRefT<ObjType>>(self).*m)(args...);\
    }\
};

GENERATE(,)
GENERATE(const,)
GENERATE(volatile,)
GENERATE(const volatile,)
GENERATE(&,)
GENERATE(const&,)
GENERATE(volatile&,)
GENERATE(const volatile&,)
GENERATE(&&,)
GENERATE(const&&,)
GENERATE(volatile&&,)
GENERATE(const volatile&&,)
GENERATE(, noexcept)
GENERATE(const, noexcept)
GENERATE(volatile, noexcept)
GENERATE(const volatile, noexcept)
GENERATE(&, noexcept)
GENERATE(const&, noexcept)
GENERATE(volatile&, noexcept)
GENERATE(const volatile&, noexcept)
GENERATE(&&, noexcept)
GENERATE(const&&, noexcept)
GENERATE(volatile&&, noexcept)
GENERATE(const volatile&&, noexcept)
#undef GENERATE

enum CVRef {
	NoneV,
	ConstV,
	VolatileV,
	LRefV = 4,
	RRefV = 8
};

template <typename ... Args, typename A>
constexpr auto select(A(*p)(Args...))
-> A(*)(Args...)
{ return p; }
template <typename A>
constexpr auto select(A(*p)())
-> A(*)()
{ return p; }

template <typename ... Args, typename A, typename B>
constexpr auto select(A(B::*p)(Args...))
-> A(B::*)(Args...)
{ return p; }
template <typename A, typename B>
constexpr auto select(A(B::*p)())
-> A(B::*)()
{ return p; }

#define GENERATE_VARIANT(v, cv) \
template <int qualifier, typename ... Args, typename A, typename B>\
requires(qualifier == (v))\
constexpr auto select(A(B::*p)(Args...)cv)\
-> A(B::*)(Args...)cv\
{ return p; }\
template <int qualifier, typename A, typename B>\
requires(qualifier == (v))\
constexpr auto select(A(B::*p)()cv)\
-> A(B::*)()cv\
{ return p; }

GENERATE_VARIANT(NoneV,)
GENERATE_VARIANT(ConstV, const)
GENERATE_VARIANT(VolatileV, volatile)
GENERATE_VARIANT(ConstV | VolatileV, const volatile)
GENERATE_VARIANT(LRefV, &)
GENERATE_VARIANT(LRefV | ConstV, const&)
GENERATE_VARIANT(LRefV | VolatileV, volatile&)
GENERATE_VARIANT(LRefV | ConstV | VolatileV, const volatile&)
GENERATE_VARIANT(RRefV, &&)
GENERATE_VARIANT(RRefV | ConstV, const&&)
GENERATE_VARIANT(RRefV | VolatileV, volatile&&)
GENERATE_VARIANT(RRefV | ConstV | VolatileV, const volatile&&)
#undef GENERATE_VARIANT

template <typename F>
class FunctionPtr;

template <typename R, typename ... Args>
class FunctionPtr<R(Args...)> {
public:
	using fn = R(Args...);
	using fptr = fn*;
	using sfptr = R(*)(void*, Args...);
private:
	sfptr stateful;
	union {
		void* state;
		fptr stateless;
	};
public:
	constexpr FunctionPtr(fptr f) noexcept : stateful(nullptr), stateless(f)
	{}
	constexpr FunctionPtr(void* s, sfptr f) noexcept : stateful(f), state(s)
	{}
	constexpr FunctionPtr() noexcept : FunctionPtr(static_cast<fptr>(nullptr))
	{}
	constexpr FunctionPtr(std::nullptr_t) noexcept : FunctionPtr(static_cast<fptr>(nullptr))
	{}
	template <auto m>
	requires (
		requires (void* ptr, Args...args) {
			{ func_impl::ExtractMember<fn, m>::Function(ptr, args...) } -> std::same_as<R>;
		}
	)
	constexpr FunctionPtr(const StaticMemFn<m>& memfn) :
		stateful(func_impl::ExtractMember<fn, m>::Function),
		state(std::addressof(memfn.self))
	{}
	template <std::invocable<Args...> C>
	requires (
		requires (C c, Args...args) {
			{ c(args...) } -> std::same_as<R>;
		} &&
		!std::is_function_v<std::remove_cvref_t<C>> &&
		!func_impl::IsMemFnToFuncV<std::remove_cvref_t<C>> &&
		!std::same_as<std::remove_cvref_t<C>, FunctionPtr<R(Args...)>>
	)
	constexpr FunctionPtr(C&& callable) :
		stateful(func_impl::ExtractCallable<fn, C>::Function),
		state((void*)std::addressof(callable))
	{}
	R operator()(Args ... args) const
	{
		if (stateful) {
			return stateful(state, std::forward<Args>(args)...);
		} else {
			return stateless(std::forward<Args>(args)...);
		}
	}
	bool is_stateful() const noexcept
	{ return stateful; }
	sfptr get_stateful_function() const noexcept
	{ return stateful; }
	void* get_state_object_ptr() const noexcept
	{ return state; }
	fptr get_stateless_function() const noexcept
	{ return stateless; }
	explicit operator bool() const noexcept
	{
		return stateful || stateless;
	}
};

} // namespace dse::util

#endif // FUNCTIONAL_H

#ifndef DSE_UTIL_FUNCTIONAL_H
#define DSE_UTIL_FUNCTIONAL_H

#include <functional>
#include <utility>
#include <type_traits>

namespace dse::util {

/*template <auto m, typename = decltype(m)>
struct StaticMemFn;*/

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

/*template <typename Sig, auto m, typename Cast = void>
struct ExtractMember;

#define GENERATE(cvr, nxcpt)\
template <typename Ret, typename Obj, typename ... Args, Ret(Obj::*m)(Args...) cvr nxcpt, typename Cast>\
struct ExtractMember<Ret(Args...) nxcpt, m, Cast> {\
    static Ret Function(Cast* obj, Args ... args) nxcpt\
    {\
        return (static_cast<StaticMemFnAddRefT<Obj cvr>>(*static_cast<Obj*>(obj)).*m)(args...);\
        return std::invoke\
            (m, static_cast<StaticMemFnAddRefT<Obj cvr>>(*static_cast<Obj*>(obj)), args...);\
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
#undef GENERATE*/

template <auto m>
struct ToExplicitThis;

#define GENERATE(cv, nxcpt)\
template <\
    typename Cls, typename Ret,\
    typename ... Args,\
    Ret(Cls::*m)(Args...) cv nxcpt\
>\
struct ToExplicitThis<m> {\
    static Ret Function(Cls cv& obj, Args ... args) nxcpt\
    {\
        return std::invoke(m, std::forward<Cls cv&>(obj), std::forward<Args>(args)...);\
    }\
    static Ret Function(Cls cv&& obj, Args ... args) nxcpt\
    {\
        return std::invoke(m, std::forward<Cls cv&&>(obj), std::forward<Args>(args)...);\
    }\
};
GENERATE(,)
GENERATE(const,)
GENERATE(volatile,)
GENERATE(const volatile,)
GENERATE(, noexcept)
GENERATE(const, noexcept)
GENERATE(volatile, noexcept)
GENERATE(const volatile, noexcept)
#undef GENERATE

#define GENERATE(cvr, nxcpt)\
template <\
    typename Cls, typename Ret,\
    typename ... Args,\
    Ret(Cls::*m)(Args...) cvr nxcpt\
>\
struct ToExplicitThis<m> {\
    static Ret Function(Cls cvr obj, Args ... args) nxcpt\
    {\
        return std::invoke(m, std::forward<Cls cvr>(obj), std::forward<Args>(args)...);\
    }\
};

GENERATE(&,)
GENERATE(const&,)
GENERATE(volatile&,)
GENERATE(const volatile&,)
GENERATE(&, noexcept)
GENERATE(const&, noexcept)
GENERATE(volatile&, noexcept)
GENERATE(const volatile&, noexcept)
GENERATE(&&,)
GENERATE(const&&,)
GENERATE(volatile&&,)
GENERATE(const volatile&&,)
GENERATE(&&, noexcept)
GENERATE(const&&, noexcept)
GENERATE(volatile&&, noexcept)
GENERATE(const volatile&&, noexcept)
#undef GENERATE

template <typename T, typename P = void>
T&& FromPtr(P* ptr)
{
    auto p0 = const_cast<std::remove_cvref_t<P>*>(ptr);
    auto p = static_cast<std::remove_cvref_t<T>*>(p0);
    auto cvp = const_cast<std::remove_reference_t<T>*>(p);
    return static_cast<T&&>(*cvp);
}

template <typename P = void, typename T>
P* ToPtr(T&& ref)
{
    auto cvp = std::addressof(ref);
    auto p = const_cast<std::remove_cvref_t<T>*>(cvp);
    return static_cast<std::remove_cvref_t<P>*>(p);
}

/*template <auto f>
struct EraseThisType;

#define GENERATE(nxcpt)\
template <typename Cls, typename Ret, typename ... Args, Ret(*f)(Cls, Args...) nxcpt>\
requires(std::is_reference_v<Cls>)\
struct EraseThisType<f> {\
    static Ret Function(void* ptr, Args ... args) nxcpt {\
        return f(FromPtr<Cls>(ptr), std::forward<Args>(args)...);\
    }\
};

GENERATE()
GENERATE(noexcept)
#undef GENERATE*/

template <auto f, typename = void>
struct ReplaceThisTypeByPtr;

#define GENERATE(nxcpt)\
template <typename Repl, typename Cls, typename Ret, typename ... Args, Ret(*f)(Cls, Args...) nxcpt>\
requires(std::is_reference_v<Cls>)\
struct ReplaceThisTypeByPtr<f, Repl> {\
    static Ret Function(Repl* ptr, Args ... args) nxcpt {\
        return f(FromPtr<Cls>(ptr), std::forward<Args>(args)...);\
    }\
};

GENERATE()
GENERATE(noexcept)
#undef GENERATE

template <auto f>
using EraseExplicitThisType = ReplaceThisTypeByPtr<ToExplicitThis<f>::Function>;

/*template <typename T>
struct IsMemFnToFunc : std::false_type {};

template <auto m>
struct IsMemFnToFunc<StaticMemFn<m>> : std::true_type {};

template <typename T>
constexpr auto& IsMemFnToFuncV = IsMemFnToFunc<T>::value;*/

template <typename T>
struct IsUnqualifiedFunction : std::false_type {};

template <typename Ret, typename ... Args>
struct IsUnqualifiedFunction<Ret(Args...)> : std::true_type {};

template <typename Ret, typename ... Args>
struct IsUnqualifiedFunction<Ret(Args...) noexcept> : std::true_type {};

template <typename T>
concept UnqualifiedFunction = IsUnqualifiedFunction<T>::value;

template <typename T, typename NewArg = void*>
struct PrependArg;

template <typename Ret, typename ... Args, typename NewArg>
struct PrependArg<Ret(Args...), NewArg> {
    using Type = Ret(NewArg, Args...);
};

template <typename Ret, typename ... Args, typename NewArg>
struct PrependArg<Ret(Args...) noexcept, NewArg> {
    using Type = Ret(NewArg, Args...) noexcept;
};

template <typename T, typename NewArg = void*>
using PrependArgT = typename PrependArg<T, NewArg>::Type;

template <typename T>
struct FunctionPtrBase;

template <typename R, typename ... Args>
struct FunctionPtrBase<R(Args...)> {
public:
	R operator()(Args&& ... args) const;
};

template <typename R, typename ... Args>
struct FunctionPtrBase<R(Args...) noexcept> {
public:
	R operator()(Args&& ... args) const noexcept;
};

}

template <func_impl::UnqualifiedFunction F>
struct FunctionPtr;

/*#define GENERATE(cvr, nxcpt)\
template <typename Ret, typename Obj, typename ... Args, Ret (Obj::*m)(Args...) cvr nxcpt>\
struct StaticMemFn<m, Ret (Obj::*)(Args...) nxcpt> {\
    using ObjType = Obj cvr;\
    friend class FunctionPtr<Ret(Args...) nxcpt>;\
private:\
    ObjType& self;\
public:\
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
#undef GENERATE*/

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

struct ThisTag {};

#define GENERATE_VARIANT(cv) \
template <std::same_as<ThisTag cv>, typename ... Args, typename A, typename B>\
constexpr auto select(A(B::*p)(Args...)cv)\
-> A(B::*)(Args...)cv\
{ return p; }\
template <std::same_as<ThisTag cv>, typename A, typename B>\
constexpr auto select(A(B::*p)()cv)\
-> A(B::*)()cv\
{ return p; }

GENERATE_VARIANT()
GENERATE_VARIANT(const)
GENERATE_VARIANT(volatile)
GENERATE_VARIANT(const volatile)
GENERATE_VARIANT(&)
GENERATE_VARIANT(const&)
GENERATE_VARIANT(volatile&)
GENERATE_VARIANT(const volatile&)
GENERATE_VARIANT(&&)
GENERATE_VARIANT(const&&)
GENERATE_VARIANT(volatile&&)
GENERATE_VARIANT(const volatile&&)
#undef GENERATE_VARIANT

template <typename Cls, typename Fn>
struct ApplyCVRefFromThis;

#define GENERATE(cvr, nxcpt)\
template <typename Cls, typename Ret, typename ... Args>\
struct ApplyCVRefFromThis<Cls cvr, Ret(Args...) nxcpt> {\
	using Type = Ret(Args...) cvr nxcpt;\
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

template <typename Cls, typename Fn>
using ApplyCVRefFromThisT = ApplyCVRefFromThis<Cls, Fn>::Type;

template <auto fn>
struct FnTag {};

template <auto fn>
FnTag<fn> fnTag = {};

template <typename Cls, auto m, typename Func>
struct MemberInvokableWith;

template <typename Cls, auto m, typename Ret, typename ... Args>
struct MemberInvokableWith<Cls, m, Ret(Args...)> {
	static constexpr
	bool Value = requires (Cls obj, Args ... args) {
		{ std::invoke(m, std::forward<Cls>(obj), std::forward<Args>(args)...) } -> std::convertible_to<Ret>;
	};
};

template <typename Cls, auto m, typename Func>
constexpr bool MemberInvokableWithV = MemberInvokableWith<Cls, m, Func>::Value;

template <func_impl::UnqualifiedFunction T>
struct FunctionPtr : public func_impl::FunctionPtrBase<T> {
	using fn = T;
	using fptr = fn*;
	using sfn = func_impl::PrependArgT<T>;
	using sfptr = sfn*;
	friend struct func_impl::FunctionPtrBase<T>;
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
	template <typename Cls, func_impl::PrependArgT<fn, Cls&>* f>
	constexpr FunctionPtr(Cls& obj, FnTag<f>) noexcept
	:
		stateful(func_impl::ReplaceThisTypeByPtr<f>::Function),
		state(func_impl::ToPtr(obj))
	{}
	constexpr FunctionPtr() noexcept :
		FunctionPtr(static_cast<fptr>(nullptr))
	{}
	constexpr FunctionPtr(std::nullptr_t) noexcept :
		FunctionPtr()
	{}
	template <typename C, ApplyCVRefFromThisT<C, fn> std::remove_cvref_t<C>::* m>
	constexpr FunctionPtr(C& obj, FnTag<m>) noexcept :
		stateful(func_impl::ReplaceThisTypeByPtr<static_cast<func_impl::PrependArgT<fn, C&>*>(func_impl::ToExplicitThis<m>::Function)>::Function),
		state(func_impl::ToPtr(obj))
	{}
	template <typename C, ApplyCVRefFromThisT<C&, fn> std::remove_cvref_t<C>::* m>
	constexpr FunctionPtr(C& obj, FnTag<m>) noexcept :
		stateful(func_impl::EraseExplicitThisType<m>::Function),
		state(func_impl::ToPtr(obj))
	{}
/*	template <auto m>
	requires (
		requires {
			{ func_impl::ExtractMember<fn, m>::Function } -> std::convertible_to<sfn&>;
		}
	)
	constexpr FunctionPtr(const StaticMemFn<m>& memfn) :
		stateful(func_impl::ExtractMember<fn, m>::Function),
		state(std::addressof(memfn.self))
	{}*/
	template <typename C>
	requires (
		!std::same_as<std::remove_cvref_t<C>, FunctionPtr<T>> &&
		//!func_impl::IsMemFnToFuncV<std::remove_cvref_t<C>> &&
		!std::is_function_v<std::remove_cvref_t<C>> &&
		requires {
			{ func_impl::ExtractCallable<fn, C>::Function } -> std::convertible_to<sfn&>;
		}
	)
	constexpr FunctionPtr(C&& callable) :
		stateful(func_impl::ExtractCallable<fn, C>::Function),
		state(func_impl::ToPtr(callable))
	{}
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

#define GENERATE(cvr)\
template <typename R, typename ... Args, typename C, R(C::*m)(Args...)cvr>\
FunctionPtr(std::remove_reference_t<C cvr>&, FnTag<m>) -> FunctionPtr<R(Args...)>;

GENERATE()
GENERATE(const)
GENERATE(volatile)
GENERATE(const volatile)
GENERATE(&)
GENERATE(const&)
GENERATE(volatile&)
GENERATE(const volatile&)
#undef GENERATE

namespace func_impl {

template <typename R, typename ... Args>
R FunctionPtrBase<R(Args...)>::operator()(Args&& ... args) const
{
	auto self = static_cast<const FunctionPtr<R(Args...)>*>(this);
	if (self->stateful) {
		return self->stateful(self->state, std::forward<Args>(args)...);
	} else {
		return self->stateless(args...);
	}
}

template <typename R, typename ... Args>
R FunctionPtrBase<R(Args...) noexcept>::operator()(Args&& ... args) const noexcept
{
	auto self = static_cast<const FunctionPtr<R(Args...) noexcept>*>(this);
	if (self->stateful) {
		return self->stateful(self->state, std::forward<Args>(args)...);
	} else {
		return self->stateless(args...);
	}
}

}

} // namespace dse::util

#endif // DSE_UTIL_FUNCTIONAL_H

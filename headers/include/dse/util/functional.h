#ifndef DSE_UTIL_FUNCTIONAL_H
#define DSE_UTIL_FUNCTIONAL_H

#include <functional>
#include <utility>
#include <type_traits>

namespace dse::util {

/*template <auto m, typename = decltype(m)>
struct StaticMemFn;*/

namespace function_ptr_impl {

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
        return std::invoke(m, static_cast<Cls cv&>(obj), static_cast<Args&&>(args)...);\
    }\
    static Ret Function(Cls cv&& obj, Args ... args) nxcpt\
    {\
        return std::invoke(m, static_cast<Cls cv&&>(obj), static_cast<Args&&>(args)...);\
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
        return std::invoke(m, static_cast<Cls cvr>(obj), static_cast<Args&&>(args)...);\
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

template <auto f, typename = void>
struct ReplaceThisTypeByPtr;

#define GENERATE(nxcpt)\
template <typename Repl, typename Cls, typename Ret, typename ... Args, Ret(*f)(Cls, Args...) nxcpt>\
requires(std::is_reference_v<Cls>)\
struct ReplaceThisTypeByPtr<f, Repl> {\
    static Ret Function(Repl* ptr, Args ... args) nxcpt {\
        return f(FromPtr<Cls&&>(ptr), static_cast<Args&&>(args)...);\
    }\
};

GENERATE()
GENERATE(noexcept)
#undef GENERATE

template <auto f, typename = void*>
struct PrepandArgument;

#define GENERATE(nxcpt)\
template <typename Arg0, typename Ret, typename ... Args, Ret(*f)(Args...) nxcpt>\
struct PrepandArgument<f, Arg0> {\
    static Ret Function(Arg0, Args ... args) nxcpt {\
        return f(static_cast<Args&&>(args)...);\
    }\
};

GENERATE()
GENERATE(noexcept)
#undef GENERATE

template <auto f>
using EraseExplicitThisType = ReplaceThisTypeByPtr<ToExplicitThis<f>::Function>;

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
	R operator()(Args ... args) const;
};

template <typename R, typename ... Args>
struct FunctionPtrBase<R(Args...) noexcept> {
public:
	R operator()(Args ... args) const noexcept;
};

}

template <function_ptr_impl::UnqualifiedFunction F>
struct FunctionPtr;

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
using ApplyCVRefFromThisT = typename ApplyCVRefFromThis<Cls, Fn>::Type;

template <auto fn>
struct FnTag {};

template <auto fn>
FnTag<fn> fnTag = {};

template <function_ptr_impl::UnqualifiedFunction T>
struct FunctionPtr : public function_ptr_impl::FunctionPtrBase<T> {
	using fn = T;
	using fptr = fn*;
	using sfn = function_ptr_impl::PrependArgT<T>;
	using sfptr = sfn*;
	friend struct function_ptr_impl::FunctionPtrBase<T>;
private:
	void* object;
	sfn* function;
public:
    constexpr FunctionPtr() noexcept :
        object(nullptr),
        function(nullptr)
    {}
    constexpr FunctionPtr(std::nullptr_t) noexcept :
        FunctionPtr()
    {}
    constexpr FunctionPtr(void* s, sfptr f) noexcept :
        object(s),
        function(f)
    {}
    template <fptr f>
    constexpr FunctionPtr(FnTag<f>) noexcept :
        FunctionPtr(nullptr,
        function_ptr_impl::PrepandArgument<f>::Function)
    {}
    template <typename Cls, function_ptr_impl::PrependArgT<fn, Cls&>* f>
    constexpr FunctionPtr(Cls& obj, FnTag<f>) noexcept
    :
        function(function_ptr_impl::ReplaceThisTypeByPtr<f>::Function),
        object(function_ptr_impl::ToPtr(obj))
    {}
    template <typename C, ApplyCVRefFromThisT<C, fn> std::remove_cvref_t<C>::* m>
    constexpr FunctionPtr(C& obj, FnTag<m>) noexcept :
        FunctionPtr(obj,
        fnTag<static_cast<function_ptr_impl::PrependArgT<fn, C&>*>(function_ptr_impl::ToExplicitThis<m>::Function)>)
    {}
    template <typename C, ApplyCVRefFromThisT<C&, fn> std::remove_cvref_t<C>::* m>
    constexpr FunctionPtr(C& obj, FnTag<m>) noexcept :
        FunctionPtr(obj,
        fnTag<function_ptr_impl::ToExplicitThis<m>::Function>)
    {}
    template <typename C>
    requires (
        !std::same_as<std::remove_cvref_t<C>, FunctionPtr<T>> &&
        !std::is_function_v<std::remove_cvref_t<C>> &&
        requires {
            { function_ptr_impl::ExtractCallable<fn, C>::Function } -> std::convertible_to<sfn&>;
        }
    )
    constexpr FunctionPtr(C&& callable) :
        function(function_ptr_impl::ExtractCallable<fn, C>::Function),
        object(function_ptr_impl::ToPtr(callable))
    {}
    sfptr get_function() const noexcept
    { return function; }
    void* get_object_ptr() const noexcept
    { return object; }
    explicit operator bool() const noexcept
    {
        return function;
    }
};

#define GENERATE(cv)\
template <typename C, typename R, typename ... Args, R(C::*m)(Args...)cv>\
FunctionPtr(C cv&, FnTag<m>) -> FunctionPtr<R(Args...)>;

GENERATE()
GENERATE(const)
GENERATE(volatile)
GENERATE(const volatile)
#undef GENERATE

#define GENERATE(cv)\
template <typename C, typename R, typename ... Args, R(C::*m)(Args...)cv&>\
FunctionPtr(C cv&, FnTag<m>) -> FunctionPtr<R(Args...)>;

GENERATE()
GENERATE(const)
GENERATE(volatile)
GENERATE(const volatile)
#undef GENERATE

namespace function_ptr_impl {

template <typename R, typename ... Args>
R FunctionPtrBase<R(Args...)>::operator()(Args ... args) const
{
	auto self = static_cast<const FunctionPtr<R(Args...)>*>(this);
	return self->function(self->object, static_cast<Args&&>(args)...);
}

template <typename R, typename ... Args>
R FunctionPtrBase<R(Args...) noexcept>::operator()(Args ... args) const noexcept
{
	auto self = static_cast<const FunctionPtr<R(Args...) noexcept>*>(this);
	return self->function(self->object, static_cast<Args&&>(args)...);
}

}

} // namespace dse::util

#endif // DSE_UTIL_FUNCTIONAL_H

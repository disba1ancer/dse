#ifndef DSE_UTIL_FUNCTIONAL_H
#define DSE_UTIL_FUNCTIONAL_H

#include <functional>
#include <utility>
#include <type_traits>

namespace dse::util {

#define DSE_UTIL_FUNCTIONAL_NONE

struct ThisTag {};

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

template <typename T>
struct RemovePointerEx_impl {
    using Type = T;
};

template <typename T>
struct RemovePointerEx_impl<T*> {
    using Type = T;
};

template <typename T, typename Cls>
struct RemovePointerEx_impl<T Cls::*> {
    using Type = T;
};

template <typename T>
struct RemovePointerEx : RemovePointerEx_impl<std::remove_cv_t<T>> {};

template <typename T>
using RemovePointerExT = typename RemovePointerEx<T>::Type;

template <typename T>
struct IsNoExcept;

#define GENERATE(cvr)\
template <typename Ret, typename ... Args>\
struct IsNoExcept<Ret(Args...) cvr> : std::false_type {};\
template <typename Ret, typename ... Args>\
struct IsNoExcept<Ret(Args...) cvr noexcept> : std::true_type {};

GENERATE(DSE_UTIL_FUNCTIONAL_NONE)
GENERATE(const)
GENERATE(volatile)
GENERATE(const volatile)
GENERATE(&)
GENERATE(const&)
GENERATE(volatile&)
GENERATE(const volatile&)
GENERATE(&&)
GENERATE(const&&)
GENERATE(volatile&&)
GENERATE(const volatile&&)
#undef GENERATE

template <typename T>
constexpr auto IsNoExceptV = IsNoExcept<T>::value;

template <typename F>
struct GetThisQualifiers;

#define GENERATE(cvr)\
template <typename Ret, typename ... Args>\
struct GetThisQualifiers<Ret(Args...) cvr noexcept> {\
    using Type = ThisTag cvr;\
};\
template <typename Ret, typename ... Args>\
struct GetThisQualifiers<Ret(Args...) cvr> {\
    using Type = ThisTag cvr;\
};

GENERATE(DSE_UTIL_FUNCTIONAL_NONE)
GENERATE(const)
GENERATE(volatile)
GENERATE(const volatile)
GENERATE(&)
GENERATE(const&)
GENERATE(volatile&)
GENERATE(const volatile&)
GENERATE(&&)
GENERATE(const&&)
GENERATE(volatile&&)
GENERATE(const volatile&&)
#undef GENERATE

template <typename F>
using GetThisQualifiersT = typename GetThisQualifiers<F>::Type;

template <typename F>
struct RemoveThisQualifiers;

#define GENERATE(cvr)\
template <typename Ret, typename ... Args>\
struct RemoveThisQualifiers<Ret(Args...) cvr> {\
    using Type = Ret(Args...);\
};\
template <typename Ret, typename ... Args>\
struct RemoveThisQualifiers<Ret(Args...) cvr noexcept> {\
    using Type = Ret(Args...) noexcept;\
};

GENERATE(DSE_UTIL_FUNCTIONAL_NONE)
GENERATE(const)
GENERATE(volatile)
GENERATE(const volatile)
GENERATE(&)
GENERATE(const&)
GENERATE(volatile&)
GENERATE(const volatile&)
GENERATE(&&)
GENERATE(const&&)
GENERATE(volatile&&)
GENERATE(const volatile&&)
#undef GENERATE

template <typename F>
using RemoveThisQualifiersT = typename RemoveThisQualifiers<F>::Type;

template <typename F, typename T>
struct ReplaceThisQualifiersImpl;

#define GENERATE(cvr)\
template <typename Ret, typename ... Args, typename T>\
struct ReplaceThisQualifiersImpl<Ret(Args...), T cvr> {\
    using Type = Ret(Args...) cvr;\
};\
template <typename Ret, typename ... Args, typename T>\
struct ReplaceThisQualifiersImpl<Ret(Args...) noexcept, T cvr> {\
    using Type = Ret(Args...) cvr noexcept;\
};

GENERATE(DSE_UTIL_FUNCTIONAL_NONE)
GENERATE(const)
GENERATE(volatile)
GENERATE(const volatile)
GENERATE(&)
GENERATE(const&)
GENERATE(volatile&)
GENERATE(const volatile&)
GENERATE(&&)
GENERATE(const&&)
GENERATE(volatile&&)
GENERATE(const volatile&&)
#undef GENERATE

template <typename F, typename T>
using ReplaceThisQualifiers = ReplaceThisQualifiersImpl<RemoveThisQualifiersT<F>, T>;

template <typename F, typename T>
using ReplaceThisQualifiersT = typename ReplaceThisQualifiers<F, T>::Type;

template <typename F>
struct RemoveNoExceptImpl;

template <typename Ret, typename ... Args>
struct RemoveNoExceptImpl<Ret(Args...)> {
    using Type = Ret(Args...);
};

template <typename Ret, typename ... Args>
struct RemoveNoExceptImpl<Ret(Args...) noexcept> {
    using Type = Ret(Args...);
};

template <typename F>
struct RemoveNoExcept {
    using Qual = GetThisQualifiersT<F>;
    using Pure = RemoveThisQualifiersT<F>;
    using Removed = typename RemoveNoExceptImpl<Pure>::Type;
    using Type = ReplaceThisQualifiersT<Removed, Qual>;
};

template <typename F>
using RemoveNoExceptT = typename RemoveNoExcept<F>::Type;

template <typename F>
struct AddNoExceptImpl;

template <typename Ret, typename ... Args>
struct AddNoExceptImpl<Ret(Args...)> {
    using Type = Ret(Args...) noexcept;
};

template <typename Ret, typename ... Args>
struct AddNoExceptImpl<Ret(Args...) noexcept> {
    using Type = Ret(Args...) noexcept;
};

template <typename F>
struct AddNoExcept {
    using Qual = GetThisQualifiersT<F>;
    using Pure = RemoveThisQualifiersT<F>;
    using Removed = typename AddNoExceptImpl<Pure>::Type;
    using Type = ReplaceThisQualifiersT<Removed, Qual>;
};

template <typename F>
using AddNoExceptT = typename AddNoExcept<F>::Type;

template <typename F>
struct ReturnTypeImpl;

template <typename Ret, typename ... Args>
struct ReturnTypeImpl<Ret(Args...)> {
    using Type = Ret;
};

template <typename F>
struct ReturnType {
    using Unqual = RemoveThisQualifiersT<F>;
    using Nxcpt = RemoveNoExceptT<Unqual>;
    using Type = typename ReturnTypeImpl<F>::Type;
};

template <typename F>
using ReturnTypeT = typename ReturnType<F>::Type;

template <typename F>
struct ArgsToFwdRef;

template <typename Ret, typename ... Args>
struct ArgsToFwdRef<Ret(Args...)> {
    using Type = Ret(Args&&...);
};

template <typename F>
using ArgsToFwdRefT = typename ArgsToFwdRef<F>::Type;

template <typename Sig, typename Cal>
struct ExtractCallable;

template <typename Ret, typename ... Args, typename Cal>
struct ExtractCallable<Ret(Args...), Cal> {
    static Ret Function(void* callable, Args ... args)
    noexcept(std::is_nothrow_invocable_v<Cal, Args...>)
    {
        return static_cast<StaticMemFnAddRefT<Cal>>(*(std::remove_reference_t<Cal>*)(callable))(static_cast<Args&&>(args)...);
    }
};

template <auto m, bool nxcpt>
struct ToExplicitThisImpl;

#define GENERATE(cv)\
template <\
    typename Cls, typename Ret,\
    typename ... Args,\
    bool nxcpt,\
    Ret(Cls::*m)(Args...) cv noexcept(nxcpt)\
>\
struct ToExplicitThisImpl<m, nxcpt> {\
    static Ret Function(Cls cv& obj, Args ... args) noexcept(nxcpt)\
    {\
        return std::invoke(m, static_cast<Cls cv&>(obj), static_cast<Args&&>(args)...);\
    }\
    static Ret Function(Cls cv&& obj, Args ... args) noexcept(nxcpt)\
    {\
        return std::invoke(m, static_cast<Cls cv&&>(obj), static_cast<Args&&>(args)...);\
    }\
};
GENERATE(DSE_UTIL_FUNCTIONAL_NONE)
GENERATE(const)
GENERATE(volatile)
GENERATE(const volatile)
#undef GENERATE

#define GENERATE(cvr)\
template <\
    typename Cls, typename Ret,\
    typename ... Args,\
    bool nxcpt,\
    Ret(Cls::*m)(Args...) cvr noexcept(nxcpt)\
>\
struct ToExplicitThisImpl<m, nxcpt> {\
    static Ret Function(Cls cvr obj, Args ... args) noexcept(nxcpt)\
    {\
        return std::invoke(m, static_cast<Cls cvr>(obj), static_cast<Args&&>(args)...);\
    }\
};

GENERATE(&)
GENERATE(const&)
GENERATE(volatile&)
GENERATE(const volatile&)
GENERATE(&&)
GENERATE(const&&)
GENERATE(volatile&&)
GENERATE(const volatile&&)
#undef GENERATE

template <auto m>
using ToExplicitThis = ToExplicitThisImpl<m, IsNoExceptV<RemovePointerExT<decltype(m)>>>;

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

template <typename F, F, typename = void>
struct ReplaceThisTypeByPtrImpl;

#define GENERATE(nxcpt)\
template <typename Repl, typename Cls, typename Ret, typename ... Args, Ret(*f)(Cls, Args...) nxcpt>\
requires(std::is_reference_v<Cls>)\
struct ReplaceThisTypeByPtrImpl<Ret(*)(Cls, Args...) nxcpt, f, Repl> {\
    static Ret Function(Repl* ptr, Args ... args) nxcpt {\
        return f(FromPtr<Cls&&>(ptr), static_cast<Args&&>(args)...);\
    }\
};

GENERATE(DSE_UTIL_FUNCTIONAL_NONE)
GENERATE(noexcept)
#undef GENERATE

template <auto f, typename Repl = void>
using ReplaceThisTypeByPtr = ReplaceThisTypeByPtrImpl<decltype(f), f, Repl>;

template <typename F, F, typename>
struct PrepandArgumentImpl;

#define GENERATE(nxcpt)\
template <typename Ret, typename ... Args, Ret(*f)(Args...) nxcpt, typename Arg0>\
struct PrepandArgumentImpl<Ret(*)(Args...) nxcpt, f, Arg0> {\
    static Ret Function(Arg0, Args ... args) nxcpt {\
        return f(static_cast<Args&&>(args)...);\
    }\
};

GENERATE(DSE_UTIL_FUNCTIONAL_NONE)
GENERATE(noexcept)
#undef GENERATE

template <auto f, typename Arg0 = void*>
using PrepandArgument = PrepandArgumentImpl<decltype(f), f, Arg0>;

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

template <typename Cls, typename Fn>
struct ApplyCVRefFromThis;

#define GENERATE(cvr)\
template <typename Cls, typename Ret, typename ... Args>\
struct ApplyCVRefFromThis<Cls cvr, Ret(Args...)> {\
	using Type = Ret(Args...) cvr;\
};\
template <typename Cls, typename Ret, typename ... Args>\
struct ApplyCVRefFromThis<Cls cvr, Ret(Args...) noexcept> {\
	using Type = Ret(Args...) cvr noexcept;\
};

GENERATE(DSE_UTIL_FUNCTIONAL_NONE)
GENERATE(const)
GENERATE(volatile)
GENERATE(const volatile)
GENERATE(&)
GENERATE(const&)
GENERATE(volatile&)
GENERATE(const volatile&)
GENERATE(&&)
GENERATE(const&&)
GENERATE(volatile&&)
GENERATE(const volatile&&)
#undef GENERATE

template <typename Cls, typename Fn>
using ApplyCVRefFromThisT = typename ApplyCVRefFromThis<Cls, Fn>::Type;

/*template <typename T>
struct FunctionPtrBase;

template <typename Ret, typename ... Args>
struct FunctionPtrBase<Ret(Args...)> {
    Ret operator()(Args...) const;
};

template <typename Ret, typename ... Args>
struct FunctionPtrBase<Ret(Args...) noexcept> {
    Ret operator()(Args...) const noexcept;
};*/
template <typename F, typename Cls>
struct SelectHelper
{
    using UnqualifiedType = function_ptr_impl::RemoveThisQualifiersT<F>;
    using Type = function_ptr_impl::PrependArgT<UnqualifiedType, Cls>;
};

template <typename F, typename Cls>
using SelectHelperT = typename SelectHelper<F, Cls>::Type;

template <typename F, typename Cls>
constexpr auto InternalSelect(SelectHelperT<F, Cls>* p)
-> SelectHelperT<F, Cls>*
{ return p; }

}

template <typename ... Args, typename A>
constexpr auto select(A(*p)(Args...))
-> A(*)(Args...)
{ return p; }
template <typename A>
constexpr auto select(A(*p)())
-> A(*)()
{ return p; }
template <typename ... Args, typename A>
constexpr auto select(A(*p)(Args...) noexcept)
-> A(*)(Args...) noexcept
{ return p; }
template <typename A>
constexpr auto select(A(*p)() noexcept)
-> A(*)() noexcept
{ return p; }

template <typename ... Args, typename A, typename B>
constexpr auto select(A(B::*p)(Args...))
-> A(B::*)(Args...)
{ return p; }
template <typename A, typename B>
constexpr auto select(A(B::*p)())
-> A(B::*)()
{ return p; }
template <typename ... Args, typename A, typename B>
constexpr auto select(A(B::*p)(Args...) noexcept)
-> A(B::*)(Args...) noexcept
{ return p; }
template <typename A, typename B>
constexpr auto select(A(B::*p)() noexcept)
-> A(B::*)() noexcept
{ return p; }

#define GENERATE_VARIANT(cv) \
template <std::same_as<ThisTag cv>, typename ... Args, typename A, typename B>\
constexpr auto select(A(B::*p)(Args...)cv)\
-> A(B::*)(Args...)cv\
{ return p; }\
template <std::same_as<ThisTag cv>, typename A, typename B>\
constexpr auto select(A(B::*p)()cv)\
-> A(B::*)()cv\
{ return p; }\
template <std::same_as<ThisTag cv>, typename ... Args, typename A, typename B>\
constexpr auto select(A(B::*p)(Args...)cv noexcept)\
-> A(B::*)(Args...)cv noexcept\
{ return p; }\
template <std::same_as<ThisTag cv>, typename A, typename B>\
constexpr auto select(A(B::*p)()cv noexcept)\
-> A(B::*)()cv noexcept\
{ return p; }

GENERATE_VARIANT(DSE_UTIL_FUNCTIONAL_NONE)
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

template <auto fn>
struct FnTag {};

template <auto fn>
FnTag<fn> fnTag = {};

template <function_ptr_impl::UnqualifiedFunction T>
struct FunctionPtr {
	using fn = T;
	using fnnx = function_ptr_impl::AddNoExceptT<T>;
	using fptr = fn *;
	using sfn = function_ptr_impl::PrependArgT<fn>;
	using sfptr = sfn*;
	using fnx = function_ptr_impl::RemoveNoExceptT<T>;
	using Ret = function_ptr_impl::ReturnTypeT<fnx>;
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
    template <typename F, F* f>
    requires (
        std::same_as<F, fn> ||
        std::same_as<F, fnnx>
    )
    constexpr FunctionPtr(FnTag<f>) noexcept :
        FunctionPtr(nullptr,
        function_ptr_impl::PrepandArgument<f>::Function)
    {}
    template <typename Cls, typename F, F *f>
    requires (
        std::same_as<F, function_ptr_impl::PrependArgT<fn, Cls&&>> ||
        std::same_as<F, function_ptr_impl::PrependArgT<fnnx, Cls&&>>
    )
    constexpr FunctionPtr(Cls&& obj, FnTag<f>) noexcept
    :
        FunctionPtr(function_ptr_impl::ToPtr(obj),
        function_ptr_impl::ReplaceThisTypeByPtr<f>::Function)
    {}
    template <typename Cls, typename F, F std::remove_cvref_t<Cls>::*m>
    requires (
        std::same_as<function_ptr_impl::RemoveThisQualifiersT<F>, fn> ||
        std::same_as<function_ptr_impl::RemoveThisQualifiersT<F>, fnnx>
    )
    constexpr FunctionPtr(Cls&& obj, FnTag<m>) noexcept :
        FunctionPtr(static_cast<Cls&&>(obj),
        fnTag<function_ptr_impl::InternalSelect<F, Cls&&>(function_ptr_impl::ToExplicitThis<m>::Function)>)
    {}
    template <typename C>
    requires (
        !std::same_as<std::remove_cvref_t<C>, FunctionPtr<fn>> &&
        !std::same_as<std::remove_cvref_t<C>, FunctionPtr<fnnx>> &&
        !std::is_function_v<std::remove_cvref_t<C>> &&
        requires {
            { function_ptr_impl::ExtractCallable<fn, C>::Function } -> std::convertible_to<sfn&>;
        }
    )
    constexpr FunctionPtr(C&& callable) :
        object(function_ptr_impl::ToPtr(callable)),
        function(function_ptr_impl::ExtractCallable<fn, C>::Function)
    {}
    template <typename ... Args>
    requires (
        std::invocable<fn, Args...>
    )
    Ret operator()(Args&& ... args) const noexcept(function_ptr_impl::IsNoExceptV<fn>)
    {
        return function(object, std::forward<Args>(args)...);
    }
    constexpr operator FunctionPtr<fnx>()
    {
        return { object, function };
    }
    sfptr GetFunction() const noexcept
    { return function; }
    void* GetObjectPtr() const noexcept
    { return object; }
    explicit operator bool() const noexcept
    {
        return function;
    }
};

template<function_ptr_impl::UnqualifiedFunction F, F *f>
FunctionPtr(FnTag<f>) -> FunctionPtr<F>;

template<typename Ret, typename Cls, typename ... Args, Ret(*f)(std::type_identity_t<Cls&&>, Args...)>
FunctionPtr(Cls&&, FnTag<f>) -> FunctionPtr<Ret(Args...)>;
template<typename Ret, typename Cls, typename ... Args, Ret(*f)(std::type_identity_t<Cls&&>, Args...) noexcept>
FunctionPtr(Cls&&, FnTag<f>) -> FunctionPtr<Ret(Args...) noexcept>;

template<typename Cls, typename F, F std::remove_cvref_t<Cls>::*f>
FunctionPtr(Cls&&, FnTag<f>) -> FunctionPtr<function_ptr_impl::RemoveThisQualifiersT<F>>;

} // namespace dse::util

#endif // DSE_UTIL_FUNCTIONAL_H

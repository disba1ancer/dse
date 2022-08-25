#ifndef TAGINVOKE_H
#define TAGINVOKE_H

#include <utility>

namespace dse::util {

namespace impl {

template <typename T1, template <typename...> typename T2>
struct InstanceOf : std::false_type {};

template <template <typename...> typename T, typename ... Args>
struct InstanceOf<T<Args...>, T> : std::true_type {};

template <typename T1, template <typename...> typename T2>
constexpr auto InstanceOfV = InstanceOf<T1, T2>::value;

template <typename T>
struct tag_type {};

template <typename T>
requires(
        !InstanceOfV<T, tag_type>
)
using TagTypeT = tag_type<T>;

}

//template <typename Cpo, typename ... Args>
//requires (!impl::InstanceOfV<Cpo, impl::tag_type>)
//auto dse_TagInvoke(const Cpo&, Args&& ... args)
//-> decltype(dse_TagInvoke(impl::TagTypeT<Cpo>{}, std::forward<Args>(args)...))
//{
//    return dse_TagInvoke(impl::TagTypeT<Cpo>{}, std::forward<Args>(args)...);
//}

inline namespace tag_invoke_ns {

inline constexpr struct dse_TagInvokeT {
    template <typename Cpo, typename ... Args>
    auto operator()(Cpo cpo, Args&& ... args) const
    -> decltype(dse_TagInvoke(cpo, std::forward<Args>(args)...))
    {
        return dse_TagInvoke(cpo, std::forward<Args>(args)...);
    }
} dse_TagInvoke;

}

template <auto& t>
struct Tag {
    using Type = std::decay_t<decltype(t)>;
};

template <auto& t>
using TagT = typename Tag<t>::Type;

template <typename Tag, typename ... Args>
using TagInvokeResultT = std::invoke_result_t<decltype(dse_TagInvoke), Tag, Args...>;

template <typename Tag, typename ... Args>
struct TagInvokeResult {
    using Type = TagInvokeResultT<Tag, Args...>;
};

}

#endif // TAGINVOKE_H

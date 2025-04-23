#ifndef DSE_UTIL_LOOPHOLE_H
#define DSE_UTIL_LOOPHOLE_H

#include <tuple>

namespace dse::util {

namespace loophole_impl {

template <typename T>
struct ct_map_load_t
{
    friend constexpr auto ct_map_load(ct_map_load_t<T>);
};

template <typename T, typename V>
struct ct_map_store_type
{
    static V decltype_helper();
    friend constexpr auto ct_map_load(ct_map_load_t<T>)
    {
        return decltype_helper();
    }
};

template <typename T, auto v>
struct ct_map_store_value
{
    friend constexpr auto ct_map_load(ct_map_load_t<T>)
    {
        return v;
    }
};

template <typename T, int i>
struct counter_tag {};

} // namespace loophole_impl

using loophole_impl::ct_map_store_type;
using loophole_impl::ct_map_store_value;

template <typename T, auto l = []{}>
requires requires(decltype(l)) {
    ct_map_load(loophole_impl::ct_map_load_t<T>{});
}
struct ct_map
{
    using type = decltype(ct_map_load(loophole_impl::ct_map_load_t<T>{}));
    static constexpr auto value()
    {
        return ct_map_load(loophole_impl::ct_map_load_t<T>{});
    }
};

template <typename T, auto l = []{}>
using ct_map_t = typename ct_map<T, l>::type;

template <typename T, auto l = []{}>
constexpr auto ct_map_v = ct_map<T, l>::value();

template <typename T, int i = 0, auto l = []{}>
consteval int counter()
{
    if constexpr (requires {typename ct_map_t<loophole_impl::counter_tag<T, i>, l>;}) {
        return counter<T, i + 1, l>();
    }
    return (std::ignore = ct_map_store_type<loophole_impl::counter_tag<T, i>, void>{}, i);
}

template <typename T>
struct cleanup_counter_tag
{};

template <typename T, int i = counter<cleanup_counter_tag<T>>()>
struct cleanup_tag
{};

template <typename T, auto fn>
requires requires(T* t) {
    fn(t);
}
constexpr auto class_defer()
{
    std::ignore = ct_map_store_value<cleanup_tag<T>, fn>{};
}

template <typename T, int i = counter<cleanup_counter_tag<T>>()>
void cleanup(T* context)
{
    if constexpr (i == 0) {
        return;
    } else {
        ct_map_v<cleanup_tag<T, i - 1>>(context);
        cleanup<T, i - 1>(context);
    }
}

} // namespace dse::util

#endif // DSE_UTIL_LOOPHOLE_H

#ifndef DSE_UTIL_PIMPL_H
#define DSE_UTIL_PIMPL_H

#include <variant>
#include <memory>
#include <exception>

namespace dse::util {

template <typename Impl>
struct impl_ptr {
    template <typename ... Args>
    impl_ptr(Args&& ... args) :
        impl(std::make_unique<Impl>(std::forward<Args>(args)...))
    {}
    auto get() -> Impl*
    {
        return impl.get();
    }
    auto get() const -> const Impl*
    {
        return impl.get();
    }
    auto operator->() -> Impl*
    {
        return get();
    }
    auto operator->() const -> const Impl*
    {
        return get();
    }
    auto operator*() -> Impl&
    {
        return *impl;
    }
    auto operator*() const -> const Impl&
    {
        return *impl;
    }
    operator Impl*()
    {
        return get();
    }
    operator const Impl*() const
    {
        return get();
    }
private:
    std::unique_ptr<Impl> impl;
};

} // namespace dse::util

#endif // DSE_UTIL_PIMPL_H

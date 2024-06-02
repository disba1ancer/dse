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
    Impl* operator->()
    {
        return *this;
    }
    auto operator->() const -> const Impl*
    {
        return *this;
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
        return impl.get();
    }
    operator const Impl*() const
    {
        return impl.get();
    }
private:
    std::unique_ptr<Impl> impl;
};

} // namespace dse::util

#endif // DSE_UTIL_PIMPL_H

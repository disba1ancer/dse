#ifndef DSE_UTIL_PIMPL_H
#define DSE_UTIL_PIMPL_H

#include <variant>
#include <memory>
#include <exception>

namespace dse::util {

template <typename Cls, typename Impl>
class pimpl {
protected:
	pimpl() noexcept : impl(std::shared_ptr<Impl>()) {}
	pimpl(std::shared_ptr<Impl> ptr) : impl(ptr) {}
	std::shared_ptr<Impl> GetImpl() {
		return impl;
	}
	std::shared_ptr<const Impl> GetImpl() const {
		return impl;
	}
public:
	explicit operator bool() const {
		auto result = GetImpl();
		return bool(result);
	}
	bool operator==(const pimpl<Cls, Impl>& second) const {
		return impl == second.impl;
	}
	bool operator==(std::nullptr_t t) const {
		return impl == t;
	}
private:
	std::shared_ptr<Impl> impl;
};

} // namespace dse::util

#endif // DSE_UTIL_PIMPL_H

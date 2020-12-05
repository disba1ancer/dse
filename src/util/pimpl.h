#ifndef PIMPL_H
#define PIMPL_H

#include <variant>
#include <memory>
#include <exception>

namespace dse::util {

template <typename Cls, typename Impl>
class pimpl {
protected:
	pimpl() noexcept : impl(std::shared_ptr<Impl>()) {}
	pimpl(std::shared_ptr<Impl> ptr) : impl(ptr) {}
	std::shared_ptr<Impl> get_impl() {
		return impl;
	}
	std::shared_ptr<const Impl> get_impl() const {
		return impl;
	}
public:
	explicit operator bool() const {
		auto result = get_impl();
		return bool(result);
	}
	bool operator==(const pimpl<Cls, Impl>& second) const {
		return impl == second.impl;
	}
	bool operator==(nullptr_t t) const {
		return impl == t;
	}
private:
	std::shared_ptr<Impl> impl;
};

}

#endif // PIMPL_H

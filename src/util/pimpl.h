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
	pimpl(std::weak_ptr<Impl> ptr) : impl(ptr) {}
	pimpl(const pimpl& other) : impl(std::weak_ptr<Impl>(other.get_weak())) {}
	pimpl(pimpl&& other) = default;
	pimpl& operator=(const pimpl& other) {
		impl = std::weak_ptr<Impl>(other.get_weak());
		return *this;
	}
	pimpl& operator=(pimpl&& other) = default;
	~pimpl() = default;
	std::shared_ptr<Impl> get_impl() {
		switch (impl.index()) {
			case 0:
				return std::get<std::shared_ptr<Impl>>(impl);
			case 1:
				return std::get<std::weak_ptr<Impl>>(impl).lock();
			default:
				throw std::runtime_error("Invalid variant");
		}
	}
	std::shared_ptr<const Impl> get_impl() const {
		switch (impl.index()) {
			case 0:
				return std::get<std::shared_ptr<Impl>>(impl);
			case 1:
				return std::get<std::weak_ptr<Impl>>(impl).lock();
			default:
				throw std::runtime_error("Invalid variant");
		}
	}
public:
	explicit operator bool() const {
		auto result = get_impl();
		return bool(result);
	}
	Cls make_strong() {
		Cls cls = static_cast<Cls&>(*this);
		cls.pimpl::impl = cls.pimpl::get_impl();
		return cls;
	}
private:
	std::weak_ptr<Impl> get_weak() const {
		switch (impl.index()) {
			case 0:
				return std::weak_ptr<Impl>(std::get<std::shared_ptr<Impl>>(impl));
			case 1:
				return std::get<std::weak_ptr<Impl>>(impl);
			default:
				throw std::runtime_error("Invalid variant");
		}
	}
	std::variant<std::shared_ptr<Impl>, std::weak_ptr<Impl>> impl;
};

}

#endif // PIMPL_H

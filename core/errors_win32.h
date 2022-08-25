#ifndef ERRORS_WIN32_H
#define ERRORS_WIN32_H

#include <swal/error.h>
#include <dse/core/errors.h>

namespace dse::core {

enum class win32_errc : DWORD {};

class win32_category : public std::error_category {
	win32_category() = default;
public:
	virtual const char* name() const noexcept override;
	virtual std::string message(int condition) const override;
	virtual std::error_condition default_error_condition([[maybe_unused]] int code) const noexcept override;
	static win32_category& instance();
};

std::error_code make_error_code(win32_errc err);

} // namespace dse::core

template <> struct std::is_error_code_enum<dse::core::win32_errc> : true_type {};

#endif // ERRORS_WIN32_H

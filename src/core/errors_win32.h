#ifndef ERRORS_WIN32_H
#define ERRORS_WIN32_H

#include <swal/error.h>
#include "errors.h"

namespace dse::core {

class win32_category : public std::error_category {
	win32_category() = default;
public:
	virtual const char* name() const noexcept override;
	virtual std::string message(int condition) const override;
	virtual std::error_condition default_error_condition([[maybe_unused]] int code) const noexcept override;
	static win32_category& instance();
};

} // namespace dse::core

#endif // ERRORS_WIN32_H

#ifndef DSE_CORE_ERRORS_H
#define DSE_CORE_ERRORS_H

#include <system_error>
#include "detail/impexp.h"

namespace dse::core {

enum class errc {
	eof = 1,
	generic
};

auto API_DSE_CORE make_error_code(errc err) noexcept -> std::error_code;
auto API_DSE_CORE make_error_condition(errc err) noexcept -> std::error_condition;

} // namespace dse::core

namespace std {

template <>
struct is_error_code_enum<::dse::core::errc> : true_type {};

}

#endif // DSE_CORE_ERRORS_H

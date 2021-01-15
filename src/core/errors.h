#ifndef ERRORS_H
#define ERRORS_H

#include <system_error>

namespace dse::core {

enum class errc {
	eof = 1,
	generic
};

auto make_error_code(errc err) noexcept -> std::error_code;
auto make_error_condition(errc err) noexcept -> std::error_condition;

}

namespace std {

template <>
struct is_error_code_enum<::dse::core::errc> : true_type {};

} // namespace dse::core

#endif // ERRORS_H

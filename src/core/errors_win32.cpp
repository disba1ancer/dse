#include "errors_win32.h"

namespace dse::core {

const char* win32_category::name() const noexcept {
	return "WIN32 error";
}

std::string win32_category::message(int condition) const {
	return swal::fromTString(swal::error::get_error_string(DWORD(condition)));
}

std::error_condition win32_category::default_error_condition([[maybe_unused]] int code) const noexcept {
	switch (DWORD(code)) {
		case ERROR_SUCCESS:
			return {};
		case ERROR_HANDLE_EOF:
			return make_error_condition(errc::eof);
		case ERROR_IO_PENDING:
			return make_error_condition(std::errc::operation_in_progress);
		default:
			return make_error_condition(errc::generic);
	}
}

win32_category& win32_category::instance() {
	static win32_category instance;
	return instance;
}

} // namespace dse::core

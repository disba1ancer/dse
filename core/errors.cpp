#include <dse/core/errors.h>

namespace dse::core {

namespace  {
	class dse_category : public std::error_category {
	public:
		virtual const char* name() const noexcept override {
			return "DSE error";
		}
		virtual std::string message([[maybe_unused]] int err) const override {
			switch (err) {
				case int(errc::generic):
					return "End of file";
				default:
					return "unknown error";
			}
		}
		static dse_category& instance() {
			static dse_category cat;
			return cat;
		}
	};
}

std::error_code make_error_code(errc err) noexcept {
	return { int(err), dse_category::instance() };
}

std::error_condition make_error_condition(errc err) noexcept {
	return { int(err), dse_category::instance() };
}

} // namespace dse::core

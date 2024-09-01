#include "status_win32.h"
#include <swal/error.h>

namespace dse::core::status::win32 {

dse::core::status::win32::StatusProvider::StatusProvider()
{}

const char8_t *StatusProvider::Name() const
{
    return u8"Win32StatusProvider";
}

const char8_t *StatusProvider::Message(int status) const {
    static std::u8string msg;
    msg = swal::u8fromTString(swal::get_error_string(-status));
    return msg.c_str();
}

}

API_DSE_CORE dse_core_Status dse_core_status_FromSystem(int code)
{
    using namespace dse::core::status;
#define GENERATE(in, out) case in: return static_cast<dse_core_Status>(Make(Code::out))
    switch (code) {
    GENERATE(ERROR_SUCCESS, Success);
    GENERATE(ERROR_HANDLE_EOF, EndOfStream);
    GENERATE(ERROR_IO_PENDING, PendingOperation);
    GENERATE(ERROR_BAD_ARGUMENTS, InvalidArgument);
    }
#undef GENERATE
    return dse_core_status_MakeSystem(-code);
}

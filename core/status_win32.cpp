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
    return "";
}

}

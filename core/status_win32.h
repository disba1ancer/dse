#ifndef STATUS_WIN32_H
#define STATUS_WIN32_H

#include <dse/core/status.h>

namespace dse::core::status::win32 {

class StatusProvider : public status_provider_base<StatusProvider> {
public:
    StatusProvider();
    auto Name() const -> const char8_t*;
    auto Message(int status) const -> const char8_t*;
};

}

namespace dse::core::status {

using SystemStatusProvider = win32::StatusProvider;

}

#endif // STATUS_WIN32_H

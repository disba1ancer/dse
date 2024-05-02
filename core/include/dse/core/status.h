#ifndef STATUS_H
#define STATUS_H

#include "detail/impexp.h"
#include <dse/util/functional.h>

typedef long dse_core_Status;

enum dse_core_status {
    dse_core_status_SourceId = 0,
    dse_core_status_SystemSourceId = 1,
    dse_core_status_ReservedSourceIds = 128
};

inline dse_core_Status dse_core_status_Make(int source, int status)
{
    constexpr auto m = -0x7FFFFFFFL;
    auto s1 = status < 0;
    auto s2 = s1 * 2 - 1;
    status = (status * s2 - s1) & 0x7FFF;
    auto resultBase = m * s1;
    auto resultSource = (source & 0xFFFF) * 0x8000L;
    return resultBase + resultSource + status;
}

inline dse_core_Status dse_core_status_MakeSystem(int status)
{
    return dse_core_status_Make(dse_core_status_SystemSourceId, status);
}

inline bool dse_core_status_IsError(dse_core_Status status)
{
    return status < 0;
}

inline unsigned dse_core_status_Source(dse_core_Status status)
{
    return ((unsigned long)status / 0x8000) & 0xFFFF;
}

inline int dse_core_status_GetCode(dse_core_Status status)
{
    auto s1 = status < 0;
    auto s2 = s1 * 2 - 1;
    int result = (unsigned long)status & 0x7FFF;
    return result * s2;
}

struct dse_core_status_IProvider;

typedef const char8_t* dse_core_status_NameFunc(const dse_core_status_IProvider* obj);
typedef const char8_t* dse_core_status_MessageFunc(const dse_core_status_IProvider* obj, int code);

struct dse_core_status_IProviderVTBL {
    dse_core_status_NameFunc *Name;
    dse_core_status_MessageFunc *Message;
};

struct dse_core_status_IProvider {
    const dse_core_status_IProviderVTBL* vtbl;
};

API_DSE_CORE unsigned dse_core_status_RegisterProvider(dse_core_status_IProvider* provider);
API_DSE_CORE dse_core_status_IProvider* dse_core_status_ProviderByID(unsigned providerID);
API_DSE_CORE void dse_core_status_UnregisterProvider(unsigned providerID);

inline const char8_t* dse_core_status_Message(dse_core_Status status)
{
    dse_core_status_IProvider* prov = dse_core_status_ProviderByID(dse_core_status_Source(status));
    return prov->vtbl->Message(prov, dse_core_status_GetCode(status));
}

inline const char8_t* dse_core_status_SourceName(dse_core_Status status)
{
    dse_core_status_IProvider* prov = dse_core_status_ProviderByID(dse_core_status_Source(status));
    return prov->vtbl->Name(prov);
}

enum dse_core_status_Code {
#define DEFINE_STATUS(name, value, message) dse_core_status_Code_##name = value,
#include "statusdef.h"
#undef DEFINE_STATUS
};

API_DSE_CORE dse_core_Status dse_core_status_FromSystem(int code);

namespace dse::core {
namespace status {

constexpr int SourceId = dse_core_status_SourceId;
constexpr int SystemSourceId = dse_core_status_SystemSourceId;
constexpr int ReservedSourceIds = dse_core_status_ReservedSourceIds;

enum class Status : dse_core_Status {};

using IProviderVTBL = dse_core_status_IProviderVTBL;
using IProvider = dse_core_status_IProvider;
using NameFunc = dse_core_status_NameFunc;
using MessageFunc = dse_core_status_MessageFunc;
template <typename T>
using NameFuncRef = const char8_t*(const T&);
template <typename T>
using NameMemFunc = const char8_t*(T::*)() const;
template <typename T>
using MessageFuncRef = const char8_t*(const T&, int);
template <typename T>
using MessageMemFunc = const char8_t*(T::*)(int) const;

template <typename T>
struct status_provider_vtbl {
    static const IProviderVTBL vtbl;
};

template <typename T>
const IProviderVTBL status_provider_vtbl<T>::vtbl = {
    util::function_ptr_impl::ReplaceThisTypeByPtr<static_cast<NameFuncRef<T>*>(util::function_ptr_impl::ToExplicitThis<static_cast<NameMemFunc<T>>(&T::Name)>::Function), const IProvider>::Function,
    util::function_ptr_impl::ReplaceThisTypeByPtr<static_cast<MessageFuncRef<T>*>(util::function_ptr_impl::ToExplicitThis<static_cast<MessageMemFunc<T>>(&T::Message)>::Function), const IProvider>::Function
};

template <typename T>
struct status_provider_base : IProvider {
    status_provider_base() {
        vtbl = &status_provider_vtbl<T>::vtbl;
    }
};

class StatusProviderAdapter {
public:
    StatusProviderAdapter(IProvider* provider) : provider(provider) {}
    const char8_t* Name() const {
        return (provider->vtbl->Name)(provider);
    }
    const char8_t* Message(int status) const {
        return provider->vtbl->Message(provider, status);
    }
private:
    IProvider* provider;
};

inline StatusProviderAdapter adapter(IProvider* provider)
{
    return { provider };
}

inline auto Make(unsigned source, int status) -> Status
{
    return static_cast<Status>(::dse_core_status_Make(source, status));
}

inline auto MakeSystem(int status) -> Status
{
    return Make(SystemSourceId, status);
}

inline bool IsError(Status status)
{
    return ::dse_core_status_IsError(static_cast<dse_core_Status>(status));
}

inline auto Source(Status status) -> unsigned
{
    return ::dse_core_status_Source(static_cast<dse_core_Status>(status));
}

inline int GetCode(Status status)
{
    return ::dse_core_status_GetCode(static_cast<dse_core_Status>(status));
}

inline auto RegisterProvider(dse_core_status_IProvider* provider) -> unsigned
{
    return ::dse_core_status_RegisterProvider(provider);
}

inline auto ProviderByID(unsigned providerID) -> dse_core_status_IProvider*
{
    return ::dse_core_status_ProviderByID(providerID);
}

inline void UnregisterProvider(unsigned providerID)
{
    return ::dse_core_status_UnregisterProvider(providerID);
}

inline auto Message(Status status) -> const char8_t*
{
    return ::dse_core_status_Message(static_cast<dse_core_Status>(status));
}

inline auto SourceName(Status status) -> const char8_t*
{
    return ::dse_core_status_SourceName(static_cast<dse_core_Status>(status));
}

inline auto FromSystem(int code) -> Status
{
    return static_cast<Status>(::dse_core_status_FromSystem(code));
}

enum class Code : int {
#define DEFINE_STATUS(name, value, message) name = value,
#include "statusdef.h"
#undef DEFINE_STATUS
};

enum class SystemCode : int {};

}

template <typename T>
struct status_traits /*{
    static auto to_status(T code) -> Status;
    static auto message(T code) -> const char8_t*;
}*/;

using Status = dse::core::status::Status;

template <>
struct status_traits<status::Code> {
    static auto to_status(status::Code code) -> Status
    {
        return status::Make(status::SourceId, int(code));
    }
    static auto message(status::Code code) -> const char8_t*
    {
        return status::Message(to_status(code));
    }
};

template <>
struct status_traits<status::SystemCode> {
    static auto to_status(status::SystemCode code) -> Status
    {
        return status::Make(status::SystemSourceId, int(code));
    }
    static auto message(status::SystemCode code) -> const char8_t*
    {
        return status::Message(to_status(code));
    }
};

namespace status {

template <typename T>
requires requires (T t) {
    { status_traits<T>::to_status(t) } -> std::same_as<Status>;
}
bool operator==(Status status, T code)
{
    return status == status_traits<T>::to_status(code);
}

/*template <typename T>
bool operator==(T code, Status status)
{
    return status == code;
}*/

template <typename T>
requires requires (T t) {
    { status_traits<T>::to_status(t) } -> std::same_as<Status>;
}
auto Make(T code) -> Status
{
    return status_traits<T>::to_status(code);
}

}

}

#endif // STATUS_H

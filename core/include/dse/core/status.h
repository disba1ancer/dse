#ifndef STATUS_H
#define STATUS_H

#include "detail/impexp.h"
#include <dse/util/functional.h>

typedef long dse_core_Status;

inline dse_core_Status dse_core_MakeStatus(unsigned source, int status, bool error)
{
    return -0x80000000LL * error + (dse_core_Status)(source & 0x7FFF) * 0x10000 + (dse_core_Status)((unsigned)status & 0xFFFF);
}

inline bool dse_core_IsError(dse_core_Status status)
{
    return status < 0;
}

inline unsigned dse_core_StatusSource(dse_core_Status status)
{
    return ((unsigned long)status / 0x10000) & 0x7FFF;
}

inline int dse_core_StatusCode(dse_core_Status status)
{
    unsigned long s2 = (unsigned long)status;
    return (short int)((long)(s2 & 0xFFFF) - (long)(s2 & 0x8000) * 2L);
}

struct dse_core_IStatusProvider;

typedef const char8_t* dse_core_NameFunc(const dse_core_IStatusProvider* obj);
typedef const char8_t* dse_core_MessageFunc(const dse_core_IStatusProvider* obj, int code);

struct dse_core_IStatusProviderVTBL {
    dse_core_NameFunc *Name;
    dse_core_MessageFunc *Message;
};

struct dse_core_IStatusProvider {
    const dse_core_IStatusProviderVTBL* vtbl;
};

API_DSE_CORE unsigned dse_core_RegisterStatusProvider(dse_core_IStatusProvider* provider);
API_DSE_CORE dse_core_IStatusProvider* dse_core_ProviderByID(unsigned providerID);
API_DSE_CORE void dse_core_UnregisterStatusProvider(unsigned providerID);

#define GENERATE_ENUM_ELEM(enum_n, name) enum_n##_##name

#define GENERATE_ENUM(name) name {\
GENERATE_ENUM_ELEM(name, Success),\
GENERATE_ENUM_ELEM(name, EndOfStream),\
GENERATE_ENUM_ELEM(name, PendingOperation),\
}

enum GENERATE_ENUM(dse_core_StatusEnum);

namespace dse::core {

#undef GENERATE_ENUM_ELEM
#define GENERATE_ENUM_ELEM(enum_n, name) name

enum class GENERATE_ENUM(StatusEnum : int);

enum class Status : dse_core_Status {};

using IStatusProviderVTBL = dse_core_IStatusProviderVTBL;
using IStatusProvider = dse_core_IStatusProvider;
using NameFunc = dse_core_NameFunc;
using MessageFunc = dse_core_MessageFunc;
template <typename T>
using NameFuncRef = const char8_t*(const T&);
template <typename T>
using NameMemFunc = const char8_t*(T::*)() const;
template <typename T>
using MessageFuncRef = const char8_t*(const T&, int);
template <typename T>
using MessageMemFunc = const char8_t*(T::*)(int) const;

template <typename T>
struct StatusProviderVTBL {
    static const IStatusProviderVTBL vtbl;
};

template <typename T>
const IStatusProviderVTBL StatusProviderVTBL<T>::vtbl = {
    util::func_impl::ReplaceThisTypeByPtr<static_cast<NameFuncRef<T>*>(util::func_impl::ToExplicitThis<static_cast<NameMemFunc<T>>(&T::Name)>::Function), const IStatusProvider>::Function,
    util::func_impl::ReplaceThisTypeByPtr<static_cast<MessageFuncRef<T>*>(util::func_impl::ToExplicitThis<static_cast<MessageMemFunc<T>>(&T::Message)>::Function), const IStatusProvider>::Function
};

template <typename T>
struct StatusProviderInit {
    StatusProviderInit() {
        auto interface = static_cast<IStatusProvider*>(static_cast<T*>(this));
        interface->vtbl = &StatusProviderVTBL<T>::vtbl;
    }
};

class StatusProviderAdapter {
public:
    StatusProviderAdapter(IStatusProvider* provider) : provider(provider) {}
    const char8_t* Name() const {
        return (provider->vtbl->Name)(provider);
    }
    const char8_t* Message(int status) const {
        return provider->vtbl->Message(provider, status);
    }
private:
    IStatusProvider* provider;
};

StatusProviderAdapter adapter(IStatusProvider* provider)
{
    return { provider };
}

inline Status MakeStatus(unsigned source, int status, bool error)
{
    return static_cast<Status>(dse_core_MakeStatus(source, status, error));
}

inline bool IsError(Status status)
{
    return dse_core_IsError(static_cast<dse_core_Status>(status));
}

inline unsigned StatusSource(Status status)
{
    return dse_core_StatusSource(static_cast<dse_core_Status>(status));
}

inline int StatusCode(dse_core_Status status)
{
    return dse_core_StatusCode(static_cast<dse_core_Status>(status));
}

}

#undef GENERATE_ENUM
#undef GENERATE_ENUM_ELEM

#endif // STATUS_H

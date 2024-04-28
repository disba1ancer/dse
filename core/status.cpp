#include <dse/core/status.h>
#include <unordered_map>
#include <unordered_set>
#include "status_win32.h"

namespace {

using namespace dse::core::status;

class CoreStatusProvider : public StatusProviderBase<CoreStatusProvider> {
public:
    CoreStatusProvider();
    const char8_t* Name() const
    {
        return u8"CoreStatusProvider";
    }
    const char8_t* Message(int status) const {
        auto statusc = static_cast<Code>(status);
        switch (statusc) {
#define DEFINE_STATUS(name, value, message) case Code::name: return message;
#include <dse/core/statusdef.h>
#undef DEFINE_STATUS
        }
        return nullptr;
    }
};

CoreStatusProvider::CoreStatusProvider()
{}

class StatusProviderRegistry {
    StatusProviderRegistry() :
        maxFreeID(ReservedSourceIds)
    {
        providers[SourceId] = &coreProvider;
        providers[SystemSourceId] = &systemProvider;
    }
    auto FindByID(unsigned providerID) -> std::unordered_map<unsigned, IProvider*>::iterator
    {
        return providers.find(providerID);
    }
    auto GenerateFreeId() -> unsigned
    {
        if (freeIDs.empty()) {
            return maxFreeID++;
        }
        auto it = freeIDs.begin();
        auto result = *it;
        freeIDs.erase(it);
        return result;
    }
public:
    static auto Instance() -> StatusProviderRegistry&
    {
        static StatusProviderRegistry instance;
        return instance;
    }
    unsigned Register(IProvider* provider)
    {
        auto result = GenerateFreeId();
        providers[result] = provider;
        return result;
    }
    IProvider* ProviderByID(unsigned providerID)
    {
        auto it = FindByID(providerID);
        if (it == providers.end()) {
            return nullptr;
        }
        return it->second;
    }
    void Unregister(unsigned providerID)
    {

        providers.erase(providerID);
        freeIDs.emplace(providerID);
    }
private:
    std::unordered_map<unsigned, IProvider*> providers;
    unsigned maxFreeID;
    std::unordered_set<unsigned> freeIDs;
    CoreStatusProvider coreProvider;
    SystemStatusProvider systemProvider;
};

}

API_DSE_CORE unsigned dse_core_status_RegisterProvider(dse_core_status_IProvider* provider)
{
    return StatusProviderRegistry::Instance().Register(provider);
}

API_DSE_CORE dse_core_status_IProvider* dse_core_status_ProviderByID(unsigned providerID)
{
    return StatusProviderRegistry::Instance().ProviderByID(providerID);
}

API_DSE_CORE void dse_core_status_UnregisterProvider(unsigned providerID)
{
    StatusProviderRegistry::Instance().Unregister(providerID);
}

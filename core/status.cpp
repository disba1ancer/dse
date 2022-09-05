#include <dse/core/status.h>
#include <unordered_map>
#include <unordered_set>

namespace {

using namespace dse::core;

class CoreStatusProvider : public IStatusProvider, public StatusProviderInit<CoreStatusProvider> {
public:
    CoreStatusProvider();
    const char8_t* Name() const
    {
        return u8"CoreStatusProvider";
    }
    const char8_t* Message(int status) const {
        auto statusc = static_cast<StatusEnum>(status);
        switch (statusc) {
            case StatusEnum::Success:
                return u8"Successful operation";
            case StatusEnum::PendingOperation:
                return u8"Operation in progress";
            case StatusEnum::EndOfStream:
                return u8"End of stream";
        }
        return nullptr;
    }
};

CoreStatusProvider::CoreStatusProvider()
{}

class StatusProviderRegistry {
    StatusProviderRegistry() :
        maxFreeID(0)
    {
        providers[GenerateFreeId()] = &coreProvider;
    }
    auto FindByID(unsigned providerID) -> std::unordered_map<unsigned, IStatusProvider*>::iterator
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
    static StatusProviderRegistry instance;
    unsigned Register(IStatusProvider* provider)
    {
        auto result = GenerateFreeId();
        providers[result] = provider;
        return result;
    }
    IStatusProvider* ProviderByID(unsigned providerID)
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
    std::unordered_map<unsigned, IStatusProvider*> providers;
    unsigned maxFreeID;
    std::unordered_set<unsigned> freeIDs;
    CoreStatusProvider coreProvider;
};

StatusProviderRegistry StatusProviderRegistry::instance;

}

API_DSE_CORE unsigned dse_core_RegisterStatusProvider(dse_core_IStatusProvider* provider)
{
    return StatusProviderRegistry::instance.Register(provider);
}

API_DSE_CORE dse_core_IStatusProvider* dse_core_ProviderByID(unsigned providerID)
{
    return StatusProviderRegistry::instance.ProviderByID(providerID);
}

API_DSE_CORE void dse_core_UnregisterStatusProvider(unsigned providerID)
{
    StatusProviderRegistry::instance.Unregister(providerID);
}

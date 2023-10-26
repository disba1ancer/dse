#include <dse/core/modloader.h>

namespace dse::core {

namespace {

struct StaticModuleManager {
    StaticModuleManager& Instance() {
        static StaticModuleManager instance;
        return instance;
    }
};

}

void API_DSE_CORE dse_core_RegisterModule(const char8_t* name, void (*(*getProc)(const char8_t*))()) noexcept
{}

ModuleHandleC API_DSE_CORE dse_core_LoadModule(const char8_t *name) noexcept
{
    return 0;
}

void API_DSE_CORE dse_core_UnloadModule(ModuleHandleC h) noexcept
{}

auto API_DSE_CORE dse_core_GetModuleEntry(ModuleHandleC h, const char8_t *name) noexcept -> void(*)()
{
    return nullptr;
}

} // namespace dse::core

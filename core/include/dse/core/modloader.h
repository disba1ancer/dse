#ifndef DSE_CORE_MODLOADER_H
#define DSE_CORE_MODLOADER_H

#include <cstdint>
#include <string_view>
#include <concepts>
#include <utility>
#include <memory>
#include "detail/impexp.h"

namespace dse::core {

using ModuleHandleC = std::uintptr_t;

extern "C" {
auto API_DSE_CORE dse_core_LoadModule(const char8_t* name) noexcept -> ModuleHandleC;
void API_DSE_CORE dse_core_UnloadModule(ModuleHandleC h) noexcept;
auto API_DSE_CORE dse_core_GetModuleEntry(ModuleHandleC h, const char8_t* name) noexcept -> void(*)();
}

struct ModuleHandle {
    ModuleHandleC handle;

    operator bool() const noexcept
    {
        return handle;
    }

    friend bool operator==(const ModuleHandle& h, std::nullptr_t) noexcept
    {
        return ModuleHandleC(h) == 0;
    }

    friend void swap(ModuleHandle& a, ModuleHandle& b) noexcept
    {
        std::swap(a.handle, b.handle);
    }
};

inline auto LoadModule(const std::u8string& name) -> ModuleHandle
{
    return ModuleHandle{dse_core_LoadModule(name.data())};
}

inline void UnloadModule(ModuleHandle h)
{
    dse_core_UnloadModule(h.handle);
}

inline auto GetModuleEntry(ModuleHandle h, const std::u8string& name) -> void(*)()
{
    return dse_core_GetModuleEntry(h.handle, name.data());
}

struct ModuleDeleter {
    using pointer = ModuleHandle;
    void operator()(ModuleHandle h) noexcept {
        UnloadModule(h);
    }
};

struct Module : std::unique_ptr<ModuleHandle, ModuleDeleter> {
    using Base = typename std::unique_ptr<ModuleHandle, ModuleDeleter>;
    Module(const std::u8string& name) :
        Base(LoadModule(name))
    {}

    auto GetEntry(const std::u8string& name) noexcept -> void(*)()
    {
        return GetModuleEntry(get(), name);
    }

    template <typename T>
    requires std::is_function_v<T>
    auto GetEntry(const std::u8string& name) noexcept -> void(*)()
    {
        return reinterpret_cast<T*>(GetEntry(name));
    }
};

} // namespace dse::core

#endif // DSE_CORE_MODLOADER_H

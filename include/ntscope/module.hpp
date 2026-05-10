#pragma once

#ifndef _WIN32
#error "ntscope targets Windows."
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include <cstddef>
#include <cstdint>
#include <optional>

namespace ntscope {

class module_view {
public:
    module_view() = default;
    explicit module_view(HMODULE module) : base_(reinterpret_cast<std::byte*>(module)) {}

    [[nodiscard]] static std::optional<module_view> current_process(const wchar_t* name)
    {
        if (!name) {
            return std::nullopt;
        }

        HMODULE module = ::GetModuleHandleW(name);
        if (!module) {
            module = ::LoadLibraryW(name);
        }

        if (!module) {
            return std::nullopt;
        }

        return module_view(module);
    }

    [[nodiscard]] std::byte* base() const noexcept { return base_; }
    [[nodiscard]] HMODULE handle() const noexcept { return reinterpret_cast<HMODULE>(base_); }

    template <class T>
    [[nodiscard]] T* at(std::uint32_t rva) const noexcept
    {
        return reinterpret_cast<T*>(base_ + rva);
    }

private:
    std::byte* base_ = nullptr;
};

} // namespace ntscope

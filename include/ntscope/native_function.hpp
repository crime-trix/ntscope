#pragma once

#include <ntscope/module.hpp>

#include <optional>
#include <utility>

namespace ntscope {

template <class Signature>
class native_function;

template <class Ret, class... Args>
class native_function<Ret(Args...)> {
public:
    native_function() = default;
    explicit native_function(FARPROC proc) : proc_(reinterpret_cast<pointer>(proc)) {}

    [[nodiscard]] static std::optional<native_function> from(module_view module, const char* name)
    {
        if (!module.base() || !name) {
            return std::nullopt;
        }

        auto* proc = ::GetProcAddress(module.handle(), name);
        if (!proc) {
            return std::nullopt;
        }

        return native_function(proc);
    }

    [[nodiscard]] explicit operator bool() const noexcept { return proc_ != nullptr; }

    Ret operator()(Args... args) const
    {
        return proc_(std::forward<Args>(args)...);
    }

private:
    using pointer = Ret(WINAPI*)(Args...);
    pointer proc_ = nullptr;
};

} // namespace ntscope

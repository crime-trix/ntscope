#pragma once

#include <string_view>
#include <utility>

namespace ntscope {

enum class error_code {
    ok,
    module_not_found,
    bad_dos_header,
    bad_nt_headers,
    missing_export_directory,
    malformed_export_directory,
};

constexpr std::string_view to_string(error_code code) noexcept
{
    switch (code) {
    case error_code::ok:
        return "ok";
    case error_code::module_not_found:
        return "module not found";
    case error_code::bad_dos_header:
        return "bad DOS header";
    case error_code::bad_nt_headers:
        return "bad NT headers";
    case error_code::missing_export_directory:
        return "missing export directory";
    case error_code::malformed_export_directory:
        return "malformed export directory";
    }
    return "unknown error";
}

template <class T>
class expected {
public:
    expected(T value) : has_value_(true), value_(std::move(value)) {}
    expected(error_code error) : has_value_(false), error_(error) {}

    [[nodiscard]] explicit operator bool() const noexcept { return has_value_; }
    [[nodiscard]] error_code error() const noexcept { return error_; }

    [[nodiscard]] const T& value() const& noexcept { return value_; }
    [[nodiscard]] T& value() & noexcept { return value_; }
    [[nodiscard]] T&& value() && noexcept { return std::move(value_); }

    [[nodiscard]] const T& operator*() const& noexcept { return value_; }
    [[nodiscard]] T& operator*() & noexcept { return value_; }
    [[nodiscard]] const T* operator->() const noexcept { return &value_; }
    [[nodiscard]] T* operator->() noexcept { return &value_; }

private:
    bool has_value_ = false;
    T value_{};
    error_code error_ = error_code::ok;
};

} // namespace ntscope

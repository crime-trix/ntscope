#pragma once

#include <ntscope/export_table.hpp>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ntscope {

enum class syscall_source {
    stub,
    export_order,
};

constexpr std::string_view to_string(syscall_source source) noexcept
{
    switch (source) {
    case syscall_source::stub:
        return "stub";
    case syscall_source::export_order:
        return "export-order";
    }
    return "unknown";
}

struct syscall_entry {
    std::string name;
    std::uint32_t number = 0;
    std::uint32_t rva = 0;
    syscall_source source = syscall_source::stub;
};

namespace detail {

constexpr bool starts_with(std::string_view text, char a, char b) noexcept
{
    return text.size() > 2 && text[0] == a && text[1] == b;
}

inline std::string nt_name_from_zw(std::string_view name)
{
    std::string copy(name);
    copy[0] = 'N';
    copy[1] = 't';
    return copy;
}

[[nodiscard]] inline std::optional<std::uint32_t> syscall_number_from_stub(const std::byte* code) noexcept
{
    const auto* bytes = reinterpret_cast<const unsigned char*>(code);

#if defined(_M_X64) || defined(__x86_64__)
    // ntdll's normal x64 syscall entry starts with: mov r10, rcx; mov eax, imm32.
    if (bytes[0] == 0x4c && bytes[1] == 0x8b && bytes[2] == 0xd1 && bytes[3] == 0xb8) {
        std::uint32_t number = 0;
        std::memcpy(&number, bytes + 4, sizeof(number));
        return number;
    }
#elif defined(_M_IX86) || defined(__i386__)
    if (bytes[0] == 0xb8) {
        std::uint32_t number = 0;
        std::memcpy(&number, bytes + 1, sizeof(number));
        return number;
    }
#endif

    return std::nullopt;
}

} // namespace detail

class syscall_table {
public:
    [[nodiscard]] static expected<syscall_table> from_ntdll()
    {
        const auto ntdll = module_view::current_process(L"ntdll.dll");
        if (!ntdll) {
            return error_code::module_not_found;
        }
        return from_module(*ntdll);
    }

    [[nodiscard]] static expected<syscall_table> from_module(module_view module)
    {
        auto exports = export_table::read_entries(module);
        if (!exports) {
            return exports.error();
        }

        syscall_table table;
        table.add_clean_stubs(module, *exports);
        table.add_export_order_fallback(*exports);
        std::ranges::sort(table.entries_, {}, &syscall_entry::name);
        return table;
    }

    [[nodiscard]] std::span<const syscall_entry> entries() const noexcept
    {
        return entries_;
    }

    [[nodiscard]] std::optional<syscall_entry> find(std::string_view name) const
    {
        const auto it = std::ranges::lower_bound(entries_, name, {}, &syscall_entry::name);
        if (it == entries_.end() || it->name != name) {
            return std::nullopt;
        }
        return *it;
    }

    [[nodiscard]] std::optional<syscall_entry> find_by_number(std::uint32_t number) const
    {
        const auto it = std::ranges::find(entries_, number, &syscall_entry::number);
        if (it == entries_.end()) {
            return std::nullopt;
        }
        return *it;
    }

    [[nodiscard]] std::size_t count_by_source(syscall_source source) const
    {
        return static_cast<std::size_t>(std::ranges::count(entries_, source, &syscall_entry::source));
    }

private:
    void add_clean_stubs(module_view module, const std::vector<export_entry>& exports)
    {
        for (const auto& item : exports) {
            const std::string_view name(item.name);
            if (!detail::starts_with(name, 'N', 't')) {
                continue;
            }

            const auto number = detail::syscall_number_from_stub(module.at<std::byte>(item.rva));
            if (!number) {
                continue;
            }

            entries_.push_back(syscall_entry{item.name, *number, item.rva, syscall_source::stub});
        }
    }

    void add_export_order_fallback(const std::vector<export_entry>& exports)
    {
        std::vector<export_entry> zw_exports;
        for (const auto& item : exports) {
            if (detail::starts_with(item.name, 'Z', 'w')) {
                zw_exports.push_back(item);
            }
        }

        std::ranges::sort(zw_exports, {}, &export_entry::rva);
        for (std::uint32_t number = 0; number < zw_exports.size(); ++number) {
            const auto name = detail::nt_name_from_zw(zw_exports[number].name);
            const auto exists = std::ranges::any_of(entries_, [&](const syscall_entry& entry) {
                return entry.name == name;
            });

            if (!exists) {
                entries_.push_back(syscall_entry{name, number, zw_exports[number].rva, syscall_source::export_order});
            }
        }
    }

    std::vector<syscall_entry> entries_;
};

} // namespace ntscope

#pragma once

#include <ntscope/pe.hpp>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ntscope {

struct export_entry {
    std::string name;
    std::uint32_t ordinal = 0;
    std::uint32_t rva = 0;
};

class export_table {
public:
    [[nodiscard]] static expected<export_table> from_module(module_view module)
    {
        auto entries = read_entries(module);
        if (!entries) {
            return entries.error();
        }

        export_table table;
        table.entries_ = std::move(*entries);
        std::ranges::sort(table.entries_, {}, &export_entry::name);
        return table;
    }

    [[nodiscard]] std::span<const export_entry> entries() const noexcept
    {
        return entries_;
    }

    [[nodiscard]] std::optional<export_entry> find(std::string_view name) const
    {
        const auto it = std::ranges::lower_bound(entries_, name, {}, &export_entry::name);
        if (it == entries_.end() || it->name != name) {
            return std::nullopt;
        }
        return *it;
    }

private:
    friend class syscall_table;

    [[nodiscard]] static expected<std::vector<export_entry>> read_entries(module_view module)
    {
        auto headers = detail::read_headers(module);
        if (!headers) {
            return headers.error();
        }

        const auto image_size = headers->image_size;
        const auto& dir = headers->nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        if (!dir.VirtualAddress || !dir.Size) {
            return error_code::missing_export_directory;
        }

        if (!detail::range_fits<IMAGE_EXPORT_DIRECTORY>(dir.VirtualAddress, image_size)) {
            return error_code::malformed_export_directory;
        }

        const auto* exports = module.at<IMAGE_EXPORT_DIRECTORY>(dir.VirtualAddress);
        if (!detail::range_fits<std::uint32_t>(exports->AddressOfNames, image_size, exports->NumberOfNames) ||
            !detail::range_fits<std::uint16_t>(exports->AddressOfNameOrdinals, image_size, exports->NumberOfNames) ||
            !detail::range_fits<std::uint32_t>(exports->AddressOfFunctions, image_size, exports->NumberOfFunctions)) {
            return error_code::malformed_export_directory;
        }

        const auto* names = module.at<std::uint32_t>(exports->AddressOfNames);
        const auto* ordinals = module.at<std::uint16_t>(exports->AddressOfNameOrdinals);
        const auto* functions = module.at<std::uint32_t>(exports->AddressOfFunctions);

        std::vector<export_entry> result;
        result.reserve(exports->NumberOfNames);

        for (std::uint32_t i = 0; i < exports->NumberOfNames; ++i) {
            const auto name_rva = names[i];
            if (!detail::c_string_fits(module, name_rva, image_size)) {
                return error_code::malformed_export_directory;
            }

            const auto ordinal = ordinals[i];
            if (ordinal >= exports->NumberOfFunctions) {
                return error_code::malformed_export_directory;
            }

            const auto rva = functions[ordinal];
            if (rva >= image_size) {
                return error_code::malformed_export_directory;
            }

            result.push_back(export_entry{module.at<char>(name_rva), ordinal, rva});
        }

        return result;
    }

    std::vector<export_entry> entries_;
};

} // namespace ntscope

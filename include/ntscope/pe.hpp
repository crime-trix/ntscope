#pragma once

#include <ntscope/expected.hpp>
#include <ntscope/module.hpp>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ntscope::detail {

template <class T>
[[nodiscard]] bool range_fits(std::uint32_t rva, std::size_t image_size, std::size_t count = 1) noexcept
{
    const std::size_t bytes = sizeof(T) * count;
    return rva <= image_size && bytes <= image_size - rva;
}

[[nodiscard]] inline bool c_string_fits(module_view module, std::uint32_t rva, std::size_t image_size) noexcept
{
    if (rva >= image_size) {
        return false;
    }

    const auto* first = module.at<char>(rva);
    const auto* last = reinterpret_cast<const char*>(module.base() + image_size);
    return std::find(first, last, '\0') != last;
}

struct image_headers {
    const IMAGE_DOS_HEADER* dos = nullptr;
    const IMAGE_NT_HEADERS* nt = nullptr;
    std::size_t image_size = 0;
};

[[nodiscard]] inline expected<image_headers> read_headers(module_view module)
{
    if (!module.base()) {
        return error_code::module_not_found;
    }

    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(module.base());
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
        return error_code::bad_dos_header;
    }

    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(module.base() + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) {
        return error_code::bad_nt_headers;
    }

    return image_headers{dos, nt, static_cast<std::size_t>(nt->OptionalHeader.SizeOfImage)};
}

} // namespace ntscope::detail

#include <ntscope/ntscope.hpp>

#include <iostream>

int main()
{
    const auto ntdll = ntscope::module_view::current_process(L"ntdll.dll");
    if (!ntdll) {
        return 1;
    }

    const auto exports = ntscope::export_table::from_module(*ntdll);
    if (!exports) {
        std::cerr << ntscope::to_string(exports.error()) << '\n';
        return 1;
    }

    const auto close_export = exports->find("NtClose");
    if (!close_export) {
        std::cerr << "NtClose was not found\n";
        return 1;
    }

    std::cout << close_export->name << " ordinal " << close_export->ordinal
              << " rva 0x" << std::hex << close_export->rva << '\n';
}

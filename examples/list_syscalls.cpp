#include <ntscope/ntscope.hpp>

#include <iostream>

int main()
{
    const auto ntdll = ntscope::module_view::current_process(L"ntdll.dll");
    if (!ntdll) {
        std::cerr << "ntdll.dll is not loaded\n";
        return 1;
    }

    const auto syscalls = ntscope::syscall_table::from_module(*ntdll);
    if (!syscalls) {
        std::cerr << "failed to read ntdll metadata: " << ntscope::to_string(syscalls.error()) << '\n';
        return 1;
    }

    for (const auto& item : syscalls->entries()) {
        std::cout << item.name << " -> " << item.number
                  << " @ 0x" << std::hex << item.rva << std::dec
                  << " [" << (item.source == ntscope::syscall_source::stub ? "stub" : "export-order") << "]\n";
    }
}

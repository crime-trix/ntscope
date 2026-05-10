#include <ntscope/ntscope.hpp>

#include <iostream>

int main()
{
    const auto syscalls = ntscope::syscall_table::from_ntdll();
    if (!syscalls) {
        std::cerr << "failed to read ntdll metadata: " << ntscope::to_string(syscalls.error()) << '\n';
        return 1;
    }

    for (const auto& item : syscalls->entries()) {
        std::cout << item.name << " -> " << item.number
                  << " @ 0x" << std::hex << item.rva << std::dec
                  << " [" << ntscope::to_string(item.source) << "]\n";
    }
}

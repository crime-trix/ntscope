#include <ntscope/ntscope.hpp>

#include <cassert>
#include <windows.h>

int main()
{
    const auto ntdll = ntscope::module_view::current_process(L"ntdll.dll");
    assert(ntdll);

    const auto exports = ntscope::export_table::from_module(*ntdll);
    assert(exports);
    assert(exports->find("NtClose"));

    const auto syscalls = ntscope::syscall_table::from_module(*ntdll);
    assert(syscalls);
    assert(syscalls->entries().size() > 100);
    assert(syscalls->find("NtClose"));
    assert(syscalls->find_by_number(syscalls->find("NtClose")->number));

    using nt_close = LONG(HANDLE);
    const auto NtClose = ntscope::native_function<nt_close>::from(*ntdll, "NtClose");
    assert(NtClose);

    const auto status = (*NtClose)(reinterpret_cast<HANDLE>(static_cast<ULONG_PTR>(0x1234)));
    assert(status == static_cast<LONG>(0xC0000008));
}

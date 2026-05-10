# ntscope

`ntscope` is a small C++20 header-only library for inspecting Windows Native API metadata in loaded modules. It reads PE exports, builds a searchable `Nt*` syscall table from `ntdll.dll`, and keeps the result visible as ordinary C++ data.

[![ci](https://github.com/crime-trix/ntscope/actions/workflows/ci.yml/badge.svg)](https://github.com/crime-trix/ntscope/actions/workflows/ci.yml)

## Example

```cpp
#include <ntscope/ntscope.hpp>

#include <iostream>

int main() {
    auto ntdll = ntscope::module_view::current_process(L"ntdll.dll");
    auto syscalls = ntscope::syscall_table::from_module(*ntdll);

    if (auto entry = syscalls->find("NtQuerySystemInformation")) {
        std::cout << entry->name << " = " << entry->number << "\n";
    }
}
```

## Surface

- `module_view`: lightweight view over a module loaded in the current process.
- `export_table`: validated PE export enumeration with lookup by name.
- `syscall_table`: searchable `Nt*` metadata with source tracking.
- `native_function<Signature>`: typed lookup for exported Native API routines.

The library does not allocate executable memory and does not install process-wide handlers. It is a metadata layer first: predictable, inspectable, and easy to embed in tools.

## Build

```sh
cmake -S . -B build -DNTSCOPE_BUILD_EXAMPLES=ON -DNTSCOPE_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Requirements

- Windows
- C++20 compiler
- CMake 3.20+ for the example/test project

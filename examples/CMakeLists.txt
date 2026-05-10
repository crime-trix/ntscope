add_executable(ntscope_list_syscalls list_syscalls.cpp)
target_link_libraries(ntscope_list_syscalls PRIVATE ntscope::ntscope)

add_executable(ntscope_find_native find_native.cpp)
target_link_libraries(ntscope_find_native PRIVATE ntscope::ntscope)

add_executable(ntscope_smoke smoke.cpp)
target_link_libraries(ntscope_smoke PRIVATE ntscope::ntscope)
add_test(NAME ntscope_smoke COMMAND ntscope_smoke)

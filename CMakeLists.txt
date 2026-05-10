cmake_minimum_required(VERSION 3.20)

project(ntscope VERSION 0.1.0 LANGUAGES CXX)

option(NTSCOPE_BUILD_EXAMPLES "Build ntscope examples" ${PROJECT_IS_TOP_LEVEL})
option(NTSCOPE_BUILD_TESTS "Build ntscope tests" ${PROJECT_IS_TOP_LEVEL})

include(GNUInstallDirs)

add_library(ntscope INTERFACE)
add_library(ntscope::ntscope ALIAS ntscope)

target_compile_features(ntscope INTERFACE cxx_std_20)
target_include_directories(ntscope INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(WIN32)
    target_compile_definitions(ntscope INTERFACE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

if(NTSCOPE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(NTSCOPE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS ntscope EXPORT ntscope-targets)
install(EXPORT ntscope-targets
    FILE ntscope-config.cmake
    NAMESPACE ntscope::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ntscope
)

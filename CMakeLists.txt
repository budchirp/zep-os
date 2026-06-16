cmake_minimum_required(VERSION 3.30)

project(
    zep
    VERSION 0.1.0
    LANGUAGES C CXX ASM
)

set(ZEP_ARCH "x86_64" CACHE STRING "Target architecture")
set_property(CACHE ZEP_ARCH PROPERTY STRINGS x86_64 aarch64)

include(cmake/Standards.cmake)
include(cmake/Flags.cmake)

add_subdirectory(kernel)
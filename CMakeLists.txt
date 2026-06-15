cmake_minimum_required(VERSION 3.30)

project(zep
    VERSION 0.1.0
    DESCRIPTION "The Zep OS"
    LANGUAGES C CXX
)

option(ZEP_USE_CCACHE "Use ccache when available" ON)

set(ZEP_ARCH "aarch64" CACHE STRING "Target architecture (aarch64, riscv64)")
set(ZEP_BOOT_BACKEND "limine" CACHE STRING "Boot backend (limine)")

if(ZEP_USE_CCACHE)
    find_program(CCACHE ccache)
    if(CCACHE)
        message(STATUS "Using ccache as compiler launcher")
        set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE}")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE}")
    endif()
endif()

include(cmake/Standards.cmake)
include(cmake/Flags.cmake)
include(cmake/FastLink.cmake)
include(cmake/Modules.cmake)

add_subdirectory(kernel)

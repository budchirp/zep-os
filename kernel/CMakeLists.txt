set(KERNEL_TARGET "${ZEP_ARCH}-none-elf")

file(GLOB_RECURSE KERNEL_CPP CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
list(FILTER KERNEL_CPP EXCLUDE REGEX "src/arch/(aarch64|x86_64)/.*")

file(GLOB_RECURSE KERNEL_CPP_ARCH CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/arch/${ZEP_ARCH}/*.cpp"
)
list(APPEND KERNEL_CPP ${KERNEL_CPP_ARCH})

file(GLOB_RECURSE KERNEL_CPPM CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cppm"
)
list(FILTER KERNEL_CPPM EXCLUDE REGEX "src/arch/(aarch64|x86_64)/.*")

file(GLOB_RECURSE KERNEL_CPPM_ARCH CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/arch/${ZEP_ARCH}/*.cppm"
)
list(APPEND KERNEL_CPPM ${KERNEL_CPPM_ARCH})

include("${CMAKE_SOURCE_DIR}/cmake/Flags.cmake")

set(ZEP_KERNEL_OBJECT "${CMAKE_CURRENT_SOURCE_DIR}/zep/build/${ZEP_ARCH}-unknown-none/objs/kernel.o")
if(NOT EXISTS "${ZEP_KERNEL_OBJECT}")
    set(ZEP_KERNEL_OBJECT "")
endif()

add_executable(zep_kernel ${KERNEL_CPP} ${ZEP_KERNEL_OBJECT})
if(KERNEL_CPPM)
    target_sources(zep_kernel PUBLIC FILE_SET CXX_MODULES FILES ${KERNEL_CPPM})
endif()
target_include_directories(zep_kernel PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/boot/limine"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
zep_set_kernel_flags(zep_kernel ${KERNEL_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/linker/${ZEP_ARCH}.ld")

add_executable(zep_kernel_direct ${KERNEL_CPP})
if(KERNEL_CPPM)
    target_sources(zep_kernel_direct PUBLIC FILE_SET CXX_MODULES FILES ${KERNEL_CPPM})
endif()
target_include_directories(zep_kernel_direct PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/boot/limine"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
zep_set_kernel_flags(zep_kernel_direct ${KERNEL_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/linker/${ZEP_ARCH}-direct.ld")

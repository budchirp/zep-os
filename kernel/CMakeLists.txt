set(KERNEL_TARGET "${ZEP_ARCH}-none-elf")

file(GLOB_RECURSE KERNEL_CPP CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE KERNEL_CPPM CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cppm"
)

include("${CMAKE_SOURCE_DIR}/cmake/Flags.cmake")

add_executable(zep_kernel ${KERNEL_CPP})
if(KERNEL_CPPM)
    target_sources(zep_kernel PUBLIC FILE_SET CXX_MODULES FILES ${KERNEL_CPPM})
endif()
target_include_directories(zep_kernel PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/boot/info"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/boot/info/limine"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
zep_set_kernel_flags(zep_kernel ${KERNEL_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/linker/${ZEP_ARCH}.ld")

add_executable(zep_kernel_direct ${KERNEL_CPP})
if(KERNEL_CPPM)
    target_sources(zep_kernel_direct PUBLIC FILE_SET CXX_MODULES FILES ${KERNEL_CPPM})
endif()
target_include_directories(zep_kernel_direct PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/boot/info"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/boot/info/limine"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
zep_set_kernel_flags(zep_kernel_direct ${KERNEL_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/linker/${ZEP_ARCH}-direct.ld")

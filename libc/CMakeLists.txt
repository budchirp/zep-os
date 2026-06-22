add_library(zep_libc STATIC
    src/stdlib.cpp
    src/arch/${ZEP_ARCH}/syscall.cpp
)

target_link_libraries(zep_libc PRIVATE common)

target_include_directories(zep_libc PRIVATE src)

target_compile_options(zep_libc PRIVATE
    -target ${ZEP_ARCH}-unknown-none
    -ffreestanding
    -nostdlib
    -O2
    -fno-rtti
    -fno-exceptions
    $<$<STREQUAL:${ZEP_ARCH},x86_64>:-mno-red-zone>
    $<$<STREQUAL:${ZEP_ARCH},aarch64>:-mgeneral-regs-only>
)

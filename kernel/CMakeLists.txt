if(ZEP_ARCH STREQUAL "x86_64")
    set(KERNEL_TARGET "x86_64-none-elf")

    set(OBJCOPY_INPUT_TARGET "elf64-x86-64")
    set(OBJCOPY_OUTPUT_TARGET "efi-app-x86_64")
    set(OBJCOPY_SECTIONS -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rela -j .reloc -j .rodata)

    set(KERNEL_ARCH_DIR "arch/x86_64")

    set(KERNEL_ASM_CRT0
        ${CMAKE_SOURCE_DIR}/third_party/gnu-efi/gnuefi/crt0-efi-x86_64.S
    )

    set(KERNEL_RELOC_C
        ${CMAKE_SOURCE_DIR}/third_party/gnu-efi/gnuefi/reloc_x86_64.c
    )

    find_program(KERNEL_OBJCOPY REQUIRED NAMES objcopy llvm-objcopy)
elseif(ZEP_ARCH STREQUAL "aarch64")
    set(KERNEL_TARGET "aarch64-none-elf")

    set(OBJCOPY_INPUT_TARGET "elf64-littleaarch64")
    set(OBJCOPY_OUTPUT_TARGET "efi-app-aarch64")
    set(OBJCOPY_SECTIONS -j .text -j .data -j .rodata -j .dynamic -j .dynsym -j .rela -j .reloc)

    set(KERNEL_ARCH_DIR "arch/aarch64")

    set(KERNEL_ASM_CRT0
        ${CMAKE_SOURCE_DIR}/third_party/gnu-efi/gnuefi/crt0-efi-aarch64.S
    )

    set(KERNEL_RELOC_C
        ${CMAKE_SOURCE_DIR}/third_party/gnu-efi/gnuefi/reloc_aarch64.c
    )

    find_program(KERNEL_OBJCOPY REQUIRED NAMES aarch64-linux-gnu-objcopy llvm-objcopy)
else()
    message(FATAL_ERROR "Unsupported ZEP_ARCH: ${ZEP_ARCH}")
endif()

set(KERNEL_LINKER_SCRIPT
    ${CMAKE_SOURCE_DIR}/third_party/gnu-efi/gnuefi/elf_${ZEP_ARCH}_efi.lds
)

file(GLOB_RECURSE KERNEL_COMMON_MODULES
    CONFIGURE_DEPENDS
    src/*.cppm
)

list(FILTER KERNEL_COMMON_MODULES EXCLUDE REGEX "src/arch/")

file(GLOB_RECURSE KERNEL_COMMON_SOURCES
    CONFIGURE_DEPENDS
    src/*.cpp
)

list(FILTER KERNEL_COMMON_SOURCES EXCLUDE REGEX "src/arch/")

file(GLOB_RECURSE KERNEL_ARCH_SOURCES
    CONFIGURE_DEPENDS
    src/${KERNEL_ARCH_DIR}/*.cpp
)

if(DEFINED KERNEL_RELOC_C)
    add_library(reloc_obj OBJECT ${KERNEL_RELOC_C})

    target_include_directories(reloc_obj PRIVATE
        ${CMAKE_SOURCE_DIR}/third_party/gnu-efi/inc
        /usr/${ZEP_ARCH}-linux-gnu/include
    )

    target_compile_options(reloc_obj PRIVATE
        -target ${KERNEL_TARGET}
        $<$<STREQUAL:${ZEP_ARCH},x86_64>:-DGNU_EFI_USE_MS_ABI>
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O2 -DNDEBUG>
    )
endif()

add_library(kernel_so SHARED
    ${KERNEL_COMMON_SOURCES}
    ${KERNEL_ARCH_SOURCES}
    ${KERNEL_ASM_CRT0}
)

target_sources(kernel_so
    PRIVATE
        FILE_SET CXX_MODULES
        FILES
            ${KERNEL_COMMON_MODULES}
)

target_include_directories(kernel_so PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party/gnu-efi/inc
)

zep_set_kernel_flags(kernel_so "${KERNEL_TARGET}" "${KERNEL_LINKER_SCRIPT}")

target_link_libraries(kernel_so PRIVATE common)

if(DEFINED KERNEL_RELOC_C)
    target_link_libraries(kernel_so PRIVATE reloc_obj)
endif()

add_custom_command(TARGET kernel_so POST_BUILD
    COMMAND ${KERNEL_OBJCOPY}
        --input-target=${OBJCOPY_INPUT_TARGET}
        --output-target=${OBJCOPY_OUTPUT_TARGET}
        --subsystem=10
        ${OBJCOPY_SECTIONS}
        "$<TARGET_FILE:kernel_so>"
        "$<TARGET_FILE_DIR:kernel_so>/kernel.efi"
    COMMENT "Generating kernel.efi from $<TARGET_FILE:kernel_so>"
    VERBATIM
)

add_custom_target(kernel DEPENDS kernel_so)

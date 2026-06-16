option(ZEP_DEBUG_FULL_SYMBOLS "Debug: use -g3 -ggdb instead of -g (much slower compiles)" OFF)

function(zep_set_warning_flags TARGET)
    target_compile_options(${TARGET} PRIVATE
        -pipe
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wno-shadow
        -Wdouble-promotion
        -Wcast-qual
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Wcast-align
        -Woverloaded-virtual
        -Wnull-dereference
        -Wformat=2
        -Wimplicit-fallthrough
        -Wunused
        -Wvla
        -Wno-c99-designator
    )
endfunction()

function(zep_set_flags TARGET)
    if(ZEP_DEBUG_FULL_SYMBOLS)
        set(zep_debug_symbols -g3 -ggdb)
    else()
        set(zep_debug_symbols -g)
    endif()

    zep_set_warning_flags(${TARGET})

    target_compile_options(${TARGET} PRIVATE
        $<$<CONFIG:Debug>:${zep_debug_symbols} -O0 -fno-omit-frame-pointer -DZEP_DEBUG>
        $<$<CONFIG:Release>:-O3 -DNDEBUG -flto -march=native -ffunction-sections -fdata-sections>
    )

    target_link_options(${TARGET} PRIVATE
        $<$<CONFIG:Release>:-flto -s -Wl,--gc-sections>
    )
endfunction()

function(zep_set_kernel_flags TARGET KERNEL_TARGET LINKER_SCRIPT)
    zep_set_warning_flags(${TARGET})

    target_compile_options(${TARGET} PRIVATE
        -target ${KERNEL_TARGET}
        -fpic
        -fshort-wchar
        -ffreestanding
        -nostdlib
        -fno-rtti
        -fno-exceptions
        -fno-stack-protector
        -fno-use-cxa-atexit
        -fno-threadsafe-statics
        -fno-builtin-memset
        $<$<STREQUAL:${ZEP_ARCH},aarch64>:-mgeneral-regs-only>
        $<$<STREQUAL:${ZEP_ARCH},x86_64>:-mno-red-zone -DGNU_EFI_USE_MS_ABI>
        $<$<CONFIG:Debug>:-g -O0 -fno-omit-frame-pointer -DZEP_DEBUG>
        $<$<CONFIG:Release>:-O2 -DNDEBUG -ffunction-sections -fdata-sections>
    )

    target_link_options(${TARGET} PRIVATE
        -target ${KERNEL_TARGET}
        -fuse-ld=lld
        -nostdlib
        -T "${LINKER_SCRIPT}"
        -Wl,-shared
        -Wl,-Bsymbolic
        -Wl,-z,norelro
    )
endfunction()

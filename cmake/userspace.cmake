function(add_user_binary TARGET_NAME SOURCE_FILE)
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    target_link_libraries(${TARGET_NAME} PRIVATE zep_libc)
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/common/include)

    target_compile_options(${TARGET_NAME} PRIVATE
        -target ${ZEP_ARCH}-unknown-none
        -ffreestanding
        -nostdlib
        -fno-rtti
        -fno-exceptions
        $<$<STREQUAL:${ZEP_ARCH},x86_64>:-mno-red-zone>
        $<$<STREQUAL:${ZEP_ARCH},aarch64>:-mgeneral-regs-only>
    )

    target_link_options(${TARGET_NAME} PRIVATE
        -target ${ZEP_ARCH}-unknown-none
        -fuse-ld=lld
        -nostdlib
        -static
        -Wl,-Ttext,0x400000
        -Wl,-e,main
    )
endfunction()

PHDRS
{
    text  PT_LOAD FLAGS(5);
    data  PT_LOAD FLAGS(6);
}

SECTIONS
{
    . = 0x40200000;

    .text : {
        KEEP(*(.text._start))
        *(.text .text.*)
    } :text

    .rodata : {
        *(.rodata .rodata.*)
    } :text

    .vectors : {
        KEEP(*(.vectors))
    } :text

    .limine_requests : {
        KEEP(*(.limine_requests_start_marker))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end_marker))
    } :text

    . = ALIGN(4096);

    .data : {
        *(.data .data.*)
    } :data

    .init_array : {
        __init_array_start = .;
        KEEP(*(.init_array .init_array.*))
        __init_array_end = .;
    } :data

    .bss : {
        __bss_start = .;
        *(COMMON)
        *(.bss .bss.*)
        . = ALIGN(16);
        stack_top = .;
        . = . + 65536;
        __bss_end = .;
    } :data

    /DISCARD/ : {
        *(.note .note.*)
        *(.eh_frame_hdr .eh_frame_hdr.*)
        *(.eh_frame .eh_frame.*)
    }
}

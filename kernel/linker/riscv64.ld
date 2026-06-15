PHDRS
{
    text       PT_LOAD FILEHDR PHDRS;
    rodata     PT_LOAD;
    data       PT_LOAD;
    limine_req PT_LOAD;
}

SECTIONS
{
    . = 0xffffffff80000000;

    .text : {
        KEEP(*(.text._start))
        *(.text .text.*)
    } :text

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    .limine_requests : {
        KEEP(*(.limine_requests_start_marker))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end_marker))
    } :limine_req

    .data : {
        *(.data .data.*)
    } :data

    .bss : {
        __bss_start = .;
        *(COMMON)
        *(.bss .bss.*)
        __bss_end = .;
    } :data

    .init_array : {
        __init_array_start = .;
        KEEP(*(.init_array .init_array.*))
        __init_array_end = .;
    } :data

    /DISCARD/ : {
        *(.note .note.*)
        *(.eh_frame_hdr .eh_frame_hdr.*)
        *(.eh_frame .eh_frame.*)
    }
}

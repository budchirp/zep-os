#include "std/runtime.h"

#include <efi.h>
#include <efiprot.h>

import zep.std.types;
import zep.std;
import zep.serial;
import zep.gfx.terminal;
import zep.gfx.color;
import zep.gfx.framebuffer;

alignas(Serial) static unsigned char serial_storage[sizeof(Serial)];
alignas(Terminal) static unsigned char terminal_storage[sizeof(Terminal)];

static Serial* serial = nullptr;
static Terminal* terminal = nullptr;

extern "C" {

void* __serial = nullptr;
void* __terminal = nullptr;

extern void main();

EFI_STATUS _entry(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
    (void)image_handle;

    serial = new (serial_storage) Serial(system_table->ConOut);
    __serial = serial;

    serial->write_line("Zep OS: serial up");

    Framebuffer* framebuffer = nullptr;
    if (system_table != nullptr && system_table->BootServices != nullptr) {
        EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
        EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = nullptr;

        EFI_STATUS status = system_table->BootServices->LocateProtocol(
            &gop_guid, nullptr, reinterpret_cast<VOID**>(&gop));

        if (status == 0 && gop != nullptr && gop->Mode != nullptr && gop->Mode->Info != nullptr) {
            auto fb_addr =
                reinterpret_cast<void*>(static_cast<uintptr>(gop->Mode->FrameBufferBase));
            auto fb_width = static_cast<u64>(gop->Mode->Info->HorizontalResolution);
            auto fb_height = static_cast<u64>(gop->Mode->Info->VerticalResolution);
            auto fb_pitch = static_cast<u64>(gop->Mode->Info->PixelsPerScanLine) * 4;

            alignas(Framebuffer) static unsigned char fb_storage[sizeof(Framebuffer)];
            framebuffer = new (fb_storage) Framebuffer(fb_addr, fb_width, fb_height, fb_pitch, 32);
        }
    }

    if (framebuffer != nullptr) {
        terminal = new (terminal_storage) Terminal(*framebuffer);
        __terminal = terminal;

        terminal->bg_color = Color::black();
        terminal->fg_color = Color::white();
        terminal->clear();

        serial->write_line("Zep OS: framebuffer up");
    } else {
        __terminal = nullptr;

        serial->write_line("Zep OS: no framebuffer");
    }

    serial->write_line("Zep OS: boot complete");

    main();

    return EFI_SUCCESS;
}
}

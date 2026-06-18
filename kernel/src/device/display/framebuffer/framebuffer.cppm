module;

#include "std/runtime.h"

#include <efi.h>

export module zep.device.display.framebuffer;

import zep.device;
import zep.std.types;

export class Framebuffer : Device {
  public:
    u8* base;

    u64 width;
    u64 height;

    u64 pitch;
    u16 bpp;

    explicit Framebuffer(void* address, u64 width, u64 height, u64 pitch, u16 bpp)
        : base(static_cast<u8*>(address)), width(width), height(height), pitch(pitch), bpp(bpp) {}

    string name() override { return "framebuffer"; }

    void write(u64 x, u64 y, u32 color) {
        if (x >= width || y >= height) {
            return;
        }
        auto offset = y * pitch + x * 4;
        auto* pixel = reinterpret_cast<u32*>(base + offset);
        *pixel = color;
    }

    void clear(u32 color) {
        for (u64 y = 0; y < height; ++y) {
            for (u64 x = 0; x < width; ++x) {
                write(x, y, color);
            }
        }
    }
};

alignas(Framebuffer) static unsigned char framebuffer_storage[sizeof(Framebuffer)];

export Framebuffer* init_framebuffer(EFI_SYSTEM_TABLE* system_table) {
    EFI_GUID guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = nullptr;

    EFI_STATUS status =
        system_table->BootServices->LocateProtocol(&guid, nullptr, reinterpret_cast<VOID**>(&gop));

    Framebuffer* framebuffer = nullptr;
    if (status == 0 && gop != nullptr && gop->Mode != nullptr && gop->Mode->Info != nullptr) {
        auto address = reinterpret_cast<void*>(static_cast<uintptr>(gop->Mode->FrameBufferBase));

        auto width = static_cast<u64>(gop->Mode->Info->HorizontalResolution);
        auto height = static_cast<u64>(gop->Mode->Info->VerticalResolution);
        auto pitch = static_cast<u64>(gop->Mode->Info->PixelsPerScanLine) * 4;

        framebuffer = new (framebuffer_storage) Framebuffer(address, width, height, pitch, 32);
    }

    return framebuffer;
}

export extern "C" void zep_framebuffer_write(Framebuffer* framebuffer, u64 x, u64 y, u32 color) {
    framebuffer->write(x, y, color);
}

export extern "C" void zep_framebuffer_clear(Framebuffer* framebuffer, u32 color) {
    framebuffer->clear(color);
}

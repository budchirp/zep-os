module;

#include "runtime/runtime.h"

#include <efi.h>

export module zep.device.gpu.framebuffer;

import zep.device;
import zep.std.types;
import zep.std.math;

export class Framebuffer : Device {
  public:
    u8* base;

    Vec2u64 resolution;

    u64 pitch;
    u16 bpp;

    explicit Framebuffer(void* address, Vec2u64 resolution, u64 pitch, u16 bpp)
        : base(static_cast<u8*>(address)), resolution(resolution), pitch(pitch), bpp(bpp) {}

    string name() override { return "framebuffer"; }

    void write(Vec2u64 position, u32 color) {
        if (position.x >= resolution.x || position.y >= resolution.y) {
            return;
        }

        auto offset = position.y * pitch + position.x * 4;
        *reinterpret_cast<u32*>(base + offset) = color;
    }

    void clear(u32 color) {
        for (u64 y = 0; y < resolution.y; ++y) {
            for (u64 x = 0; x < resolution.x; ++x) {
                write(Vec2u64(x, y), color);
            }
        }
    }

    Vec2u64 size() const { return resolution; }
};

alignas(Framebuffer) static unsigned char framebuffer_storage[sizeof(Framebuffer)];

export Framebuffer* init_framebuffer(EFI_SYSTEM_TABLE* system_table) {
    EFI_GUID guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = nullptr;

    EFI_STATUS status =
        system_table->BootServices->LocateProtocol(&guid, nullptr, reinterpret_cast<VOID**>(&gop));

    if (status != EFI_SUCCESS || gop == nullptr || gop->Mode == nullptr ||
        gop->Mode->Info == nullptr) {
        return nullptr;
    }

    auto address = reinterpret_cast<void*>(static_cast<uintptr>(gop->Mode->FrameBufferBase));

    auto resolution = Vec2u64(static_cast<u64>(gop->Mode->Info->HorizontalResolution),
                              static_cast<u64>(gop->Mode->Info->VerticalResolution));

    auto pitch = static_cast<u64>(gop->Mode->Info->PixelsPerScanLine) * 4;

    return new (framebuffer_storage) Framebuffer(address, resolution, pitch, 32);
}

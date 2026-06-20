module;

#include "runtime/runtime.h"

#include <efi.h>

export module zep.device.gpu.framebuffer;

import zep.device;
import zep.std.types;
import zep.std.math;

export class Framebuffer : public Device {
  private:
  public:
    using Device::write;

    u8* front = nullptr;
    u8* back = nullptr;

    Vec2u64 resolution;

    u64 pitch;
    u16 bpp;

    explicit Framebuffer(u8* front, u8* back, Vec2u64 resolution, u64 pitch, u16 bpp)
        : front(front), back(back), resolution(resolution), pitch(pitch), bpp(bpp) {}

    ~Framebuffer() override = default;

    string name() override { return "framebuffer"; }

    void write(Vec2u64 position, u32 color) {
        if (position.x >= resolution.x || position.y >= resolution.y) {
            return;
        }

        u64 offset = position.y * pitch + position.x * 4;

        *reinterpret_cast<u32*>(back + offset) = color;
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

export Framebuffer* init_framebuffer(EFI_SYSTEM_TABLE* system_table) {
    if (system_table == nullptr) {
        return nullptr;
    }

    EFI_GUID guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = nullptr;

    EFI_STATUS status =
        system_table->BootServices->LocateProtocol(&guid, nullptr, reinterpret_cast<VOID**>(&gop));

    if (status != EFI_SUCCESS || gop == nullptr || gop->Mode == nullptr ||
        gop->Mode->Info == nullptr) {
        return nullptr;
    }

    u8* front = reinterpret_cast<u8*>(static_cast<uintptr>(gop->Mode->FrameBufferBase));

    Vec2u64 resolution = Vec2u64(static_cast<u64>(gop->Mode->Info->HorizontalResolution),
                                 static_cast<u64>(gop->Mode->Info->VerticalResolution));

    u64 pitch = static_cast<u64>(gop->Mode->Info->PixelsPerScanLine) * 4;

    u8* back = new u8[pitch * resolution.y];

    if (back == nullptr) {
        return nullptr;
    }

    return new Framebuffer(front, back, resolution, pitch, 32);
}

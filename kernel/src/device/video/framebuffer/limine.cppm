module;
#include "boot/limine/info.h"
#include "../../../std/runtime.h"

export module zep.device.video.framebuffer.limine;

import zep.std.types;
import zep.gfx.framebuffer;
import zep.device;

export class LimineFramebufferDevice : Device<Framebuffer*> {
    LimineInfo::Framebuffer& info;

  public:
    explicit LimineFramebufferDevice(LimineInfo::Framebuffer& info) : info(info) {}

    Framebuffer* init() override {
        return new Framebuffer(info.address, info.width, info.height, info.pitch, info.bpp);
    }
};

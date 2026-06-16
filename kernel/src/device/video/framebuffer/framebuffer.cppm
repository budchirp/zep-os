module;
#include "boot/limine/info.h"

export module zep.device.video.framebuffer;

import zep.std.types;
import zep.gfx.framebuffer;
import zep.device;
import zep.device.video.framebuffer.limine;

export class FramebufferDevice : Device<Framebuffer*> {
  private:
    LimineInfo& info;

  public:
    explicit FramebufferDevice(LimineInfo& info) : info(info) {}

    Framebuffer* init() override {
        if (info.framebuffer.address != nullptr) {
            LimineFramebufferDevice limine(info.framebuffer);
            return limine.init();
        }
        return nullptr;
    }
};

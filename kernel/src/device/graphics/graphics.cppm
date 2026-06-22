module;

#include "runtime/runtime.h"

export module zep.device.graphics;

import zep.std.types;
import zep.std.math;
import zep.device.graphics.framebuffer;
import zep.gfx.renderer;
import zep.gfx.terminal;
import zep.context;
import zep.common.logger;
import zep.device;
import zep.boot.info;

export void init_graphics(BootInfo* boot_info) {
    auto* context = get_context();

    if (boot_info->framebuffer.address == nullptr) {
        context->logger->log("no framebuffer");
        return;
    }

    u8* front = reinterpret_cast<u8*>(boot_info->framebuffer.address);
    Vec2u64 resolution = Vec2u64(boot_info->framebuffer.width, boot_info->framebuffer.height);
    u64 pitch = boot_info->framebuffer.pitch;

    u8* back = new u8[pitch * resolution.y];
    if (back == nullptr) {
        context->logger->log("Failed to allocate back buffer");
        return;
    }

    auto* framebuffer = new Framebuffer(front, back, resolution, pitch, boot_info->framebuffer.bpp);

    context->device_manager->add(framebuffer);

    context->renderer = init_renderer(framebuffer);
    context->logger->terminal = init_terminal(context->renderer, framebuffer->size());

    context->logger->log("framebuffer up");
}

module;

#include "runtime/runtime.h"

#include <efi.h>

export module zep.device.graphics;

import zep.std.types;
import zep.std.math;
import zep.device.graphics.framebuffer;
import zep.gfx.renderer;
import zep.gfx.terminal;
import zep.context;
import zep.common.logger;
import zep.device;

export void init_graphics(EFI_SYSTEM_TABLE* system_table) {
    auto* context = get_context();

    Framebuffer* framebuffer = init_framebuffer(system_table);
    if (framebuffer != nullptr) {
        context->device_manager->add(framebuffer);

        context->renderer = init_renderer(framebuffer);
        context->logger->terminal = init_terminal(context->renderer, framebuffer->size());

        context->logger->log("framebuffer up");
    } else {
        context->logger->log("no framebuffer");
    }
}

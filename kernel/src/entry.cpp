#include "runtime/runtime.h"

#include <efi.h>
#include <efiprot.h>

import zep.std.types;
import zep.std;
import zep.device.serial;
import zep.gfx.terminal;
import zep.gfx.renderer;
import zep.gfx.color;
import zep.device.gpu.framebuffer;
import zep.std.math;
import zep.context;
import zep.common.logger;

extern "C" {

extern u64 main(Context* context);

EFI_STATUS _entry(void*, EFI_SYSTEM_TABLE* system_table) {
    init_context();

    auto* context = get_context();

    context->logger = init_logger();
    context->logger->serial = init_serial(system_table);

    context->logger->log("serial up");

    context->framebuffer = init_framebuffer(system_table);
    if (context->framebuffer != nullptr) {
        context->renderer = init_renderer(context->framebuffer);

        context->logger->terminal = init_terminal(context->renderer, context->framebuffer->size());

        context->logger->log("framebuffer up");
    } else {
        context->logger->log("no framebuffer");
    }

    context->logger->log("boot complete");

    context->logger->switch_to_graphics();

    main(context);

    return EFI_SUCCESS;
}
}

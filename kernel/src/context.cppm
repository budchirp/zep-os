module;

#include "runtime/runtime.h"

export module zep.context;

import zep.device.serial;
import zep.gfx.terminal;
import zep.gfx.renderer;
import zep.device.gpu.framebuffer;
import zep.common.logger;

export class Context {
  public:
    Logger* logger;

    Framebuffer* framebuffer;

    Renderer* renderer;
    Terminal* terminal;
};

static Context* context = nullptr;

alignas(Context) static unsigned char context_storage[sizeof(Context)];

export void init_context() {
    context = new (context_storage) Context{};
}

export extern "C" Context* get_context() {
    return context;
}

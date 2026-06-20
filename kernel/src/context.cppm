module;

#include "runtime/runtime.h"

export module zep.context;

import zep.device.serial;
import zep.gfx.terminal;
import zep.gfx.renderer;
import zep.device.gpu.framebuffer;
import zep.common.logger;
import zep.device;

export class Context {
  public:
    Logger* logger = nullptr;
    Renderer* renderer = nullptr;
    Terminal* terminal = nullptr;
    DeviceManager* device_manager = nullptr;
};

static Context* context = nullptr;

alignas(Context) static unsigned char context_storage[sizeof(Context)];

export void init_context() {
    context = new (context_storage) Context{};
}

export extern "C" Context* get_context() {
    return context;
}

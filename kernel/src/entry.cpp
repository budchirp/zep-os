#include "runtime/runtime.h"

#include <efi.h>
#include <efiprot.h>

import zep.std.types;
import zep.std;
import zep.device.serial;
import zep.gfx.terminal;
import zep.gfx.renderer;
import zep.gfx.color;
import zep.device.graphics;
import zep.std.math;
import zep.context;
import zep.common.logger;
import zep.device;
import zep.memory;
import zep.fs;
import zep.fs.fat;
import zep.device.nvme;
import zep.std.string_view;
import zep.test;

extern "C" {

extern u64 main(Context* context);

EFI_STATUS _entry(void* image, EFI_SYSTEM_TABLE* system_table) {
    (void)image;

    init_context();
    auto* context = get_context();

    context->logger = init_logger();

    Serial* serial = init_serial(system_table);
    context->logger->serial = serial;

    context->logger->log("serial up");

    context->device_manager = init_device_manager();

    init_memory(system_table);
    context->logger->log("memory map captured");

    init_graphics(system_table);

    auto* nvme = init_nvme(system_table);
    if (nvme == nullptr) {
        panic("Failed to initialize NVMe driver");
    }
    context->device_manager->add(nvme);

    auto* fat_fs = new Fat32FileSystem(nvme);
    if (!fat_fs->init()) {
        panic("Failed to initialize FAT32 filesystem");
    }

    context->device_manager->add(StringView("fat"), fat_fs);

    auto* fs = new FileSystem(fat_fs);
    context->device_manager->add(StringView("fs"), fs);

    test_fat_filesystem();

    context->logger->log("boot complete");

    context->logger->switch_to_graphics();

    if (context->renderer != nullptr) {
        context->renderer->sync();
    }

    main(context);

    return EFI_SUCCESS;
}
}

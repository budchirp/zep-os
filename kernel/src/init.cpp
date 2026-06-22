#include "runtime/runtime.h"

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
import zep.boot.info;

extern "C" {

extern void main(Context* context);

void kernel_init(BootInfo* boot_info) {
    init_context();
    auto* context = get_context();

    context->logger = init_logger();

    Serial* serial = init_serial(boot_info->console);
    context->logger->serial = serial;

    context->logger->log("serial up");

    context->device_manager = init_device_manager();

    init_memory(boot_info);
    context->logger->log("memory map captured");

    init_graphics(boot_info);

    NvmeDriver* primary_disk = nullptr;
    for (usize i = 0; i < boot_info->disk_count; ++i) {
        auto* nvme = init_nvme(boot_info->disks[i].address, boot_info->disks[i].size);
        if (nvme == nullptr) {
            panic("Failed to initialize NVMe driver");
        }

        context->device_manager->add(nvme);
        if (primary_disk == nullptr) {
            primary_disk = nvme;
        }
    }

    if (primary_disk == nullptr) {
        panic("No NVMe disk found");
    }

    auto* fat_fs = new Fat32FileSystem(primary_disk);
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
}
}

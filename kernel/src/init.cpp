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
import zep.memory.vmm;
import zep.system.gdt;
import zep.system.interrupts;
import zep.system.scheduler;
import zep.fs;
import zep.fs.fat;
import zep.device.nvme;
import zep.std.string_view;
import zep.boot.info;
import zep.system.syscalls;
import zep.system.elf;
import zep.arch;

extern "C" {

void kernel_init(BootInfo* boot_info) {
    init_context();
    auto* context = get_context();

    context->logger = init_logger();

    Serial* serial = init_serial(boot_info->console);
    context->logger->serial = serial;

    context->logger->log(StringView("serial up"));

    GdtManager::init();
    context->logger->log(StringView("gdt/tss up"));

    context->device_manager = init_device_manager();

    init_memory(boot_info);
    context->logger->log(StringView("memory map captured"));

    init_vmm(boot_info);
    context->logger->log(StringView("vmm up"));

    InterruptManager::init();
    context->logger->log(StringView("idt/interrupts up"));

    init_graphics(boot_info);

    NvmeDriver* primary_disk = nullptr;
    for (usize i = 0; i < boot_info->disk_count; ++i) {
        auto* nvme = init_nvme(boot_info->disks[i].address, boot_info->disks[i].size);
        if (nvme == nullptr) {
            panic(StringView("Failed to initialize NVMe driver"));
        }

        context->device_manager->add(nvme);
        if (primary_disk == nullptr) {
            primary_disk = nvme;
        }
    }

    if (primary_disk == nullptr) {
        panic(StringView("No NVMe disk found"));
    }

    auto* fat_fs = new Fat32FileSystem(primary_disk);
    if (!fat_fs->init()) {
        panic(StringView("Failed to initialize FAT32 filesystem"));
    }

    context->device_manager->add(StringView("fat"), fat_fs);

    auto* fs = new FileSystem(fat_fs);
    context->device_manager->add(StringView("fs"), fs);

    context->logger->log(StringView("boot complete"));

    context->logger->switch_to_graphics();

    init_syscalls();

    Thread boot_thread;
    Scheduler::add_thread(&boot_thread);

    u32 init_pid = Scheduler::spawn("/System/Binaries/init", nullptr, 0);
    if (init_pid == 0) {
        panic(StringView("Failed to spawn /System/Binaries/init"));
    }

    context->logger->log(StringView("Starting scheduler..."));
    while (true) {
        thread_yield();
    }
}
}

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
import zep.device;
import zep.memory;
import zep.fs;
import zep.fs.fat;
import zep.device.nvme;
import zep.std.string_view;

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

    PageAllocator page_allocator(system_table->BootServices);

    void* heap_mem = page_allocator.allocate_pages(4096);

    if (heap_mem == nullptr) {
        panic("Failed to allocate heap memory");
    }

    init_heap(heap_mem, 16 * 1024 * 1024);

    Framebuffer* framebuffer = init_framebuffer(system_table);
    if (framebuffer != nullptr) {
        context->device_manager->add(framebuffer);

        context->renderer = init_renderer(framebuffer);
        context->logger->terminal = init_terminal(context->renderer, framebuffer->size());

        context->logger->log("framebuffer up");
    } else {
        context->logger->log("no framebuffer");
    }

    NvmeDriver* nvme = init_nvme(system_table);
    if (nvme == nullptr) {
        panic("Failed to initialize NVMe driver");
    }
    context->device_manager->add(nvme);

    Fat32FileSystem* fat_fs = new Fat32FileSystem(nvme);
    if (!fat_fs->init()) {
        panic("Failed to initialize FAT32 filesystem");
    }

    context->device_manager->add(StringView("fat"), fat_fs);

    FileSystem* fs = new FileSystem(fat_fs);

    context->device_manager->add(StringView("fs"), fs);

    context->logger->log("Verifying FileSystem middle-man APIs...");

    u8 read_buffer[64] = {0};

    usize bytes_read = fs->read(StringView("HELLO.TXT"), 0, read_buffer, 63);

    read_buffer[bytes_read] = '\0';

    context->logger->log("HELLO.TXT read via FileSystem:");

    context->logger->log(reinterpret_cast<string>(read_buffer));

    context->logger->log("Creating NEWFILE.TXT...");

    if (fs->create(StringView("NEWFILE.TXT"))) {
        context->logger->log("NEWFILE.TXT created successfully!");
    } else {
        context->logger->log("Failed to create NEWFILE.TXT!");
    }

    context->logger->log("Deleting NEWFILE.TXT...");

    if (fs->remove(StringView("NEWFILE.TXT"))) {
        context->logger->log("NEWFILE.TXT deleted successfully!");
    } else {
        context->logger->log("Failed to delete NEWFILE.TXT!");
    }

    context->logger->log("boot complete");

    context->logger->switch_to_graphics();

    if (context->renderer != nullptr) {
        context->renderer->sync();
    }

    main(context);

    return EFI_SUCCESS;
}
}

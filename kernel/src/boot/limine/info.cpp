#include "info.h"

#include "limine.h"

#include <stddef.h>
#include <stdint.h>

static_assert(offsetof(limine_memmap_entry, base) == 0, "");
static_assert(offsetof(limine_memmap_entry, length) == 8, "");
static_assert(offsetof(limine_memmap_entry, type) == 16, "");

volatile limine_requests_start_marker_type limine_requests_start_marker
    __attribute__((used, section(".limine_requests_start_marker"))) = LIMINE_REQUESTS_START_MARKER;

volatile limine_requests_end_marker_type limine_requests_end_marker
    __attribute__((used, section(".limine_requests_end_marker"))) = LIMINE_REQUESTS_END_MARKER;

volatile limine_base_revision_type limine_base_revision
    __attribute__((used, section(".limine_requests"))) = LIMINE_BASE_REVISION(6);

static volatile limine_framebuffer_request framebuffer_request
    __attribute__((section(".limine_requests"))) = {.id = LIMINE_FRAMEBUFFER_REQUEST_ID,
                                                    .revision = 0};

static volatile limine_memmap_request memmap_request
    __attribute__((section(".limine_requests"))) = {
        .id = LIMINE_MEMMAP_REQUEST_ID,
        .revision = 0,
        .response = nullptr,
};

static volatile limine_hhdm_request hhdm_request __attribute__((section(".limine_requests"))) = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0,
    .response = nullptr,
};

extern "C" void init();

static volatile limine_entry_point_request entry_point_request
    __attribute__((used, section(".limine_requests"))) = {
        .id = LIMINE_ENTRY_POINT_REQUEST_ID,
        .revision = 0,
        .response = nullptr,
        .entry = init,
};

LimineInfo LimineInfo::get() {
    LimineInfo info;

    if (framebuffer_request.response != nullptr &&
        framebuffer_request.response->framebuffer_count > 0) {
        auto* limine_fb = framebuffer_request.response->framebuffers[0];
        info.framebuffer.address = limine_fb->address;
        info.framebuffer.width = limine_fb->width;
        info.framebuffer.height = limine_fb->height;
        info.framebuffer.pitch = limine_fb->pitch;
        info.framebuffer.bpp = limine_fb->bpp;
    }

    if (hhdm_request.response != nullptr) {
        info.hhdm_offset = hhdm_request.response->offset;
    }

    if (memmap_request.response != nullptr) {
        auto count = memmap_request.response->entry_count;
        info.memory_map_count = count;

        static LimineInfo::MemoryRange translated[256];

        for (uint64_t i = 0; i < count && i < 256; ++i) {
            auto* entry = memmap_request.response->entries[i];
            translated[i].base = entry->base;
            translated[i].length = entry->length;

            switch (entry->type) {
            case LIMINE_MEMMAP_USABLE:
                translated[i].type = LimineInfo::MemoryRange::Type::Usable;
                break;
            case LIMINE_MEMMAP_RESERVED:
            case LIMINE_MEMMAP_RESERVED_MAPPED:
                translated[i].type = LimineInfo::MemoryRange::Type::Reserved;
                break;
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
                translated[i].type = LimineInfo::MemoryRange::Type::AcpiReclaimable;
                break;
            case LIMINE_MEMMAP_ACPI_NVS:
                translated[i].type = LimineInfo::MemoryRange::Type::AcpiNvs;
                break;
            case LIMINE_MEMMAP_BAD_MEMORY:
                translated[i].type = LimineInfo::MemoryRange::Type::BadMemory;
                break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
                translated[i].type = LimineInfo::MemoryRange::Type::BootloaderReclaimable;
                break;
            case LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
                translated[i].type = LimineInfo::MemoryRange::Type::KernelModule;
                break;
            case LIMINE_MEMMAP_FRAMEBUFFER:
                translated[i].type = LimineInfo::MemoryRange::Type::Framebuffer;
                break;
            default:
                translated[i].type = LimineInfo::MemoryRange::Type::Reserved;
                break;
            }
        }

        info.memory_map = translated;
    }

    return info;
}

#ifndef ZEP_BOOT_LIMINE_H
#define ZEP_BOOT_LIMINE_H

#include <stddef.h>
#include <stdint.h>

#define LIMINE_REQUESTS_START_MARKER                                                               \
    {0xf6b8f4b39de6d1ae, 0xfab91a6940fcb9cf, 0x785c6ed015d3e75e, 0x181e920a7852f9e5}
#define LIMINE_REQUESTS_END_MARKER {0x4e3b6c28af2a2a4c, 0x7b5c0b5c5e5c5e5c}
#define LIMINE_BASE_REVISION(N) {0xf9562b2d5c95a6c8, 0x6a7b384944536bdc, (N)}

typedef uint64_t limine_requests_start_marker_type[4];
typedef uint64_t limine_requests_end_marker_type[2];
typedef uint64_t limine_base_revision_type[3];

class limine_uuid {
  public:
    uint32_t a;
    uint16_t b;
    uint16_t c;
    uint8_t d[8];
};

constexpr limine_uuid LIMINE_FRAMEBUFFER_REQUEST_ID = {
    0xcbfe81d9, 0x0d51, 0x4762, {0x90, 0xde, 0x4c, 0x40, 0x09, 0x50, 0x42, 0x82}};
constexpr limine_uuid LIMINE_MEMMAP_REQUEST_ID = {
    0x67cf3d9e, 0x38c8, 0x454f, {0x97, 0x51, 0x9c, 0x8f, 0x5d, 0x68, 0x22, 0xb9}};
constexpr limine_uuid LIMINE_HHDM_REQUEST_ID = {
    0x48dcf1cb, 0x8dab, 0x4ee1, {0xb9, 0x1d, 0x00, 0xc0, 0x80, 0x2b, 0x37, 0x59}};
constexpr limine_uuid LIMINE_ENTRY_POINT_REQUEST_ID = {
    0x13d86c49, 0x4bcb, 0x4e38, {0x8a, 0x97, 0xe0, 0x60, 0x77, 0x81, 0x50, 0x64}};

class limine_framebuffer_request {
  public:
    limine_uuid id;
    uint64_t revision;
    volatile class limine_framebuffer_response* response;
};

class limine_framebuffer {
  public:
    void* address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint16_t bpp;
    uint8_t memory_model;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
    uint8_t unused[7];
    uint64_t edid_size;
    void* edid;
    uint64_t mode_count;
    class limine_video_mode** modes;
};

class limine_framebuffer_response {
  public:
    uint64_t revision;
    uint64_t framebuffer_count;
    class limine_framebuffer** framebuffers;
};

class limine_video_mode {
  public:
    uint64_t width;
    uint64_t height;
    uint16_t bpp;
    uint8_t memory_model;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
};

class limine_memmap_request {
  public:
    limine_uuid id;
    uint64_t revision;
    volatile class limine_memmap_response* response;
};

class limine_memmap_entry {
  public:
    uint64_t base;
    uint64_t length;
    uint64_t type;
};

class limine_memmap_response {
  public:
    uint64_t revision;
    uint64_t entry_count;
    class limine_memmap_entry** entries;
};

class limine_hhdm_request {
  public:
    limine_uuid id;
    uint64_t revision;
    volatile class limine_hhdm_response* response;
};

class limine_hhdm_response {
  public:
    uint64_t revision;
    uint64_t offset;
};

class limine_entry_point_request {
  public:
    limine_uuid id;
    uint64_t revision;
    volatile class limine_entry_point_response* response;
    void (*entry)(void);
};

class limine_entry_point_response {
  public:
    uint64_t revision;
};

#define LIMINE_MEMMAP_USABLE 0
#define LIMINE_MEMMAP_RESERVED 1
#define LIMINE_MEMMAP_RESERVED_MAPPED 2
#define LIMINE_MEMMAP_ACPI_RECLAIMABLE 3
#define LIMINE_MEMMAP_ACPI_NVS 4
#define LIMINE_MEMMAP_BAD_MEMORY 5
#define LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE 6
#define LIMINE_MEMMAP_EXECUTABLE_AND_MODULES 7
#define LIMINE_MEMMAP_FRAMEBUFFER 8

#endif

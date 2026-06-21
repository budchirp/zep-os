module;

#include "runtime/runtime.h"

export module zep.test;

import zep.std.types;
import zep.std.string_view;
import zep.context;
import zep.device;
import zep.fs;
import zep.common.logger;

export void test_fat_filesystem() {
    auto* context = get_context();

    context->logger->log("Verifying FileSystem middle-man APIs...");

    auto* fs = context->device_manager->get<FileSystem>(StringView("fs"));
    if (fs == nullptr) {
        context->logger->log("Error: FileSystem not found in device manager!");
        return;
    }

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
}

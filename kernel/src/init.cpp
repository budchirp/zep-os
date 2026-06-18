#include "std/runtime.h"

#include <efi.h>
#include <efiprot.h>

import zep.std.types;
import zep.std;
import zep.device.serial;
import zep.gfx.terminal;
import zep.device.display.framebuffer;

static Serial* serial = nullptr;
static Terminal* terminal = nullptr;

extern "C" {

void* __serial = nullptr;
void* __terminal = nullptr;

extern void main();

EFI_STATUS _entry(EFI_HANDLE, EFI_SYSTEM_TABLE* system_table) {
    serial = init_serial(system_table);
    __serial = serial;

    serial->write("Zep OS: serial up\n");

    auto framebuffer = init_framebuffer(system_table);
    if (framebuffer != nullptr) {
        terminal = init_terminal(framebuffer);
        __terminal = terminal;
    } else {
        serial->write("Zep OS: no framebuffer\n");
    }

    serial->write("Zep OS: boot complete\n");

    main();

    return EFI_SUCCESS;
}
}

module;

#include "runtime/runtime.h"

export module zep.device;

import zep.std.types;
import zep.std.string_view;
import zep.std.map;

export class Device {
  public:
    Device() = default;
    virtual ~Device() {}

    virtual StringView name() = 0;

    virtual usize read(usize offset, u8* dest, usize size) {
        (void)offset;
        (void)dest;
        (void)size;
        return 0;
    }

    virtual usize write(usize offset, const u8* src, usize size) {
        (void)offset;
        (void)src;
        (void)size;
        return 0;
    }
};

export class DeviceManager {
  private:
    Map<void*> devices;

  public:
    DeviceManager() = default;

    bool add(Device* device) {
        if (device == nullptr) {
            return false;
        }

        return devices.insert(device->name(), static_cast<void*>(device));
    }

    bool add(StringView name, void* ptr) {
        if (ptr == nullptr) {
            return false;
        }

        return devices.insert(name, ptr);
    }

    template <typename T>
    T* get(StringView name) {
        void* ptr = devices.find(name);

        if (ptr == nullptr) {
            return nullptr;
        }

        return static_cast<T*>(ptr);
    }
};

alignas(DeviceManager) static unsigned char device_manager_storage[sizeof(DeviceManager)];

export DeviceManager* init_device_manager() {
    return new (device_manager_storage) DeviceManager();
}

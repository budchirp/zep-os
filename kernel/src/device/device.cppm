export module zep.device;

import zep.std.types;

export template <typename T>
class Device {
  public:
    virtual ~Device() = default;
    virtual T init() = 0;
};

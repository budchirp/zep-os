export module zep.device;

import zep.std.types;

export class Device {
  public:
    Device() = default;
    virtual ~Device() = 0;

    virtual string name() = 0;
};

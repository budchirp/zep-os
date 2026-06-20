export module zep.fs;

import zep.std.types;
import zep.std.string_view;

export class FileSystemDriver {
  private:
  public:
    FileSystemDriver() = default;
    virtual ~FileSystemDriver() = default;

    virtual usize read_file(StringView path, usize offset, u8* buffer, usize size) = 0;
    virtual bool create_file(StringView path) = 0;
    virtual bool delete_file(StringView path) = 0;
};

export class FileSystem {
  private:
    FileSystemDriver* driver = nullptr;

  public:
    explicit FileSystem(FileSystemDriver* driver) : driver(driver) {}

    usize read(StringView path, usize offset, u8* buffer, usize size) {
        if (driver == nullptr) {
            return 0;
        }

        return driver->read_file(path, offset, buffer, size);
    }

    bool create(StringView path) {
        if (driver == nullptr) {
            return false;
        }

        return driver->create_file(path);
    }

    bool remove(StringView path) {
        if (driver == nullptr) {
            return false;
        }

        return driver->delete_file(path);
    }
};

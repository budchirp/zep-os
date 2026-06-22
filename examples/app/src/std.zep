public extern fn print(str: cstr) -> void
public extern fn panic(str: cstr) -> never
public extern fn halt() -> never
public extern fn read_file(path: cstr, buffer: u64, size: u64, offset: u64) -> i64
public extern fn allocate(size: u64) -> u64
public extern fn free(ptr: u64) -> void
public extern fn print_buffer(buf: u64) -> void
public extern fn exit(code: i32) -> never

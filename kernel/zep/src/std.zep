public extern fn print(str: cstr) -> void
public extern fn panic(str: cstr) -> never

public extern fn halt() -> never

public extern fn kernel_allocate(size: u64) -> *u8
public extern fn kernel_deallocate(ptr: *u8) -> void
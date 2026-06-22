import std { exit, print, read_file, allocate, free, print_buffer }

public fn main() -> never {
    print("Example process running.")

    var buf: u64 = allocate(64)

    // 1. Try to read /HELLO.TXT
    var res1: i64 = read_file("/HELLO.TXT", buf, 64, 0)
    if (res1 >= 0) {
        print("Read /HELLO.TXT succeeded: ")
        print_buffer(buf)
    } else {
        print("Read /HELLO.TXT failed (ERROR!).")
    }

    // 2. Try to read /Config/users
    var res2: i64 = read_file("/Config/users", buf, 64, 0)
    if (res2 >= 0) {
        print("Read /Config/users succeeded: ")
        print_buffer(buf)
    } else {
        print("Read /Config/users failed (ERROR!).")
    }

    free(buf)
    print("Example process exiting.")
    exit(0)
}

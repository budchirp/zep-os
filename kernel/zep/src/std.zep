import global { logger }

public extern fn zep_panic() -> never
public extern fn zep_halt() -> never

public fn panic(str: cstr) -> never {
    logger.print("PANIC")
    logger.print(str)

    zep_panic()
}

public fn halt() -> never {
    zep_halt()
}
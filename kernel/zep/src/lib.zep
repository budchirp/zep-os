import std { halt, print, kernel_allocate, kernel_deallocate }

import context { Context }

public fn main(context: *Context) -> never {
    print("Hi!")

    halt()
}
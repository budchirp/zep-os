public struct Framebuffer {
    public:
        var base: *u8

        var width: u64
        var height: u64

        var pitch: u64
        var bpp: u16
}
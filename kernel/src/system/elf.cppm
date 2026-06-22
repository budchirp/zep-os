export module zep.system.elf;

import zep.std.types;
import zep.std.string_view;
import zep.std;
import zep.memory.vmm;

struct [[gnu::packed]] ElfHeader {
    u8 ident[16];
    u16 type;
    u16 machine;
    u32 version;
    u64 entry;
    u64 phoff;
    u64 shoff;
    u32 flags;
    u16 ehsize;
    u16 phentsize;
    u16 phnum;
    u16 shentsize;
    u16 shnum;
    u16 shstrndx;
};

struct [[gnu::packed]] ElfProgramHeader {
    u32 type;
    u32 flags;
    u64 offset;
    u64 vaddr;
    u64 paddr;
    u64 filesz;
    u64 memsz;
    u64 align;
};

export class ElfLoader {
  private:
    static void memset(void* dest, u8 val, usize size) {
        auto* d = reinterpret_cast<u8*>(dest);
        for (usize i = 0; i < size; ++i) {
            d[i] = val;
        }
    }

    static void memcpy(void* dest, const void* src, usize size) {
        auto* d = reinterpret_cast<u8*>(dest);
        const auto* s = reinterpret_cast<const u8*>(src);
        for (usize i = 0; i < size; ++i) {
            d[i] = s[i];
        }
    }

  public:
    static u64 load(const u8* elf_data, usize elf_size) {
        if (elf_size < sizeof(ElfHeader)) {
            panic(StringView("ELF loader: data too small"));
        }

        const auto* header = reinterpret_cast<const ElfHeader*>(elf_data);

        if (header->ident[0] != 0x7F || header->ident[1] != 'E' || header->ident[2] != 'L' ||
            header->ident[3] != 'F') {
            panic(StringView("ELF loader: invalid magic"));
        }

        auto* active_pml4 = get_active_pml4();
        PageTableManager manager(active_pml4);

        const auto* ph_base = elf_data + header->phoff;

        for (u16 i = 0; i < header->phnum; ++i) {
            const auto* ph =
                reinterpret_cast<const ElfProgramHeader*>(ph_base + i * header->phentsize);

            if (ph->type == 1) { // PT_LOAD
                u64 vaddr = ph->vaddr;
                u64 memsz = ph->memsz;
                u64 filesz = ph->filesz;
                u64 offset = ph->offset;

                u64 vaddr_aligned = vaddr & ~static_cast<u64>(4095);
                u64 vaddr_end = (vaddr + memsz + 4095) & ~static_cast<u64>(4095);
                usize num_pages = (vaddr_end - vaddr_aligned) / 4096;

                for (usize p = 0; p < num_pages; ++p) {
                    u64 page_vaddr = vaddr_aligned + p * 4096;

                    void* raw = allocate(4096 + 4095);
                    u64 addr = reinterpret_cast<u64>(raw);
                    u64 aligned_addr = (addr + 4095) & ~static_cast<u64>(4095);

                    memset(reinterpret_cast<void*>(aligned_addr), 0, 4096);

                    manager.map_page(page_vaddr, aligned_addr, 7);
                }

                if (filesz > 0) {
                    memcpy(reinterpret_cast<void*>(vaddr), elf_data + offset, filesz);
                }

                if (memsz > filesz) {
                    memset(reinterpret_cast<void*>(vaddr + filesz), 0, memsz - filesz);
                }
            }
        }

        return header->entry;
    }
};

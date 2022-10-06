#include "elf_loader.h"
#include "fs/file.h"
#include "status.h"
#include <stdbool.h>
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "string/string.h"
#include "memory/paging/paging.h"
#include "kernel.h"
#include "config.h"

const char elf_signature[] = {0x7f, 'E', 'L', 'F'};

static bool elf_valid_signature(void* buffer) {
    return memcmp(buffer, (void*)elf_signature, sizeof(elf_signature)) == 0;
}

static bool elf_valid_class(struct elf_header* elf_header) {
    // Only support 32 bit binaries
    return (elf_header->e_ident[EI_CLASS] == ELFCLASSNONE) ||
           (elf_header->e_ident[EI_CLASS] == ELFCLASS32);
}

static bool elf_valid_encoding(struct elf_header* elf_header) {
    return (elf_header->e_ident[EI_DATA] == ELFDATANONE) ||
           (elf_header->e_ident[EI_DATA] == ELFDATA2LSB);
}

static bool elf_is_executable(struct elf_header* elf_header) {
    return (elf_header->e_type == ET_EXEC) &&
           (elf_header->e_entry >= PEACHOS_PROGRAM_VIRTUAL_ADDRESS);
}

static bool elf_has_program_header(struct elf_header* elf_header) {
    return elf_header->e_phoff != 0;
}

void* elf_memory(struct elf_file* file) {
    return file->elf_memory;
}

struct elf_header* elf_header(struct elf_file* file) {
    return file->elf_memory;
}

struct elf32_shdr* elf_sheader(struct elf_header* elf_header) {
    return (struct elf32_shdr*)((int) elf_header + elf_header->e_shoff);
}

struct elf32_phdr* elf_pheader(struct elf_header* elf_header) {
    if (elf_header->e_phoff == 0) {
        return 0;
    }
    return (struct elf32_phdr*)((int) elf_header + elf_header->e_phoff);
}

struct elf32_phdr* elf_program_header_by_index(struct elf_header* elf_header, int index) {
    return &elf_pheader(elf_header)[index];
}

struct elf32_shdr* elf_section_header_by_index(struct elf_header* elf_header, int index) {
    return &elf_sheader(elf_header)[index];
}

void* elf_phdr_phys_address(struct elf_file* file, struct elf32_phdr* phdr) {
    return elf_memory(file) + phdr->p_offset;
}

char* str_table(struct elf_header* elf_header) {
    return (char*) elf_header + elf_section_header_by_index(elf_header, elf_header->e_shstrndx)->sh_offset;
}

void* elf_virtual_base(struct elf_file* file) {
    return file->virtual_base_address;
}

void* elf_virtual_end(struct elf_file* file) {
    return file->virtual_end_address;
}

void* elf_physical_base(struct elf_file* file) {
    return file->physical_base_address;
}

void* elf_physical_end(struct elf_file* file) {
    return file->physical_end_address;
}

int elf_validate_loaded(struct elf_header* elf_header) {
    return elf_valid_signature(elf_header) &&
           elf_valid_class(elf_header) &&
           elf_valid_encoding(elf_header) &&
           elf_has_program_header(elf_header) ? NO_ERROR : -ERROR_INVALID_FORMAT;
}

int elf_process_phdr_pt_load(struct elf_file* elf_file, struct elf32_phdr* phdr) {
    if (elf_file->virtual_base_address >= (void*)phdr->p_vaddr || elf_file->virtual_base_address == 0x00) {
        elf_file->virtual_base_address = (void*)phdr->p_vaddr;
        elf_file->physical_base_address = elf_memory(elf_file) + phdr->p_offset;
    }

    unsigned int end_virtual_address = phdr->p_vaddr + phdr->p_filesz;
    if (elf_file->virtual_end_address <= (void*) end_virtual_address || elf_file->virtual_end_address == 0x00) {
        elf_file->virtual_end_address = (void*)end_virtual_address;
        elf_file->physical_end_address = elf_memory(elf_file) + phdr->p_offset + phdr->p_filesz;
    }

    return 0;
}

int elf_process_pheader(struct elf_file* elf_file, struct elf32_phdr* phdr) {
    int res = 0;
    switch (phdr->p_type) {
        case PT_LOAD:
            res = elf_process_phdr_pt_load(elf_file, phdr);
            break; 
        default:
            break;
    }

    return res;
}

int elf_process_pheaders(struct elf_file* elf_file) {
    int res = 0;
    struct elf_header* header = elf_header(elf_file);
    for (int i = 0; i < header->e_phnum; i++)
    {
        struct elf32_phdr* phdr = elf_program_header_by_index(header, i);
        res = elf_process_pheader(elf_file, phdr);
        if (res < 0) {
            break;
        }
    }
    
    return res;
}

int elf_process_loaded(struct elf_file* elf_file) {
    int res = 0;
    struct elf_header* header = elf_header(elf_file);
    res = elf_validate_loaded(header);
    if (res < 0) {
        goto out;
    }

    res = elf_process_pheaders(elf_file);

    if (res < 0) {
        goto out;
    }

out:
    return res;
}

int elf_load(const char* filename, struct elf_file** file_out) {
    struct elf_file* elf_file = kzalloc(sizeof(struct elf_file));
    int fd = 0;
    int res = fopen(filename, "r");
    if (res <= 0) {
        goto out;
    }

    fd = res;
    struct file_stat stat;
    res = fstat(fd, &stat);
    if (res < 0) {
        goto out;
    }

    elf_file->elf_memory = kzalloc(stat.file_size);
    res = fread(elf_file->elf_memory, stat.file_size, 1, fd);
    if (res < 0) {
        goto out;
    }

    res = elf_process_loaded(elf_file);
    if (res < 0) {
        goto out;
    }

    *file_out = elf_file;
out:
    fclose(fd);
    return res;
}

void elf_close(struct elf_file* elf_file) {
    if (!elf_file) {
       return;
    }

    kfree(elf_file->elf_memory);
    kfree(elf_file);
}
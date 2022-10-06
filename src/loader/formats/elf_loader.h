#ifndef ELF_LOADER_H
#define ELF_LOADER_H

#include <stdint.h>
#include <stddef.h>

#include "elf.h"
#include "config.h"

struct elf_file {
    char filename[PEACHOS_MAX_PATH];
    int in_memory_size;
    /* Physical memory address that this ELF is loaded at */
    void* elf_memory;
    /* Virtual base address of this binary */
    void* virtual_base_address;
    /* Ending virtual address of this binar */
    void* virtual_end_address;
    /* Physical base address of this binar */
    void* physical_base_address;
    /* Physical end address of this binary */
    void* physical_end_address;
};
struct elf_header* elf_header(struct elf_file* file);
struct elf32_shdr* elf_sheader(struct elf_header* elf_header);
void* elf_memory(struct elf_file* file);
struct elf32_phdr* elf_pheader(struct elf_header* elf_header);
struct elf32_phdr* elf_program_header_by_index(struct elf_header* elf_header, int index);
struct elf32_shdr* elf_section_header_by_index(struct elf_header* elf_header, int index);
void* elf_phdr_phys_address(struct elf_file* file, struct elf32_phdr* phdr);


void* elf_virtual_base(struct elf_file* file);
void* elf_virtual_end(struct elf_file* file);
void* elf_physical_base(struct elf_file* file);
void* elf_physical_end(struct elf_file* file);

int elf_load(const char* filename, struct elf_file** file_out);
void elf_close(struct elf_file* elf_file);
#endif
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

struct elf32_phdr* elf_program_by_index(struct elf_header* elf_header, int index) {
    return elf_pheader(elf_header)[index];
}

struct elf32_shdr* elf_section_by_index(struct elf_header* elf_header, int index) {
    return elf_sheader(elf_header)[index];
}

char* str_table(struct elf_header* elf_header) {
    return (char*) elf_header + elf_section_by_index(elf_header, elf_header->e_shstrndx);
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
           elf_has_program_header(elf_header) ? NO_ERROR : ERROR_INVALID_ARG;
}
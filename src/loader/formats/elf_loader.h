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

#endif
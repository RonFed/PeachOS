#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "config.h"
#include "task.h"

#define PROCESS_FILETYPE_ELF    0
#define PROCESS_FILETYPE_BINARY 1

typedef unsigned char PROCESS_FILETYPE;


struct process {
    // The process id
    uint16_t id;

    char filename[PEACHOS_MAX_PATH];
    // The main process task
    struct task* task;

    // The memory (malloc) allocations of the process
    void* allocations[PEACHOS_MAX_PROCESS_ALLOCATIONS];

    PROCESS_FILETYPE filetype;

    // The physical pointer to process memory (code and data..)
    union {
        void* ptr;
        struct elf_file* elf_file;
    };
    
    // The physical pointer to stack memory
    void* stack;

    // The size of the data pointed by 'ptr'
    uint32_t size;

    struct keyboard_buffer {
        char buffer[PEACHOS_KEYBOARD_BUFFER_SIZE];
        int tail;
        int head;
    } keyboard;
};

int process_switch(struct process* process);
int process_load_for_slot(const char* filename, struct process** process, int process_slot);
int process_load_switch(const char* filename, struct process** process);
int process_load(const char* filename, struct process** process);
struct process* process_current();
struct process* process_get(int process_id);

#endif
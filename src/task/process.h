#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "config.h"
#include "task.h"

struct process {
    // The process id
    uint16_t id;

    char filename[PEACHOS_MAX_PATH];
    // The main process task
    struct task* task;

    // The memory (malloc) allocations of the process
    void* allocations[PEACHOS_MAX_PROCESS_ALLOCATIONS];

    // The physical pointer to process memory (code and data)
    void* ptr;

    // The physical pointer to stack memory
    void* stack;

    // The size of the data pointed by 'ptr'
    uint32_t size;
};

int process_load_for_slot(const char* filename, struct process** process, int process_slot);
int process_load(const char* filename, struct process** process);
#endif
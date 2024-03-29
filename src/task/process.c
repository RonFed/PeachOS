#include "process.h"
#include "memory/memory.h"
#include "status.h"
#include "task.h"
#include "memory/heap/kheap.h"
#include "fs/file.h"
#include "string/string.h"
#include "kernel.h"
#include "memory/paging/paging.h"
#include "loader/formats/elf_loader.h"

// The current process running
struct process* current_process = 0;

static struct process* processes[PEACHOS_MAX_PROCESSES] = {};

static void process_init(struct process* process) {
    memset(process, 0x00, sizeof(struct process));

}

struct process* process_current() {
    return current_process;
}

struct process* process_get(int process_id) {
    if (process_id < 0 || process_id >= PEACHOS_MAX_PROCESSES) {
        return NULL;
    }

    return processes[process_id];
}

int process_switch(struct process* process) {
    current_process = process;
    return 0;
}

static int process_find_free_allocation_index(struct process* process) {
    int res = -ERROR_NO_MEMORY;
    for (int i = 0; i < PEACHOS_MAX_PROCESS_ALLOCATIONS; i++)
    {
        if (process->allocations[i].ptr == 0) {
            res = i;
            break;
        }
    }

    return res;
}

void* process_malloc(struct process* process, size_t size) {
    int index = process_find_free_allocation_index(process);
    if (ISERROR(index)) {
        goto out_err;
    }
    
    void* ptr = kzalloc(size);
    if (!ptr) {
        goto out_err;
    }

    // Map the malloced memory
    // Currently only one task (thread), if multiple tasks - add mapping to all of them
    int res = paging_map_to(process->task->page_directory,
                            ptr,
                            ptr,
                            paging_align_address(ptr + size),
                            PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    
    if (ISERROR(res)) {
        goto out_err;
    }

    process->allocations[index].ptr = ptr;
    process->allocations[index].size = size;
    return ptr;

out_err:
    if (ptr) {
        kfree(ptr);
    }

    return 0;
}

static bool process_is_process_pointer(struct process* process, void* ptr){
    for (int i = 0; i < PEACHOS_MAX_PROCESS_ALLOCATIONS; i++)
    {
        if (process->allocations[i].ptr == ptr) {
            return true;
        }
    }
    
    return false;
}

static void process_remove_allocation(struct process* process, void* ptr) {
    for (int i = 0; i < PEACHOS_MAX_PROCESS_ALLOCATIONS; i++)
    {
        if (process->allocations[i].ptr == ptr) {
            process->allocations[i].ptr = 0x00;
            process->allocations[i].size = 0;
       }
    }
}

static struct process_allocation* proocess_get_allocation_by_address(struct process* process, void* addr) {
    for (int i = 0; i < PEACHOS_MAX_PROCESS_ALLOCATIONS; i++) {
        if (process->allocations[i].ptr == addr) {
            return &process->allocations[i];
        }
    }

    return 0;
}

int process_terminata_allocations(struct process* process) {
    for (int i = 0; i < PEACHOS_MAX_PROCESS_ALLOCATIONS; i++)
    {
        // Possible to just kfree here since the process will terminate
        // process_free also remove the paging mapping
        process_free(process, process->allocations[i].ptr);
    }

    return 0;
}

int process_free_binary_program_data(struct process* process) {
    kfree(process->ptr);
    return 0;
}

int process_free_elf_program_data(struct process* process) {
    elf_close(process->elf_file);
    return 0;
}

int process_free_program_data(struct process* process) {
    int res = 0;

    switch (process->filetype) {
    case PROCESS_FILETYPE_BINARY:
        res = process_free_binary_program_data(process);
        break;
    case PROCESS_FILETYPE_ELF:
        res = process_free_elf_program_data(process);
        break;
    default:
        res = -ERROR_INVALID_ARG;
        break;
    }

    return res;
}

void process_switch_to_any() {
    for (int i = 0; i < PEACHOS_MAX_PROCESSES; i++)
    {
        if (processes[i]) {
            process_switch(processes[i]);
            return;
        }
    }
    
    panic("No processes to switch to\n");
}

static void process_unlink(struct process* process) {
    processes[process->id] = 0x00;

    if (current_process == process) {
        process_switch_to_any();
    }
}

int process_terminate(struct process* process) {
    int res = 0;

    res = process_terminata_allocations(process);
    if (res < 0) {
        goto out;
    }

    res = process_free_program_data(process);
    if (res < 0) {
        goto out;
    }

    // free the process stack memory
    kfree(process->stack);
    // free the task
    task_free(process->task);
    // unlink the process from the processes array
    process_unlink(process);
    
    kfree((void*)process);
    print("process_terminate\n");
out:
    return res;
}

void process_get_arguments(struct process* process, int* argc, char*** argv) {
    *argc = process->arguments.argc;
    *argv = process->arguments.argv;
}

int process_count_command_arguments(struct command_argument* root_arg) {
    struct command_argument* current = root_arg;
    int i = 0;
    while (current)
    {
        i++;
        current = current->next;
    }
    
    return i;
}

int process_inject_arguments(struct process* process, struct command_argument* root_arg) {
    int res = 0;
    struct command_argument* current = root_arg;
    int i = 0;
    int argc = process_count_command_arguments(root_arg);
    if (argc == 0) {
        res = -ERROR_IO;
        goto out;
    }

    char** argv = process_malloc(process, sizeof(const char *) * argc);
    if (!argv) {
        res = -ERROR_NO_MEMORY;
        goto out;
    }

    while (current) {
        char* arg_str = process_malloc(process, sizeof(current->argument));
        if (!arg_str) {
            res = -ERROR_NO_MEMORY;
            goto out;
        }

        strncpy(arg_str, current->argument, sizeof(current->argument));
        argv[i] = arg_str;
        current = current->next;
        i++;
    }

    process->arguments.argc = argc;
    process->arguments.argv = argv;
out:
    return res;
}

void process_free(struct process* process, void* ptr) {
    // Unlink the pages from the process for the given address
    struct process_allocation* allocation = proocess_get_allocation_by_address(process, ptr);
    if (!allocation) {
        // not our pointer
        return;
    }

    // Remove paging mapping (flagged as not accessable)
    int res = paging_map_to(process->task->page_directory,
                            allocation->ptr,
                            allocation->ptr,
                            paging_align_address(allocation->ptr + allocation->size),
                            0x00);

    if (res < 0) {
        return;
    }

    // Remove the allocation from the saved allocations for the process
    process_remove_allocation(process, ptr);
    kfree(ptr);
}

static int process_load_binary(const char* filename, struct process* process) {
    int res = 0;
    void* program_data_ptr = 0x00;

    int fd = fopen(filename, "r");
    if (!fd) {
        res = -ERROR_IO;
        goto out;
    }

    struct file_stat stat;
    res = fstat(fd, &stat);
    if (res != NO_ERROR) {
        goto out;
    }

    program_data_ptr = kzalloc(stat.file_size);
    if (!program_data_ptr) {
        res = -ERROR_NO_MEMORY;
        goto out;
    }

    if (fread(program_data_ptr, stat.file_size, 1, fd) != 1) {
        res = -ERROR_IO;
        goto out;
    }

    process->filetype = PROCESS_FILETYPE_BINARY;
    process->ptr = program_data_ptr;
    process->size = stat.file_size;

out:
    if (res < 0) {
        if (program_data_ptr) {
            kfree(program_data_ptr);
        }
    }
    fclose(fd);
    return res;
}

static int process_load_elf(const char* filename, struct process* process) {
    int res = 0;
    struct elf_file* elf_file = 0;
    res = elf_load(filename, &elf_file);
    if (ISERROR(res)) {
        goto out;
    }

    process->filetype = PROCESS_FILETYPE_ELF;
    process->elf_file = elf_file;
out:
    return res;
}

static int process_load_data(const char* filename, struct process* process) {
    int res = 0;
    res = process_load_elf(filename, process);
    if (res == -ERROR_INVALID_FORMAT) {
        res = process_load_binary(filename, process);
    }
    return res;

}

int process_map_binary(struct process* process) {
    int res = 0;
    res = paging_map_to(process->task->page_directory,
                        (void*) PEACHOS_PROGRAM_VIRTUAL_ADDRESS,
                        process->ptr,
                        paging_align_address(process->ptr + process->size),
                        PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);
    return res;
}
static int process_map_elf(struct process* process) {
    int res = 0;

    struct elf_file* elf_file = process->elf_file;
    struct elf_header* header = elf_header(elf_file);
    struct elf32_phdr* phdrs = elf_pheader(header);
    for (int i = 0; i < header->e_phnum; i++)
    {
        struct elf32_phdr* phdr = &phdrs[i];
        void* phdr_phys_addr = elf_phdr_phys_address(elf_file, phdr);
        int flags = PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL;
        if (phdr->p_flags & PF_W) {
            flags |= PAGING_IS_WRITEABLE;
        }

        res = paging_map_to(process->task->page_directory,
                paging_align_to_lower_page((void*)phdr->p_vaddr),
                paging_align_to_lower_page(phdr_phys_addr),
                paging_align_address(phdr_phys_addr + phdr->p_memsz),
                flags);
        
        if (ISERROR(res)) {
            break;
        }
    }
    

    return res;
}
int process_map_memory(struct process* process) {
    int res = 0;
    switch (process->filetype) {
        case PROCESS_FILETYPE_ELF:
            res = process_map_elf(process);
            break;
        case PROCESS_FILETYPE_BINARY:
            res = process_map_binary(process);
            break;
        default:
            panic("Process map memory invalid file type\n");
    }
    
    if (res < 0) {
        goto out;
    }

    // Map the stack memory
    res = paging_map_to(process->task->page_directory,
                (void*) PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END, 
                process->stack,
                paging_align_address(process->stack + PEACHOS_USER_PROGRAM_STACK_SIZE),
                PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);
    
    if (res < 0) {
        goto out;
    }

out:
    return res;
}

int process_get_free_slot() {
    for (int i = 0; i < PEACHOS_MAX_PROCESSES; i++)
    {
        if (processes[i] == 0) {
            return i;
        }
    }

    return -ERROR_IS_TAKEN;
    
}

int process_load(const char* filename, struct process** process) {
    int res = 0;

    int process_slot = process_get_free_slot();
    if (process_slot < 0) {
        res = -ERROR_IS_TAKEN;
        goto out;
    }

    res = process_load_for_slot(filename, process, process_slot);

out:
    return res;
}

int process_load_switch(const char* filename, struct process** process) {
    int res = process_load(filename, process);
    if (res == 0) {
        process_switch(*process);
    }

    return res;
}

int process_load_for_slot(const char* filename, struct process** process, int process_slot) {
    int res = 0;
    struct task* task = 0;
    struct process* _process;
    void* program_stack_ptr = 0;

    if (process_get(process_slot) != NO_ERROR) {
        res = -ERROR_IS_TAKEN;
        goto out;
    }

    _process = kzalloc(sizeof(struct process));
    if (!_process) {
        res = -ERROR_NO_MEMORY;
        goto out;
    }

    process_init(_process);
    res = process_load_data(filename, _process);
    if (res < 0) {
        goto out;
    }

    program_stack_ptr = kzalloc(PEACHOS_USER_PROGRAM_STACK_SIZE);
    if (!program_stack_ptr) {
        res = -ERROR_NO_MEMORY;
        goto out;
    }

    strncpy(_process->filename, filename, sizeof(_process->filename));
    _process->stack = program_stack_ptr;
    _process->id = process_slot;

    // Create a task
    task = task_new(_process);
    if (ERROR_I(task) == 0) {
        res = ERROR_I(task);
        goto out;
    }

    _process->task = task;
    res = process_map_memory(_process);
    if (res < 0) {
        goto out;
    }

    *process = _process;

    // Add the process to the array
    processes[process_slot] = _process;


out:
    if (ISERROR(res)) {
        if (_process && _process->task) {
            task_free(_process->task);
        }

        // Free the process data TODO
    }
    return res;
}


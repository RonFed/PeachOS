#include "task.h"
#include "memory/memory.h"
#include "status.h"
#include "kernel.h"
#include "memory/heap/kheap.h"
#include "process.h"
#include "idt/idt.h"
#include "memory/paging/paging.h"
#include "string/string.h"
#include "loader/formats/elf_loader.h"

// Current task running
struct task* current_task = 0;
// Task lined list
struct task* task_tail = 0;
struct task* task_head = 0;

// Copy the string pointed by virt (virtual address) in the user task
// to the physical memory phys. max is the number of bytes to copy
int copy_string_from_task(struct task* task, void* virt, void* phys, int max) {
    if (max > PAGING_PAGE_SIZE) {
        return -ERROR_INVALID_ARG;
    }

    int res = 0;

    char* tmp = kzalloc(max);
    if (!tmp) {
        res = -ERROR_NO_MEMORY;
        goto out;
    }

    uint32_t* task_directory = task->page_directory->directory_entry;
    // Save the old mapping to tmp to restore later
    uint32_t old_entry = paging_get(task_directory, tmp);
    // Map the tmp memory in the user page table so the user (task) can "see" it
    paging_map(task->page_directory, tmp, tmp, PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    
    // Switch to user paging (so we can understand the virtual address) and copy the string to tmp
    paging_switch(task->page_directory);
    strncpy(tmp, virt, max);
    
    // Switch back to kernel paging
    kernel_page();

    // Restore the old entry (avoid the user having pointer to kernel memory)
    res = paging_set(task_directory, tmp, old_entry);
    if (res < 0) {
        res = -ERROR_IO;
        goto out_free;
    }

    strncpy(phys, tmp, max);

out_free:
    kfree(tmp);
out:
    return res;
}

int task_init(struct task* task, struct process* process);

struct task* task_current() {
    return current_task;
}

struct task* task_new(struct process* process) {
    int res = 0;
    struct task* task = kzalloc(sizeof(struct task));
    if (!task) {
        res = -ERROR_NO_MEMORY;
        goto out;
    }

    res = task_init(task, process);
    if (res != NO_ERROR) {
        goto out;
    }

    if (task_head == 0) {
        task_head = task;
        task_tail = task;
        current_task = task;
        goto out;
    }

    task_tail->next = task;
    task->prev = task_tail;
    task_tail = task;

out:
    if (ISERROR(res)) {
        task_free(task);
        return ERROR(res);
    }
    return task;
}
struct task* task_get_next() {
    if (!current_task->next) {
        return current_task;
    }

    return current_task->next;
}

static void task_list_remove(struct task* task) {
    if (task->prev) {
        task->prev->next = task->next;
    }
    if (task == task_head) {
        task_head = task->next;
    }
    if (task == task_tail) {
        task_tail = task->prev;
    }
    if (task == current_task) {
        current_task = task_get_next();
    }
    // if (task->next) {
    //     task->next->prev = task->prev;
    // }
}

int task_free(struct task* task) {
    paging_free_4gb(task->page_directory);
    task_list_remove(task);

    kfree(task);
    return 0;
}

int task_switch(struct task* task) {
    current_task = task;
    paging_switch(task->page_directory);
    return 0;
}

void task_save_state(struct task* task, struct interrupt_frame* int_frame) {
    task->registers.ip =    int_frame->ip;
    task->registers.cs =    int_frame->cs;
    task->registers.flags = int_frame->flags;
    task->registers.esp =   int_frame->esp;
    task->registers.ss =    int_frame->ss;
    task->registers.eax =   int_frame->eax;
    task->registers.ebp =   int_frame->ebp;
    task->registers.ebx =   int_frame->ebx;
    task->registers.ecx =   int_frame->ecx;
    task->registers.edi =   int_frame->edi;
    task->registers.edx =   int_frame->edx;
    task->registers.esi =   int_frame->esi;
}

void task_current_save_state(struct interrupt_frame* int_frame) {
    if (!task_current()) {
        panic("NO current task to save\n");
    }

    struct task* task = task_current();
    task_save_state(task, int_frame);
}

int task_page() {
    // Set all segments registers back to user land
    user_registers();
    task_switch(current_task);
    return 0;
}

int task_page_task(struct task* task) {
    user_registers();
    paging_switch(task->page_directory);
    return 0;
}

void task_run_first_ever_task() {
    if (!current_task) {
        panic("task_run_first_ever_task: No current task exists \n");
    }

    task_switch(task_head);
    task_return(&task_head->registers);
}

int task_init(struct task* task, struct process* process) {
    memset(task, 0, sizeof(struct task));
    // Map the entire 4GB space to itself
    task->page_directory = paging_new_4gb(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (!task->page_directory)
    {
        return -ERROR_IO;
    }

    task->registers.ip = PEACHOS_PROGRAM_VIRTUAL_ADDRESS;
    if (process->filetype == PROCESS_FILETYPE_ELF) {
        task->registers.ip = elf_header(process->elf_file)->e_entry;
    }
    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.cs = USER_CODE_SEGMENT;
    task->registers.esp = PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;

    task->process = process;
    return 0;
}

void* task_get_stack_item(struct task* task, int index) {
    void* result = 0 ;

    uint32_t* sp_ptr = (uint32_t*) task->registers.esp;

    // Switch to the given task page
    task_page_task(task);

    result = (void*)sp_ptr[index];

    // Switch back to the kernel page
    kernel_page();

    return result;
}

void* task_virt_to_phys(struct task* task, void* virt_addr) {
    return paging_get_physical_address(task->page_directory->directory_entry, virt_addr);
}
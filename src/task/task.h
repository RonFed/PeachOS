#ifndef TASK_H
#define TASK_H
#include "config.h"
#include "memory/paging/paging.h"

struct interrupt_frame;

struct registers {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
};

struct process;

struct task {
    // Page directory of the task
    struct paging_4gb_chunk* page_directory;
    // Registers of the task when it is not running
    struct registers registers;
    // The process of the task
    struct process* process;
    // The next task in the linked list
    struct task* next;
    // Previous task in the linked list
    struct task* prev;
};

struct task* task_new(struct process* process);
struct task* task_current();
struct task* task_get_next();
int task_free(struct task* task);

// Move to user land
void task_return(struct registers* regs);
void restore_general_purpose_registers(struct registers* regs);
void user_registers();
int task_switch(struct task* task);
int task_page();
int task_page_task(struct task* task);
void task_next();

void task_run_first_ever_task();
void task_current_save_state(struct interrupt_frame* int_frame);
int copy_string_from_task(struct task* task, void* virt, void* phys, int max);

void* task_get_stack_item(struct task* task, int index);
void* task_virt_to_phys(struct task* task, void* virt_addr);
#endif
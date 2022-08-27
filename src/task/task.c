#include "task.h"
#include "memory/memory.h"
#include "status.h"
#include "kernel.h"
#include "memory/heap/kheap.h"

// Current task running
struct task* current_task = 0;
// Task lined list
struct task* task_tail = 0;
struct task* task_head = 0;

int task_init(struct task* task);

struct task* task_current() {
    return current_task;
}

struct task* task_new() {
    int res = 0;
    struct task* task = kzalloc(sizeof(struct task));
    if (!task) {
        res = -ERROR_NO_MEMORY;
        goto out;
    }

    res = task_init(task);
    if (res != NO_ERROR) {
        goto out;
    }

    if (task_head == 0) {
        task_head = task;
        task_tail = task;
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

int task_init(struct task* task) {
    memset(task, 0, sizeof(struct task));
    // Map the entire 4GB space to itself
    task->page_directory = paging_new_4gb(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (!task->page_directory)
    {
        return -ERROR_IO;
    }

    task->registers.ip = PEACHOS_PROGRAM_VIRTUAL_ADDRESS;
    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.esp = PEACHOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START;

    return 0;
}
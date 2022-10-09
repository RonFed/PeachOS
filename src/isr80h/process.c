#include "process.h"
#include "task/task.h"
#include "config.h"
#include "status.h"
#include "task/process.h"
#include "string/string.h"

void* isr80h_command6_process_load_start(struct interrupt_frame* int_frame) {
    void* filename_user_ptr = task_get_stack_item(task_current(), 0);
    char filename[PEACHOS_MAX_PATH];
    int res = copy_string_from_task(task_current(), filename_user_ptr, filename, sizeof(filename));

    if (res < 0) {
        goto out;
    }
    // This is bad - better implement enviroment variavles
    char path[PEACHOS_MAX_PATH];
    strcpy(path, "0:/");
    strcpy(path + 3, filename);
    struct process* process = 0;
    
    // Load neew process image and switch to it
    res = process_load_switch(path, &process);
    if (res < 0) {
        goto out;
    }

    // Switch to the new process' task (and change paging prespective accordingly)
    task_switch(process->task);
    // Drop to user space (not going throgh the 'out' label)
    task_return(&process->task->registers);
out:
    return 0;
}

#ifndef ISR80_IO_H
#define ISR80_IO_H

struct interrupt_frame;

void* isr80h_command1_print(struct interrupt_frame* frame);

#endif
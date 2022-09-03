#ifndef IDT_H
#define IDT_H
#include <stdint.h>

struct interrupt_frame;
typedef void*(*ISR80_COMMAND)(struct interrupt_frame* frame);
typedef void(*INTERRUPT_CALLBACK_FUNCTION)();

// Interrupt descriptor table descriptor
struct idt_desc
{
    uint16_t offset_1;  // offset bits 0 -15
    uint16_t selector; // Selector in our GDT
    uint8_t zero;       // Unused
    uint8_t type_attr;  // Descriptor type
    uint16_t offset_2;  // offset bits 16 -31
} __attribute__((packed));

struct idtr_desc
{
    uint16_t limit; // SIze of descriptor table
    uint32_t base;  //  Base address of the interrupt descrtiptor table
} __attribute__((packed));

struct interrupt_frame
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t reserved; // sp from pushad (not intersting)
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;   // User's stack pointer
    uint32_t ss;
}__attribute__((packed));

void idt_init();
void enable_interrupts();
void disable_interrupts();
void isr80h_register_command(int command, ISR80_COMMAND command_handler);
int idt_register_interrupt_callback(int interrupt, INTERRUPT_CALLBACK_FUNCTION interrubt_cb);

#endif

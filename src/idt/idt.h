#ifndef IDT_H
#define IDT_H
#include <stdint.h>

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

void idt_init();

#endif

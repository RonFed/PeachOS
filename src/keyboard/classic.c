#include "classic.h"
#include "keyboard.h"
#include "kernel.h"
#include "idt/idt.h"
#include "task/task.h"
#include <stdint.h>
#include <stddef.h>
#include "io/io.h"

#define CLASSIC_KEYBOARD_CAPSLOCK   0x3A
int classic_keyboard_init();
void classic_keyboard_handle_interrupt();

static uint8_t keyboard_scan_set_one[] = {
    0x00, 0x1B, '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    0x08, '\t', 'Q', 'W', 'E', 'R', 'T',
    'Y', 'U', 'I', 'O', 'P', '[', ']',
    0x0d, 0x00, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ';', '\'', '`',
    0x00, '\\', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', ',', '.', '/', 0x00, '*',
    0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, '7', '8', '9', '-', '4', '5',
    '6', '+', '1', '2', '3', '0', '.' 
};

struct keyboard classic_keyboard = {
    .name = "Classic",
    .init = classic_keyboard_init};

int classic_keyboard_init()
{
    idt_register_interrupt_callback(ISR_KEYBOARD_INTERRUPT, classic_keyboard_handle_interrupt);
    keyboard_set_capslock(&classic_keyboard, KEYBOARD_CAPSLOCK_OFF);

    outb(PS2_PORT, PS2_COMMAND_ENABLE_FIRST_PORT); // Enable the first PS2 port
    return 0;
}

uint8_t classic_keyboard_scancode_to_char(uint8_t scancode) {
    size_t size_of_keyboard_set_one = sizeof(keyboard_scan_set_one) / sizeof(uint8_t);
    if (scancode > size_of_keyboard_set_one) {
        return 0;
    }

    char c = keyboard_scan_set_one[scancode];

    if (keyboard_get_capslock(&classic_keyboard) == KEYBOARD_CAPSLOCK_OFF) {
        if (c >= 'A' && c <= 'Z') {
            c += 32;
        }
    }

    return c;
}

void classic_keyboard_handle_interrupt() {
    kernel_page();
    uint8_t scan_code = 0;
    scan_code = insb(KEYBOARD_INPUT_PORT);
    // ignore roge bytes
    insb(KEYBOARD_INPUT_PORT);

    if (scan_code & CLASSIC_KEYBOARD_KEY_RELEASED) {
        // Only care for key presses not release
        return;
    }

    if (scan_code == CLASSIC_KEYBOARD_CAPSLOCK) {
        KEYBOARD_CAPSLOCK_STATE old_state = keyboard_get_capslock(&classic_keyboard);
        keyboard_set_capslock(&classic_keyboard, old_state == KEYBOARD_CAPSLOCK_ON ? KEYBOARD_CAPSLOCK_OFF : KEYBOARD_CAPSLOCK_ON);
    }
    

    uint8_t c = classic_keyboard_scancode_to_char(scan_code);
    if (c != 0) {
        // valid scancode
        keyboard_push(c);
    }

    task_page();
}

struct keyboard* classic_init()
{
    return &classic_keyboard;
}
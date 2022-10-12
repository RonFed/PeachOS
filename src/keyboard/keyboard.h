#ifndef KRYBOARD_H
#define KRYBOARD_H

#define KEYBOARD_CAPSLOCK_ON    1
#define KEYBOARD_CAPSLOCK_OFF   0
typedef int KEYBOARD_CAPSLOCK_STATE;

typedef int(*KEYBOARD_INIT_FUNCTION)();

struct process;

struct keyboard {
    KEYBOARD_INIT_FUNCTION  init;
    char name[20];
    KEYBOARD_CAPSLOCK_STATE capslock_state;
    struct keyboard* next;
};

void keyboard_init();
void keyboard_backspace(struct process* process);
void keyboard_push(char c);
char keyboard_pop();
int keyboard_insert(struct keyboard* keyboard);

void keyboard_set_capslock(struct keyboard* keyboard, KEYBOARD_CAPSLOCK_STATE state);
KEYBOARD_CAPSLOCK_STATE keyboard_get_capslock(struct keyboard* keyboard);
#endif
#ifndef ISR80H_H
#define ISR80H_H

enum system_commands {
    SYSTEM_COMMAND0_SUM,
    SYSTEM_COMMAND1_PRINT,
    SYSTEM_COMMAND2_GETKEY,
    SYSTEM_COMMAND3_PUTCHAR,
    SYSTEM_COMMAND4_MALLOC,
    SYSTEM_COMMAND5_FREE,
};

void isr80h_register_commands();


#endif
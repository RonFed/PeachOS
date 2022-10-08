#include "peachos.h"
#include <stdbool.h>

int peachos_getkey_block() {
    int val = 0;
    do
    {
        val = peachos_getkey();
    } while (val == 0);
    return val;
}

void peachos_terminal_readline(char* out, int max, bool output_while_typing) {
    int i = 0;
    for (i = 0; i < max - 1; i++)
    {
        char key = peachos_getkey_block();
        if (key == '\r') {
            // Carrige return pressed
            break;
        }

        if (output_while_typing) {
            peachos_putchar(key);
        }

        // Backspace
        if (key == 0x08 && i >= 1) {
            out[i-1] = 0x00;
            // -2 becouse we have ++ upon continue
            i -= 2;
            continue;
        }

        out[i] = key;
    }

    out[i] = 0x00;
    
}
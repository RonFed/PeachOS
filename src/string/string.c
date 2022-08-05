#include "string.h"

int strlen(const char* ptr) {
    int i = 0;
    while(*ptr != 0) {
        i++;
        ptr++;
    }
    return i;
}

int strnlen(const char* ptr, int max_len) {
    int i = 0;
    for (i = 0; i < max_len; i++)
    {
        if (ptr[i] == 0) {
            break;
        }
    }

    return i;
    
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

int to_numeric_digit(char c) {
    return c - '0';
}
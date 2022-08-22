#include "string.h"

char toLower(char s1) {
    if (s1 >= 65 && s1 <= 90) {
        s1 += 32;
    }
    return s1;
}

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

char* strcpy(char* dest, const char* src) {
    char* res = dest;
    while (*src != 0) {
        *dest = *src;
        src  += 1;
        dest += 1;
    }
    *dest = 0x00;
    return res;
}

int istrncmp(const char* str1, const char* str2, int n) { 
    unsigned char u1, u2;
    while (n-- > 0) {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2 && toLower(u1) != toLower(u2)) {
            return u1 - u2;
        }
        if (u1 == '\0') {
            return 0;
        }
    }

    return 0;
}

int strncmp(const char* str1, const char* str2, int n) {
    unsigned char u1, u2;
    while (n-- > 0) {
        u1 = (unsigned char)*str1++;
        u2 = (unsigned char)*str2++;
        if (u1 != u2) {
            return u1 - u2;
        }
        if (u1 == '\0') {
            return 0;
        }
    }

    return 0;
}

int strnlen_terminator(const char* str, int max, char terminator) {
    int i = 0;
    for (i = 0; i < max; i++)
    {
        if (str[i] == '\0' || str[i] == terminator) {
            break;
        }
    }
    return i;
}
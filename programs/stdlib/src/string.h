#ifndef PEACHOS_STRING_H
#define PEACHOS_STRING_H

#include <stdbool.h>

int strlen(const char* ptr);
bool is_digit(char c);
int to_numeric_digit(char c);
int strnlen(const char* ptr, int max_len);
char* strcpy(char* dest, const char* src);
char toLower(char s1);
int strncmp(const char* str1, const char* str2, int n);
int istrncmp(const char* str1, const char* str2, int n);
int strnlen_terminator(const char* str, int max, char terminator);
char* strncpy(char* dest, const char* src, int count);
char* strtok(char* str, const char* delimeters);
#endif
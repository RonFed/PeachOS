#include "peachos.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char const *argv[])
{
    printf("My age is %i\n", 24);
    char words[] = "hello how are you";
    const char* token = strtok(words, " ");
    while (token) {
        printf("%s\n", token);
        token = strtok(NULL, " ");
    }
    while(1) {}
    return 0;
}

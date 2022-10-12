#include "peachos.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc, char const *argv[])
{
    for (int i = 0; i < argc; i++)
    {
        printf("%s\n", argv[i]);
    }

    char* ptr = (char*) 0x00;
    *ptr = 0x50;
    printf("bi\n");
    while(1) {}
    return 0;
}

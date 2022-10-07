#include "peachos.h"
#include "stdlib.h"

int main(int argc, char const *argv[])
{
    print("Hello from blank !\n");

    void* ptr = malloc(512);
    free(ptr);

    while(1) {
        if (getkey() != 0) {
            print("key pressed\n");
        }
    }
    return 0;
}

#include "shell.h"
#include "stdio.h"
#include "stdlib.h"
#include "peachos.h"


int main(int argc, char const *argv[])
{
    print("Peachos v1.0\n");

    while(1) {
        print("> ");
        char buf[1024];
        peachos_terminal_readline(buf, sizeof(buf), true);
        peachos_process_load_start(buf);
        print("\n");
    }
    return 0;
}

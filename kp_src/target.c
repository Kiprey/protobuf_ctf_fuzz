#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
    if (argc < 3) {
        printf("[usage] %s <target_binary_path> <input_file(@@)>\n", argv[0]);
        exit(-1);
    }
    char* target_binary_path = argv[1];
    char* input_file = argv[2];

    freopen(input_file, "r", stdin);

    char *newargv[] = { target_binary_path, NULL };
    char *newenviron[] = { NULL };
    execve(target_binary_path, newargv, newenviron);

    printf("[ERROR] execve fail! Maybe path [%s] wrong? \n", target_binary_path);
    return -1;
}
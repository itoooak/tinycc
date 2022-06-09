#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が間違っています\n");
        return 1;
    }

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    char *p = argv[1];

    printf("    mov rax, %ld\n", strtol(p, &p, 10));

    while(*p) {
        if (*p == '+') {
            p++;
            printf("    add rax, %ld\n", strtol(p, &p, 10));
        } else if (*p == '-') {
            p++;
            printf("    sub rax, %ld\n", strtol(p, &p, 10));
        } else {
            fprintf(stderr, "不正な入力です: '%c'\n", *p);
            return 1;
        }
    }

    printf("    ret\n");
    return 0;
}

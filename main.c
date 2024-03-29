#include "tinycc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が間違っています\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize(user_input);
    program();

    for (int i = 0; code[i]; i++)
        add_typeinfo(code[i]);

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");

    for (int i = 0; code[i]; i++)
        gen(code[i]);

    return 0;
}

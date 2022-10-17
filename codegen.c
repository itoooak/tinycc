#include "tinycc.h"

const char *REG_NAME[ARG_NUM_MAX] = 
    { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");

    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

void gen(Node *node) {
    switch (node->kind) {
        case ND_NUM:
            printf("    push %d\n", node->val);
            return;
        case ND_ASSIGN:
            gen_lval(node->lhs);
            gen(node->rhs);

            printf("    pop rdi\n");
            printf("    pop rax\n");
            printf("    mov [rax], rdi\n");
            printf("    push rdi\n");
            return;
        case ND_LVAR:
            gen_lval(node);
            printf("    pop rax\n");
            printf("    mov rax, [rax]\n");
            printf("    push rax\n");
            return;
        case ND_RETURN:
            gen(node->lhs);
            printf("    pop rax\n");
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
            return;
        case ND_IF:
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            if (node->els)
                printf("    je .Lelse%p\n", node);
            else
                printf("    je .Lend%p\n", node);
            gen(node->then);
            printf("    jmp .Lend%p\n", node);
            if (node->els) {
                printf(".Lelse%p:\n", node);
                gen(node->els);
            }
            printf(".Lend%p:\n", node);
            return;
        case ND_WHILE:
            printf(".Lstart%p:\n", node);
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .Lend%p\n", node);
            gen(node->then);
            printf("    jmp .Lstart%p\n", node);
            printf(".Lend%p:\n", node);
            return;
        case ND_FOR:
            if (node->init)
                gen(node->init);
            printf(".Lstart%p:\n", node);
            if (node->cond) {
                gen(node->cond);
                printf("    pop rax\n");
                printf("    cmp rax, 0\n");
                printf("    je .Lend%p\n", node);
            }
            gen(node->then);
            if (node->step)
                gen(node->step);
            printf("    jmp .Lstart%p\n", node);
            printf(".Lend%p:\n", node);
            return;
        case ND_BLOCK:
            while (node->next) {
                node = node->next;
                gen(node);
            }
            return;
        case ND_FUNCCALL:
            for (int i = node->argsnum - 1; i >= 0; i--)
                gen(node->funcargs[i]);
            for (int i = 0; i < node->argsnum; i++)
                printf("    pop %s\n", REG_NAME[i]);
            printf("    call %s\n", node->funcname);
            printf("    push rax\n");
            return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("    add rax, rdi\n");
            break;
        case ND_SUB:
            printf("    sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("    imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("    cqo\n");
            printf("    idiv rdi\n");
            break;
        case ND_EQ:
            printf("    cmp rax, rdi\n");
            printf("    sete al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_NE:
            printf("    cmp rax, rdi\n");
            printf("    setne al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LT:
            printf("    cmp rax, rdi\n");
            printf("    setl al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LE:
            printf("    cmp rax, rdi\n");
            printf("    setle al\n");
            printf("    movzb rax, al\n");
            break;
    }

    printf("    push rax\n");
}

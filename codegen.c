#include "tinycc.h"

const char *arg_reg_8byte[ARG_NUM_MAX] = 
    { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

const char *arg_reg_4byte[ARG_NUM_MAX] = 
    { "edi", "esi", "edx", "ecx", "r8d", "r9d" };

int size_of(Type *type) {
    switch (type->kind) {
        case TY_INT:
            return 8;
        case TY_PTR:
            return 8;
    }
}

int align_to(int cur_offset, int boundary) {
    return (cur_offset + boundary - 1) / boundary * boundary;
}

void gen_addr(Node *node) {
    switch (node->kind) {
        case ND_LVAR:
            printf("    mov rax, rbp\n");
            printf("    sub rax, %d\n", node->lvar->offset);
            printf("    push rax\n");
            return;
        case ND_DEREF:
            gen(node->lhs);
            return;
    }
}

void gen_funcdef(Node *node) {
    printf("%s:\n", node->funcname);

    // prologue
    printf("    push rbp\n");
    // R12は関数呼び出し時のアラインメントに使う
    printf("    push r12\n");
    // RSPを16の倍数にするためにもう一回push
    printf("    push r13\n");
    printf("    mov rbp, rsp\n");

    int prev_offset = 0;
    int arg_idx = node->argsnum;
    for (LVar *lvar = node->locals; lvar->next != NULL; prev_offset = lvar->offset, lvar = lvar->next) {
        lvar->offset = align_to(prev_offset + size_of(lvar->type), size_of(lvar->type));
        
        if (lvar->is_arg) {
            printf("    mov [rbp-%d], %s\n", lvar->offset, arg_reg_8byte[arg_idx-1]);
            arg_idx--;
        }
    }
    
    int offset_sum = align_to(prev_offset, 16);
    printf("    sub rsp, %d\n", offset_sum);

    gen(node->funcbody);
    
    // epilogue
    printf("    mov rsp, rbp\n");
    printf("    pop r13\n");
    printf("    pop r12\n");
    printf("    pop rbp\n");
    printf("    ret\n");
}

void gen_funccall(Node *node) {
    for (int i = node->argsnum - 1; i >= 0; i--)
        gen(node->funcargs[i]);
    for (int i = 0; i < node->argsnum; i++)
        printf("    pop %s\n", arg_reg_8byte[i]);

    // align RSP to 16-byte boundary
    printf("    mov r12, rsp\n");
    printf("    and r12, 0b1111\n");
    printf("    sub rsp, r12\n");
    printf("    mov rax, 0\n");
    printf("    call %s\n", node->funcname);
    printf("    add rsp, r12\n");
    printf("    push rax\n");
}

void gen(Node *node) {
    switch (node->kind) {
        case ND_NUM:
            printf("    push %d\n", node->val);
            return;
        case ND_ASSIGN:
            gen_addr(node->lhs);
            gen(node->rhs);

            printf("    pop rdi\n");
            printf("    pop rax\n");
            printf("    mov [rax], rdi\n");
            printf("    push rdi\n");
            return;
        case ND_LVAR:
            gen_addr(node);
            printf("    pop rax\n");
            printf("    mov rax, [rax]\n");
            printf("    push rax\n");
            return;
        case ND_RETURN:
            gen(node->lhs);
            printf("    pop rax\n");
            printf("    mov rsp, rbp\n");
            printf("    pop r13\n");
            printf("    pop r12\n");
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
            gen_funccall(node);
            return;
        case ND_FUNCDEF:
            gen_funcdef(node);
            return;
        case ND_ADDR:
            gen_addr(node->lhs);
            return;
        case ND_DEREF:
            gen(node->lhs);
            printf("    pop rax\n");
            printf("    mov rax, [rax]\n");
            printf("    push rax\n");
            return;
        case ND_DECL:
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

#include "tinycc.h"

FuncInfo *funcinfo_list;

Type *functype(char *funcname) {
    for (FuncInfo *info = funcinfo_list; info; info = info->next)
        if (strlen(funcname) == strlen(info->name) &&
            !memcmp(funcname, info->name, strlen(funcname))) {
            return info->type;
        }
    return NULL;
}

// 2つの型を比較して、同じなら1、異なるなら0を返す
int is_same(Type *type1, Type *type2) {
    if (!type1 || !type2) return 0;

    if (type1->kind != type2->kind)
        return 0;
    else if (type1->kind == TY_PTR)
        return is_same(type1->ptr_to, type2->ptr_to);
    else
        return 1;
}

int is_ptr(Type *type) {
    if (!type) return 0;
    else
        return type->kind == TY_PTR;
}

Type *type_int() {
    Type *type = calloc(1, sizeof(Type));
    type->kind = TY_INT;
    return type;
}

Type *type_ptr(Type *ptr_to) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = TY_PTR;
    type->ptr_to = ptr_to;
    return type;
}

int size_of(Type *type) {
    if (!type)
        error("型の情報がありません");
    switch (type->kind) {
        case TY_INT:
            return 4;
        case TY_PTR:
            return 8;
    }
}

void add_typeinfo(Node *node) {
    if (!node) return;

    add_typeinfo(node->lhs);
    add_typeinfo(node->rhs);
    add_typeinfo(node->cond);
    add_typeinfo(node->then);
    add_typeinfo(node->els);
    add_typeinfo(node->init);
    add_typeinfo(node->step);

    add_typeinfo(node->funcbody);

    switch (node->kind) {
        case ND_ADD:
            if (is_ptr(node->lhs->type) && is_ptr(node->rhs->type))
                error_at(node->str, "加算は両辺にpointerをとることができません");
            else if (is_ptr(node->lhs->type))
                node->type = node->lhs->type;
            else
                // int + int もしくは int + pointer
                node->type = node->rhs->type;
            break;
        case ND_SUB:
            if (is_same(node->lhs->type, type_int()) && is_ptr(node->rhs->type))
                error_at(node->str, "intからpointerを引く減算はできません");
            // int - int, pointer - int, pointer - pointer の3パターンのみ
            node->type = node->lhs->type;
            break;
        case ND_MUL:
        case ND_DIV:
            node->type = type_int();
            break;
        case ND_NUM:
            node->type = type_int();
            break;
        case ND_EQ:
        case ND_NE:
        case ND_LT:
        case ND_LE:
            if (node->lhs->type && node->rhs->type &&
                !is_same(node->lhs->type, node->rhs->type))
                error_at(node->str, "一致しない型の比較はできません");
            node->type = type_int();
            break;
        case ND_ASSIGN:
            node->type = node->lhs->type = node->rhs->type;
            break;
        case ND_LVAR:
            node->type = node->lvar->type;
            break;
        case ND_FUNCCALL:
            node->type = functype(node->funcname);
            for (int i = 0; node->funcargs[i] && i < ARG_NUM_MAX; i++)
                add_typeinfo(node->funcargs[i]);
            break;
        case ND_FUNCDEF:
            // parseの段階で型付けされている
            break;
        case ND_ADDR:
            node->type = type_ptr(node->lhs->type);
            break;
        case ND_DEREF:
            if (!is_ptr(node->lhs->type))
                error_at(node->str, "単項'*'の引数にpointer以外をとることはできません");
            node->type = node->lhs->type->ptr_to;
            break;
        case ND_SIZEOF:
            node->type = type_int();
            if (node->lhs)
                node->val = size_of(node->lhs->type);
            break;

        case ND_RETURN:
        case ND_IF:
        case ND_WHILE:
        case ND_FOR:
        case ND_BLOCK:
        case ND_DECL:
            node->type = NULL;
            break;

    }

    add_typeinfo(node->next);
}

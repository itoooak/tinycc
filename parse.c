#include "tinycc.h"

Node *code[100];

// ローカル変数がそれ以前に存在していない場合でも
// localsのoffsetにアクセスできるように初期化
// (関数primary内でアクセスが起こる)
LVar locals_init = { NULL, "", 0, 0 };

Node *parsing_func;

Node *new_node(NodeKind kind, Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->str = tok->str;
    return node;
}

Node *new_node_unary(NodeKind kind, Node *child, Token *tok) {
    Node *node = new_node(kind, tok);
    node->lhs = child;
    return node;
}

Node *new_node_bin(NodeKind kind, Node *lhs, Node *rhs, Token *tok) {
    Node *node = new_node(kind, tok);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val, Token *tok) {
    Node *node = new_node(ND_NUM, tok);
    node->val = val;
    return node;
}

LVar *new_lvar(Type *ty, Token *tok) {
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->next = parsing_func->locals;
    lvar->name = token->str;
    lvar->len = token->len;
    lvar->type = ty;
    return lvar;
}

LVar *find_lvar(Token *tok) {
    for (LVar *var = parsing_func->locals; var; var = var->next)
        if (var->len == tok->len &&
            !memcmp(tok->str, var->name, var->len)) {
            return var;
        }
    return NULL;
}

char *copy(char *from, int len) {
    char *copy = calloc(len + 1, sizeof(char));
    strncpy(copy, from, len);
    return copy;
}

void program() {
    int idx = 0;
    while (!at_eof())
        code[idx++] = func_def();

    code[idx] = NULL;
    return;
}

Node *func_def() {
    Node *node = new_node(ND_FUNCDEF, token);

    parsing_func = node;

    node->type = type();

    if (token->kind != TK_IDENT)
        error_at(token->str, "expected an identifier\n");

    node->funcname = copy(token->str, token->len);
    token = token->next;

    FuncInfo *funcinfo = calloc(1, sizeof(FuncInfo));
    funcinfo->next = funcinfo_list;
    funcinfo->name = node->funcname;
    funcinfo->type = node->type;
    funcinfo_list = funcinfo;

    expect("(");

    node->argsnum = 0;

    node->locals = &locals_init;

    if (!consume(")")) {
        for (int i = 0; i < ARG_NUM_MAX; i++) {
            node->locals = new_lvar(type(), token);
            node->locals->is_arg = true;

            node->argsnum++;

            token = token->next;

            if (consume(")"))
                break;

            expect(",");
        }
    }

    node->funcbody = compound_stmt();

    return node;
}

Node *stmt() {
    Node *node;

    if (token->kind == TK_RETURN) {
        Token *return_token = token;
        token = token->next;
        node = new_node_unary(ND_RETURN, expr(), return_token);
        expect(";");
    } else if (token->kind == TK_IF) {
        node = new_node(ND_IF, token);
        token = token->next;

        expect("(");
        node->cond = expr();
        expect(")");

        node->then = stmt();
        if (token->kind == TK_ELSE) {
            token = token->next;
            node->els = stmt();
        }
    } else if (token->kind == TK_WHILE) {
        node = new_node(ND_WHILE, token);
        token = token->next;

        expect("(");
        node->cond = expr();
        expect(")");

        node->then = stmt();
    } else if (token->kind == TK_FOR) {
        node = new_node(ND_FOR, token);
        token = token->next;

        expect("(");
        if (!consume(";")) {
            node->init = expr();
            expect(";");
        }
        if (!consume(";")) {
            node->cond = expr();
            expect(";");
        }
        if (!consume(")")) {
            node->step = expr();
            expect(")");
        }

        node->then = stmt();
    } else if (token->kind == TK_RESERVED &&
               token->len == 1 &&
               *(token->str) == '{') {
        node = compound_stmt();
    } else {
        node = expr();
        expect(";");
    }

    return node;
}

Node *compound_stmt() {
    expect("{");

    Node *node = new_node(ND_BLOCK, token);
    Node *cur = node;
    while (!consume("}")) {
        if (token->kind == TK_INT)
            cur->next = declaration();
        else
            cur->next = stmt();
        cur = cur->next;
    }

    return node;
}

Node *declaration() {
    Node *node = new_node(ND_DECL, token);
    Type *ty = type();

    if (token->kind != TK_IDENT)
        error_at(token->str, "expected an identifier");

    LVar *lvar = find_lvar(token);
    if (lvar) {
        char *name = copy(token->str, token->len);
        error_at(token->str, "redeclaration of '%s'", name);
    } else {
        node->lvar = new_lvar(node->type, token);
        node->lvar->is_arg = false;
        node->lvar->type = ty;
        parsing_func->locals = node->lvar;
    }
    token = token->next;

    expect(";");

    return node;
}

Type *type() {
    if (token->kind != TK_INT)
        error_at(token->str, "expected a typename");
    token = token->next;

    Type *ty = type_int();

    while (consume("*")) {
        Type *tmp = type_ptr(ty);
        ty = tmp;
    }

    return ty;
}

Node *expr() {
    return assign();
}

Node *assign() {
    Node *node = equality();

    if (consume("=")) {
        Token *eq_token = token;
        node = new_node_bin(ND_ASSIGN, node, assign(), eq_token);
    }

    return node;
}

Node *equality() {
    Node *node = relational();

    Token *op_token = token;
    while (true) {
        if (consume("=="))
            node = new_node_bin(ND_EQ, node, relational(), op_token);
        else if (consume("!="))
            node = new_node_bin(ND_NE, node, relational(), op_token);
        else
            return node;
    }
}

Node *relational() {
    Node *node = add();

    Token *op_token = token;
    while (true) {
        if (consume("<="))
            node = new_node_bin(ND_LE, node, add(), op_token);
        else if (consume(">="))
            node = new_node_bin(ND_LE, add(), node, op_token);
        else if (consume("<"))
            node = new_node_bin(ND_LT, node, add(), op_token);
        else if (consume(">"))
            node = new_node_bin(ND_LT, add(), node, op_token);
        else
            return node;
    }
}

Node *add() {
    Node *node = mul();

    Token *op_token = token;
    while (true) {
        if (consume("+"))
            node = new_node_bin(ND_ADD, node, mul(), op_token);
        else if (consume("-"))
            node = new_node_bin(ND_SUB, node, mul(), op_token);
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();

    Token *op_token = token;
    while (true) {
        if (consume("*"))
            node = new_node_bin(ND_MUL, node, unary(), op_token);
        else if (consume("/"))
            node = new_node_bin(ND_DIV, node, unary(), op_token);
        else
            return node;
    }
}

Node *unary() {
    Node *node;

    Token *op_token = token;
    if (consume("+"))
        node = primary();
    else if (consume("-"))
        node = new_node_bin(ND_SUB, new_node_num(0, token), primary(), op_token);
    else if (consume("&"))
        node = new_node_unary(ND_ADDR, unary(), op_token);
    else if (consume("*"))
        node = new_node_unary(ND_DEREF, unary(), op_token);
    else if (token->kind == TK_SIZEOF) {
        token = token->next;

        if (consume("(")) {
            if (token->kind == TK_INT) {
                // 型を引数に取る場合
                node = new_node_unary(ND_SIZEOF, NULL, op_token);
                Type *ty = type();
                node->val = size_of(ty);
            } else {
                node = new_node_unary(ND_SIZEOF, unary(), op_token);
            }
            expect(")");
        } else {
            node = new_node_unary(ND_SIZEOF, unary(), op_token);
        }
    }
    else
        node = primary();

    return node;
}

Node *primary() {
    Node *node;

    if (consume("(")) {
        node = expr();
        expect(")");
    } else if (token->kind == TK_IDENT) {
        node = calloc(1, sizeof(Node));
        Token *ident_token = token;
        token = token->next;

        // function call
        if (consume("(")) {
            node->kind = ND_FUNCCALL;
            node->argsnum = 0;

            if (!consume(")")) {
                for (int i = 0; i < ARG_NUM_MAX; i++) {
                    node->funcargs[i] = expr();
                    node->argsnum++;

                    if (consume(")"))
                        break;

                    expect(",");
                }
            }

            node->funcname = copy(ident_token->str, ident_token->len);
        }

        // variable
        else {
            node->kind = ND_LVAR;

            LVar *lvar = find_lvar(ident_token);
            if (lvar) {
                node->lvar = lvar;
            } else {
                char *name = copy(ident_token->str, ident_token->len);
                error_at(ident_token->str, "variable '%s' does not exist", name);
            }
        }
    } else {
        node = new_node_num(expect_number(), token);
    }

    return node;
}

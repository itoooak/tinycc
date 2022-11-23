#include "tinycc.h"

Node *code[100];

// ローカル変数がそれ以前に存在していない場合でも
// localsのoffsetにアクセスできるように初期化
// (関数primary内でアクセスが起こる)
LVar locals_init = { NULL, "", 0, 0 };

Node *parsing_func;

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
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
    Node *node = new_node(ND_FUNCDEF, NULL, NULL);

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
        token = token->next;
        node = new_node(ND_RETURN, expr(), NULL);
        expect(";");
    } else if (token->kind == TK_IF) {
        token = token->next;
        node = new_node(ND_IF, NULL, NULL);

        expect("(");
        node->cond = expr();
        expect(")");

        node->then = stmt();
        if (token->kind == TK_ELSE) {
            token = token->next;
            node->els = stmt();
        }
    } else if (token->kind == TK_WHILE) {
        token = token->next;
        node = new_node(ND_WHILE, NULL, NULL);

        expect("(");
        node->cond = expr();
        expect(")");

        node->then = stmt();
    } else if (token->kind == TK_FOR) {
        token = token->next;
        node = new_node(ND_FOR, NULL, NULL);

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

    Node *node = new_node(ND_BLOCK, NULL, NULL);
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
    Node *node = new_node(ND_DECL, NULL, NULL);
    node->type = type();

    if (token->kind != TK_IDENT)
        error_at(token->str, "expected an identifier");
    
    LVar *lvar = find_lvar(token);
    if (lvar) {
        char *name = copy(token->str, token->len);
        error_at(token->str, "redeclaration of '%s'", name);
    } else {
        node->lvar = new_lvar(node->type, token);
        node->lvar->is_arg = false;
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

    Type *ty = calloc(1, sizeof(Type));
    ty->kind = TY_INT;
    ty->ptr_to = NULL;

    while (consume("*")) {
        Type *tmp = calloc(1, sizeof(Type));
        tmp->kind = TY_PTR;
        tmp->ptr_to = ty;
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
        node = new_node(ND_ASSIGN, node, assign());
    }

    return node;
}

Node *equality() {
    Node *node = relational();

    while (true) {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

Node *relational() {
    Node *node = add();

    while (true) {
        if (consume("<="))
            node = new_node(ND_LE, node, add());
        else if (consume(">="))
            node = new_node(ND_LE, add(), node);
        else if (consume("<"))
            node = new_node(ND_LT, node, add());
        else if (consume(">"))
            node = new_node(ND_LT, add(), node);
        else
            return node;
    }
}

Node *add() {
    Node *node = mul();

    while (true) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();

    while (true) {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary() {
    Node *node;

    if (consume("+"))
        node = primary();
    else if (consume("-"))
        node = new_node(ND_SUB, new_node_num(0), primary());
    else if (consume("&"))
        node = new_node(ND_ADDR, unary(), NULL);
    else if (consume("*"))
        node = new_node(ND_DEREF, unary(), NULL);
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
        Token *tok = token;
        token = token->next;
        if (consume("(")) {
            // function call
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

            node->funcname = copy(tok->str, tok->len);
        } else {
            node->kind = ND_LVAR;

            LVar *lvar = find_lvar(tok);
            if (lvar) {
                node->lvar = lvar;
            } else {
                char *name = copy(tok->str, tok->len);
                error_at(tok->str, "variable '%s' does not exist", name);
            }
        }
    } else {
        node = new_node_num(expect_number());
    }

    return node;
}

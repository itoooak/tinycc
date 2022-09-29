#include "tinycc.h"

Node *code[100];

// ローカル変数がそれ以前に存在していない場合でも
// localsのoffsetにアクセスできるように初期化
// (関数primary内でアクセスが起こる)
LVar locals_init = { NULL, "", 0, 0 };
LVar *locals = &locals_init;

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

LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next)
        if (var->len == tok->len &&
            !memcmp(tok->str, var->name, var->len)) {
            return var;
        }
    return NULL;
}

void program() {
    int idx = 0;
    while (!at_eof())
        code[idx++] = stmt();

    code[idx] = NULL;
    return;
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

        node->lhs = stmt();
        if (token->kind == TK_ELSE) {
            token = token->next;
            node->rhs = stmt();
        }
    } else if (token->kind == TK_WHILE) {
        token = token->next;
        node = new_node(ND_WHILE, NULL, NULL);

        expect("(");
        node->cond = expr();
        expect(")");

        node->lhs = stmt();
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
            node->lhs = expr();
            expect(")");
        }

        node->rhs = stmt();
    } else {
        node = expr();
        expect(";");
    }

    return node;
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
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(token);
        if (lvar) {
            node->offset = lvar->offset;
        } else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = token->str;
            lvar->len = token->len;
            lvar->offset = locals->offset + 8;
            node->offset = lvar->offset;
            locals = lvar;
        }
        token = token->next;
    } else {
        node = new_node_num(expect_number());
    }

    return node;
}

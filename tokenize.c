#include "tinycc.h"

char *user_input;
Token *token;

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

void expect(char *op) {
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error("expected %s", op);
    token = token->next;
}

int expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str, "expected a number");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    tok->len = len;
    return tok;
}

Token *new_token_num(Token *cur, int val) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = TK_NUM;
    tok->val = val;
    cur->next = tok;
    return tok;
}

Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (!memcmp(p, "==", 2) || !memcmp(p, "!=", 2)) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            cur->len = 2;
            p += 2;
            continue;
        }

        if (!memcmp(p, "<=", 2) || !memcmp(p, ">=", 2)) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            cur->len = 2;
            p += 2;
            continue;
        }

        if (*p == '<' || *p == '>') {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            cur->len = 1;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            cur->len = 1;
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token_num(cur, strtol(p, &p, 10));
            continue;
        }

        error_at(p, "不正なトークンです");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

/* tokenizer */

typedef enum {
    TK_RESERVED,
    TK_IDENT,       // Identifier
    TK_NUM,
    TK_EOF,         // End-of-file
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
} TokenKind;

typedef struct Token Token;
struct Token {
    TokenKind kind;
    Token *next;
    int val;        // Value of TK_NUM
    char *str;
    int len;        // Length of token
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
bool is_alnum(char c);

Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *new_token_num(Token *cur, int val);
Token *tokenize(char *p);

extern char *user_input;
extern Token *token;


/* parser */

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
    ND_EQ,
    ND_NE,
    ND_LT,      // less than
    ND_LE,
    ND_ASSIGN,
    ND_LVAR,    // local variable
    ND_RETURN,
    ND_IF,
    ND_WHILE,
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node *lhs;      // left-hand side
    Node *rhs;      // right-hand side
    Node *cond;     // condition of ND_IF
    int val;
    int offset;     // offset of ND_LVAR
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

// local variable
typedef struct LVar LVar;
struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
};

LVar *find_lvar(Token *tok);

void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

extern Node *code[100];
extern LVar *locals;


/* code generator */

void gen_lval(Node *node);
void gen(Node *node);

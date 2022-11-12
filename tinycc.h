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
    TK_FOR,
    TK_INT,
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
    TY_INT,
    TY_PTR,
} TypeKind;

typedef struct Type Type;
struct Type {
    TypeKind kind;
    Type *ptr_to;
};

// local variable
typedef struct LVar LVar;
struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
    Type *type;
    bool is_arg;
};

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
    ND_FOR,
    ND_BLOCK,
    ND_FUNCCALL,
    ND_FUNCDEF,
    ND_ADDR,
    ND_DEREF,
    ND_DECL,
} NodeKind;

#define ARG_NUM_MAX 6

typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node *lhs;      // left-hand side
    Node *rhs;      // right-hand side
    Node *cond;     // condition of ND_IF, ND_WHILE, ND_FOR
    Node *then;     // statement executed if cond holds in ND_IF, ND_WHILE, ND_FOR
    Node *els;      // statement executed if cond doesn't hold in ND_IF
    Node *init;     // initialization of ND_FOR
    Node *step;     // expression evaluated after iteration of ND_FOR
    Node *next;     // statement executed after this statement
    int val;        // value of ND_NUM
    LVar *lvar;     // ND_LVAR
    char *funcname; // name of function in ND_FUNCCALL
    Node *funcargs[ARG_NUM_MAX];
    int argsnum;
    Node *funcbody;
    LVar *locals;
    Type *type;
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

LVar *find_lvar(Token *tok);

void program();
Node *func_def();
Node *stmt();
Node *compound_stmt();
Node *declaration();
Type *type();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

extern Node *code[100];
extern Node *parsing_func;


/* code generator */

int size_of(Type *type);
int align_to(int cur_offset, int boundary);

void gen_addr(Node *node);
void gen_funcdef(Node *node);
void gen_funccall(Node *node);
void gen(Node *node);

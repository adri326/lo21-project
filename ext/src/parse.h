#ifndef PARSE_H
#define PARSE_H

#include <stdbool.h>
#include <rule.h>
#include <btree.h>
#include <linkedlist.h>
#include <inttypes.h>

#define BEGIN_PAREN '('
#define END_PAREN ')'

#define BEGIN_STRING '"'
#define END_STRING '"'

#define BEGIN_COMMENT '{'
#define END_COMMENT '}'

#define TERMINAL ';'
#define UNDERSCORE '_'

#define AND "&&"
#define OR "||"
#define THUS "=>"
#define NOT "!"
#define ERR "error"

enum EXPR_KIND {
    KIND_OR,
    KIND_NOT,
    KIND_AND,
    KIND_SYMBOL,
    KIND_ERROR
};

struct subexpr_ast {
    enum EXPR_KIND kind;
    char symbol[SYMBOL_LEN];
};
typedef struct subexpr_ast subexpr_ast;

struct ccl_symbol {
    bool negate;
    char symbol[SYMBOL_LEN];
};
typedef struct ccl_symbol ccl_symbol;

DECL_BT(subexpr_ast);
DECL_LL(ccl_symbol);

struct expr_ast {
    BT(subexpr_ast)* condition;
    LL(ccl_symbol)* conclusion;
};
typedef struct expr_ast expr_ast;

DECL_LL(expr_ast);

#endif // PARSE_H

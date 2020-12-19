#ifndef PARSE_H
#define PARSE_H

#include <stdbool.h>
#include <rule.h>
#include <btree.h>
#include <linkedlist.h>
#include <vec.h>

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
    KIND_ERROR,
    KIND_UNKNOWN = 99
};

/// Represents a node within the condition's AST
struct subexpr_ast {
    enum EXPR_KIND kind;
    char symbol[SYMBOL_LEN];
};
typedef struct subexpr_ast subexpr_ast;

/// Represents a symbol in a rule's conclusion, possibly negated
struct ccl_symbol {
    bool negate;
    char symbol[SYMBOL_LEN];
};
typedef struct ccl_symbol ccl_symbol;

DECL_BT(subexpr_ast);
DECL_VEC(ccl_symbol);

/// Represents a rule: contains an AST for its condition and a list of symbols for the conclusion
struct expr_ast {
    BT(subexpr_ast)* condition;
    VEC(ccl_symbol)* conclusion;
    const char* remaining_string;
};
typedef struct expr_ast expr_ast;

DECL_VEC(expr_ast);

/** Turns a raw string following the [grammar](../grammar.ebnf) into a set of expressions, whose conditions are stored as ASTs.

    @param raw - String following the grammar
    @returns VEC(expr_ast)*
    @throw If `raw` is not a string following the grammar or if a symbol is too long
**/
VEC(expr_ast)* ast_parse(const char* raw);

// Ordering: ||, &&, !
struct cond_and {
    VEC(ccl_symbol)* symbols;
};
typedef struct cond_and cond_and;

DECL_VEC(cond_and);

struct expr_flat {
    VEC(cond_and)* condition;
    VEC(ccl_symbol)* conclusion;
};
typedef struct expr_flat expr_flat;

DECL_VEC(expr_flat);

/** Turns AST-encoded expressions into flattenned expressions (`expr_flat`).
    The condition half is encoded as a list of "AND" expressions.
    The order of the operators (from outer to inner) of the final condition will be `||`, `&&` and `!`.

    The conclusion is left as-is.
    If `GENERATE_OPPOSITE` is defined, then the opposite expressions are generated if possible (if the opposite expression's condition would only be made up of one "AND" expression).

    @param ast - The `expr_ast` list to flatten
    @returns VEC(expr_flat)
**/
VEC(expr_flat)* flatten_expressions(VEC(expr_ast)* ast);

#endif // PARSE_H

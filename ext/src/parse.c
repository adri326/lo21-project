#include <string.h>
#include <assert.h>
#include <colors.h>
#include "parse.h"

DECL_BT_SOURCES(subexpr_ast);
DECL_BT_SOURCES_PRINTF_CUSTOM(subexpr_ast, {
    if (value.kind == KIND_SYMBOL) {
        printf("\"%s\"", value.symbol);
    } else if (value.kind == KIND_AND) {
        printf(CYAN("&&"));
    } else if (value.kind == KIND_OR) {
        printf(CYAN("||"));
    } else if (value.kind == KIND_NOT) {
        printf(GREEN("!"));
    }
})
DECL_VEC_SOURCES(ccl_symbol);
DECL_VEC_SOURCES(expr_ast);

struct ast_ctx {
    BT(subexpr_ast)* subexpr;
    const char* start_at;
    enum EXPR_KIND kind;
    bool negate_next;
};
typedef struct ast_ctx ast_ctx;

DECL_VEC(ast_ctx);
DECL_VEC_SOURCES(ast_ctx);

/** Puts `new_subexpr` on the current context's AST
    @param ctx - The current parsing context, will be mutated
    @param new_subexpr - The new node to be appended as part of an entry to the list of elements to be assembled together

    If ctx hasn't got an AST yet, then the new node is put as part of it.
    If ctx has an AST, then the new node and the previous AST are assembled together using an operator node, using ctx's stored kind.
    In that case, if ctx doesn't hold a kind (ie. when there wasn't any '&&' or '||' symbol), then the function fails.

    If ctx.negate_next is true, then it is set back to false and a NOT node is appended
**/
void push_subexpr(ast_ctx* ctx, BT(subexpr_ast)* new_subexpr) {
    if (ctx->negate_next) {
        subexpr_ast not_subexpr = {
            .kind = KIND_NOT,
            .symbol = {0}
        };
        new_subexpr = subexpr_ast_bt_connect(new_subexpr, NULL, not_subexpr);
        ctx->negate_next = false;
    }

    if (ctx->kind == KIND_AND || ctx->kind == KIND_OR) {
        subexpr_ast op_subexpr = {
            .kind = ctx->kind,
            .symbol = {0}
        };
        ctx->subexpr = subexpr_ast_bt_connect(ctx->subexpr, new_subexpr, op_subexpr);
    } else if (ctx->kind == KIND_UNKNOWN && ctx->subexpr == NULL) {
        ctx->subexpr = new_subexpr;
    } else {
        printf("Unexpected state: kind = %d, subexpr = %p; did you forget a '" CYAN("&&") "' or a '" CYAN("||") "'?\n", ctx->kind, ctx->subexpr);
        exit(2);
    }
}

/** Parses a single expression
    @param raw - The raw string to parse

    The parsing uses a stack machine to produce an AST for the condition of a rule.
    The conclusion of a rule is simple enough for it to directly generate the list of conclusion symbols.
**/
expr_ast ast_parse_sub(const char* raw) {
    assert(raw != NULL);
    VEC(ast_ctx)* stack = ast_ctx_vec_new(1);
    expr_ast res = {
        .conclusion = ccl_symbol_vec_new(1)
    };

    { // Push the initial context to the stack
        ast_ctx first_ctx = {
            .subexpr = NULL,
            .start_at = raw,
            .kind = KIND_UNKNOWN,
            .negate_next = false
        };
        ast_ctx_vec_push(stack, first_ctx);
    }

    bool in_comment = false;
    bool conclusion = false;

    while (*raw != '\0') {
        if (*raw == TERMINAL) break;
        else if (*raw == BEGIN_COMMENT) in_comment = true;
        else if (*raw == END_COMMENT) in_comment = false;
        else if (*raw == ' ') {
            raw++;
            continue;
        }

        // Main symbol-parsing part: pop the last ctx, modify it and push it back
        if (!in_comment) {
            ast_ctx ctx = ast_ctx_vec_pop(stack);

            if (*raw == '"') { // Parse symbol
                char symbol_name[SYMBOL_LEN];
                size_t symbol_length = 0;

                // Parses the body of the symbol
                while (*(++raw) != '"') {
                    symbol_name[symbol_length++] = *raw;
                    if (symbol_length >= SYMBOL_LEN - 1) {
                        printf("Error: symbol too long!\n");
                        exit(2);
                    }
                }

                symbol_name[symbol_length] = 0;

                // Appends the symbol to either the conclusion or the condition
                if (conclusion) {
                    ccl_symbol symbol = {
                        .negate = ctx.negate_next,
                    };
                    ctx.negate_next = false;
                    strcpy(symbol.symbol, symbol_name);
                    ccl_symbol_vec_push(res.conclusion, symbol);
                } else {
                    subexpr_ast symbol_subexpr = {
                        .kind = KIND_SYMBOL,
                    };
                    strcpy(symbol_subexpr.symbol, symbol_name);
                    push_subexpr(&ctx, subexpr_ast_bt_new(symbol_subexpr));
                }
            } else if (*raw == BEGIN_PAREN && !conclusion) { // Parse (
                // Creates a new context, pushes back ctx and then the new context
                ast_ctx new_ctx = {
                    .subexpr = NULL,
                    .start_at = raw,
                    .kind = KIND_UNKNOWN
                };
                ast_ctx_vec_push(stack, ctx);
                ast_ctx_vec_push(stack, new_ctx);
                raw++;
                continue;
            } else if (*raw == END_PAREN && !conclusion) { // Parse )
                // Pops the parent ctx and adds the current ctx as a node of it
                if (ast_ctx_vec_length(stack) == 0) {
                    printf("Too many '" CYAN(")") "'!\n");
                    exit(2);
                }

                ast_ctx parent_ctx = ast_ctx_vec_pop(stack);

                push_subexpr(&parent_ctx, ctx.subexpr);

                ctx = parent_ctx;
            // TODO: use `THUS`
            } else if (*raw == '=' && raw[1] == '>' && !conclusion) { // Parse =>
                raw++;
                conclusion = true;
            // TODO: use `AND`
            } else if (*raw == '&' && raw[1] == '&') { // Parse &&
                raw++;
                if (!conclusion) {
                    if (ctx.kind == KIND_UNKNOWN || ctx.kind == KIND_AND) {
                        ctx.kind = KIND_AND;
                    } else {
                        printf("Operator precedence is not supported!\n");
                        exit(2);
                    }
                }
            } else if (*raw == '|' && raw[1] == '|') { // Parse ||
                raw++;
                if (!conclusion) {
                    if (ctx.kind == KIND_UNKNOWN || ctx.kind == KIND_OR) {
                        ctx.kind = KIND_OR;
                    } else {
                        printf("Operator precedence is not supported!\n");
                        exit(2);
                    }
                } else {
                    printf("Using '" CYAN("||") "' in the conclusion is not suported!");
                    exit(2);
                }
            } else if (*raw == '!') { // Parse !
                if (ctx.negate_next) {
                    printf("Double '" CYAN("!") "' detected; did you forget a '" CYAN("(") "'?\n");
                    exit(2);
                }
                ctx.negate_next = true;
            } else { // Uh oh!
                printf("Unexpected symbol: '" CYAN("%c") "'\n", *raw);
                exit(2);
            }

            ast_ctx_vec_push(stack, ctx);
        }

        raw++;
    }

    if (!conclusion) {
        printf("Rule has no conclusion! Did you forget a '" CYAN("=>") "'?\n");
        exit(2);
    }

    res.condition = ast_ctx_vec_get(stack, 0)->subexpr;
    res.remaining_string = raw;
    subexpr_ast_bt_printf(res.condition);
    ast_ctx_vec_free(stack);
    return res;
}

VEC(expr_ast)* ast_parse(const char* raw) {
    size_t n_expr = 0;

    for (const char* tmp = raw; *tmp != '\0'; tmp++) {
        if (*tmp == TERMINAL) n_expr++;
    }

    VEC(expr_ast)* res = expr_ast_vec_new(n_expr);

    for (size_t n = 0; n < n_expr; n++) {
        expr_ast expr = ast_parse_sub(raw);
        raw = expr.remaining_string + 1; // + 1 to accompass for the `;`
        expr_ast_vec_push(res, expr);
    }

    while (*raw != '\0') {
        if (*raw != ' ' && *raw != '\n') {
            printf("Unexpected character: '" CYAN("%c") "', did you forget a '" CYAN(";") "'?\n", *raw);
            exit(2);
        }
    }

    return res;
}

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
});
DECL_VEC_SOURCES(ccl_symbol);
DECL_VEC_SOURCES_PRINTF_CUSTOM(ccl_symbol, {
    printf("%s%s", value.negate ? "!" : "", value.symbol);
})

DECL_VEC_SOURCES(expr_ast);

DECL_VEC_SOURCES(cond_and);
DECL_VEC_SOURCES_PRINTF_CUSTOM(cond_and, {
    printf("\n");
    for (size_t n = 0; n < ccl_symbol_vec_length(value.symbols); n++) {
        printf("%s%s ", ccl_symbol_vec_get(value.symbols, n)->negate ? "!" : "", ccl_symbol_vec_get(value.symbols, n)->symbol);
    }
});

DECL_VEC_SOURCES(expr_flat);
DECL_VEC_SOURCES_PRINTF_CUSTOM(expr_flat, {
    for (size_t n = 0; n < cond_and_vec_length(value.condition); n++) {
        if (n != 0) printf(" || ");
        printf("(");
        VEC(ccl_symbol)* symbols = cond_and_vec_get(value.condition, n)->symbols;
        for (size_t o = 0; o < ccl_symbol_vec_length(symbols); o++) {
            if (o != 0) printf(" && ");
            printf("%s%s", ccl_symbol_vec_get(symbols, o)->negate ? "!" : "", ccl_symbol_vec_get(symbols, o)->symbol);
        }
        printf(")");
    }
    printf(" => ");

    for (size_t o = 0; o < ccl_symbol_vec_length(value.conclusion); o++) {
        if (o != 0) printf(" && ");
        printf("%s%s", ccl_symbol_vec_get(value.conclusion, o)->negate ? "!" : "", ccl_symbol_vec_get(value.conclusion, o)->symbol);
    }
})

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
    enum EXPR_KIND kind = KIND_THUS;

    while (*raw != '\0') {
        if (*raw == TERMINAL) break;
        else if (*raw == BEGIN_COMMENT) in_comment = true;
        else if (*raw == END_COMMENT) {
            in_comment = false;
            raw++;
            continue;
        }
        else if (*raw == ' ' || *raw == '\n') {
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
            } else if (*raw == '<' && raw[1] == '=' && raw[2] == '>' && !conclusion) { // Parse <=>
                raw += 2;
                conclusion = true;
                kind = KIND_EQV;
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
    if (kind == KIND_EQV && ccl_symbol_vec_length(res.conclusion) != 1) {
        printf("A rule with an equivalence symbol ('" CYAN("<=>") "' is expected to have exactly one conclusion symbol!\n");
        exit(2);
    }
    if (ccl_symbol_vec_length(res.conclusion) == 0) {
        printf("Rule has no conclusion symbol!\n");
        exit(2);
    }

    res.kind = kind;
    res.condition = ast_ctx_vec_get(stack, 0)->subexpr;
    res.remaining_string = raw;
    // subexpr_ast_bt_printf(res.condition);
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
        raw++;
    }

    return res;
}

VEC(expr_ast)* ast_expand_eqv(VEC(expr_ast)* ast) {
    VEC(expr_ast)* res = expr_ast_vec_new(expr_ast_vec_length(ast));
    for (size_t n = 0; n < expr_ast_vec_length(ast); n++) {
        if (expr_ast_vec_get(ast, n)->kind == KIND_THUS) {
            expr_ast_vec_push(res, *expr_ast_vec_get(ast, n));
        } else if (expr_ast_vec_get(ast, n)->kind == KIND_EQV) {
            expr_ast e = *expr_ast_vec_get(ast, n);
            e.kind = KIND_THUS;

            // Push regular expression
            expr_ast_vec_push(res, e);

            // Clone condition and conclusion
            e.condition = subexpr_ast_bt_clone(e.condition);
            e.conclusion = ccl_symbol_vec_clone(e.conclusion);

            // Invert conclusion symbol
            ccl_symbol* ccl_symbol = ccl_symbol_vec_get(e.conclusion, 0);
            ccl_symbol->negate = !ccl_symbol->negate;

            // Invert condition
            e.condition = subexpr_ast_bt_connect(e.condition, NULL, (subexpr_ast){
                .kind = KIND_NOT,
                .symbol = ""
            });

            // Push inverted expression
            expr_ast_vec_push(res, e);
        }
    }

    expr_ast_vec_free(ast);
    return res;
}

/// Flattens a single subexpression
VEC(cond_and)* flatten_subexpr(BT(subexpr_ast)* subexpr, bool negate) {
    if (subexpr == NULL) return cond_and_vec_new(1);

    if (subexpr->value.kind == KIND_AND && !negate || subexpr->value.kind == KIND_OR && negate) {
        VEC(cond_and)* left = flatten_subexpr(subexpr_ast_bt_left(subexpr), negate);
        VEC(cond_and)* right = flatten_subexpr(subexpr_ast_bt_right(subexpr), negate);
        VEC(cond_and)* res = cond_and_vec_new(cond_and_vec_length(left) * cond_and_vec_length(right));

        // return left x right
        for (size_t n = 0; n < cond_and_vec_length(left); n++) {
            for (size_t o = 0; o < cond_and_vec_length(right); o++) {
                VEC(ccl_symbol)* left_symbols = cond_and_vec_get(left, n)->symbols;
                VEC(ccl_symbol)* right_symbols = cond_and_vec_get(right, 0)->symbols;
                VEC(ccl_symbol)* merged_symbols = ccl_symbol_vec_new(
                    ccl_symbol_vec_length(left_symbols)
                    + ccl_symbol_vec_length(right_symbols)
                );
                for (size_t p = 0; p < ccl_symbol_vec_length(left_symbols); p++) {
                    ccl_symbol_vec_push(merged_symbols, *ccl_symbol_vec_get(left_symbols, p));
                }
                for (size_t p = 0; p < ccl_symbol_vec_length(right_symbols); p++) {
                    ccl_symbol_vec_push(merged_symbols, *ccl_symbol_vec_get(right_symbols, p));
                }
                cond_and_vec_push(res, (cond_and){.symbols = merged_symbols});
            }
        }

        for (size_t n = 0; n < cond_and_vec_length(left); n++) {
            ccl_symbol_vec_free(cond_and_vec_get(left, n)->symbols);
        }

        for (size_t n = 0; n < cond_and_vec_length(right); n++) {
            ccl_symbol_vec_free(cond_and_vec_get(right, n)->symbols);
        }

        cond_and_vec_free(left);
        cond_and_vec_free(right);
        return res;
    } else if (subexpr->value.kind == KIND_OR && !negate || subexpr->value.kind == KIND_AND && negate) {
        VEC(cond_and)* left = flatten_subexpr(subexpr_ast_bt_left(subexpr), negate);
        VEC(cond_and)* right = flatten_subexpr(subexpr_ast_bt_right(subexpr), negate);
        cond_and_vec_resize(left, cond_and_vec_length(left) + cond_and_vec_length(right));

        for (size_t n = 0; n < cond_and_vec_length(right); n++) {
            cond_and_vec_push(left, *cond_and_vec_get(right, n));
        }

        cond_and_vec_free(right);
        return left;
    } else if (subexpr->value.kind == KIND_NOT) {
        return flatten_subexpr(subexpr_ast_bt_left(subexpr), !negate);
    } else if (subexpr->value.kind == KIND_SYMBOL) {
        VEC(cond_and)* res = cond_and_vec_new(1);
        VEC(ccl_symbol)* res_symbols = ccl_symbol_vec_new(1);

        ccl_symbol res_symbol = {
            .negate = negate
        };
        strcpy(res_symbol.symbol, subexpr->value.symbol);
        ccl_symbol_vec_push(res_symbols, res_symbol);

        cond_and_vec_push(res, (cond_and){.symbols = res_symbols});

        return res;
    }
}

// Consumes ast
VEC(expr_flat)* flatten_expressions(VEC(expr_ast)* ast) {
    VEC(expr_flat)* res = expr_flat_vec_new(expr_ast_vec_length(ast));

    for (size_t n = 0; n < expr_ast_vec_length(ast); n++) {
        expr_ast* curr_ast = expr_ast_vec_get(ast, n);
        expr_flat_vec_push(res, (expr_flat){
            .condition = flatten_subexpr(curr_ast->condition, false),
            .conclusion = curr_ast->conclusion
        });

#ifdef GENERATE_OPPOSITE
        VEC(cond_and)* opposite_condition = flatten_subexpr(curr_ast->condition, true);
        if (cond_and_vec_length(opposite_condition) == 1) {
            printf("Generating opposite rule for rule #%zu\n", n);
            VEC(ccl_symbol)* conclusion = cond_and_vec_get(opposite_condition, 0)->symbols;
            VEC(cond_and)* condition = cond_and_vec_new(ccl_symbol_vec_length(curr_ast->conclusion));
            for (size_t o = 0; o < ccl_symbol_vec_length(curr_ast->conclusion); o++) {
                VEC(ccl_symbol)* symbols = ccl_symbol_vec_new(1);
                ccl_symbol symbol = {
                    .negate = !ccl_symbol_vec_get(curr_ast->conclusion, o)->negate
                };
                strcpy(symbol.symbol, ccl_symbol_vec_get(curr_ast->conclusion, o)->symbol);
                ccl_symbol_vec_push(symbols, symbol);
                cond_and_vec_push(condition, (cond_and){
                    .symbols = symbols
                });
            }

            expr_flat_vec_push(res, (expr_flat){
                .condition = condition,
                .conclusion = conclusion
            });
        }
        cond_and_vec_free(opposite_condition);
#endif // GENERATE_OPPOSITE

        subexpr_ast_bt_free(curr_ast->condition);
    }

    expr_ast_vec_free(ast);
    return res;
}

knowledgebase_t* simplify_expressions(VEC(expr_flat)* expressions) {
    knowledgebase_t* res = NULL;

    for (size_t n = 0; n < expr_flat_vec_length(expressions); n++) {
        expr_flat* expr = expr_flat_vec_get(expressions, n);
        for (size_t d = 0; d < cond_and_vec_length(expr->condition); d++) {
            VEC(ccl_symbol)* condition = cond_and_vec_get(expr->condition, d)->symbols;
            for (size_t l = 0; l < ccl_symbol_vec_length(expr->conclusion); l++) {
                rule_t* rule = NULL;
                for (size_t o = 0; o < ccl_symbol_vec_length(condition); o++) {
                    // Convert condition ccl_symbol to char[]
                    char symbol[SYMBOL_LEN];
                    if (strlen(ccl_symbol_vec_get(condition, o)->symbol) >= SYMBOL_LEN - 1) {
                        printf("Symbol length too long!\n");
                        exit(2);
                    }
                    sprintf(symbol, "%s%s",
                        ccl_symbol_vec_get(condition, o)->negate ? "!" : "",
                        ccl_symbol_vec_get(condition, o)->symbol
                    );
                    // Push condition symbol
                    rule = push_symbol(rule, symbol);
                }

                // Convert conclusion symbol to char[]
                char conclusion[SYMBOL_LEN];
                if (strlen(ccl_symbol_vec_get(expr->conclusion, l)->symbol) >= SYMBOL_LEN - 1) {
                    printf("Symbol length too long!\n");
                    exit(2);
                }
                sprintf(conclusion, "%s%s",
                    ccl_symbol_vec_get(expr->conclusion, l)->negate ? "!" : "",
                    ccl_symbol_vec_get(expr->conclusion, l)->symbol
                );
                // Push conclusion symbol
                rule = push_symbol_conclusion(rule, conclusion);
                res = push_rule(res, rule);
            }
        }
    }

    return res;
}

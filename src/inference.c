#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "colors.h"
#include "inference.h"

bool symbols_contain(const symbols_t* hay, const char* needle) {
    if (hay == NULL) return false;
    else if (strcmp(hay->symbol, needle) == 0) return true;
    else return symbols_contain(hay->next, needle);
}

bool is_condition_true(rule_t* rule, const symbols_t* symbols) {
    bool res = true;
    // If the rule has a condition...
    if (!is_condition_empty(rule)) {
        rule_t* tmp = rule;
        // Traverse the condition of the rule...
        while (tmp->next != NULL && res) {
            // If the current conditional symbol isn't true yet, then return false
            if (!symbols_contain(symbols, tmp->symbol)) res = false;
            tmp = tmp->next;
        }
    }
    return res;
}

symbols_t* inference_engine(knowledgebase_t* base, const symbols_t* input_symbols) {
    symbols_t* res = NULL;

    // Clone `input_symbols` into `res`
    {
        const symbols_t* tmp = input_symbols;
        while (tmp != NULL) {
            res = new_rule(tmp->symbol, res);
            tmp = tmp->next;
        }
    }

    // Main loop: repeat until no symbol gets added to `res` (tracked by has_added)
    bool has_added = false;
    do {
        has_added = false;

        // For every rule of the knowledge base...
        knowledgebase_t* tmp = base;
        while (tmp != NULL) {
            assert(tmp->rule != NULL);

            // If the conclusion of the rule doesn't belong to `res` yet...
            if (!symbols_contain(res, rule_conclusion(kb_head(tmp)))) {
                // If the condition is true, then we add the conclusion to `res`
                if (is_condition_true(kb_head(tmp), res)) {
                    #ifdef PRINT_ERRORS
                        if (strcmp(rule_conclusion(kb_head(tmp)), "error") == 0) {
                            print_error(kb_head(tmp), res);
                        }
                    #endif // PRINT_ERRORS

                    res = new_rule(rule_conclusion(kb_head(tmp)), res);
                    has_added = true;
                }
            }

            tmp = tmp->next;
        }
    } while (has_added);
    return res;
}

void print_symbols(const char* header, const symbols_t* symbols) {
    printf(GRAY("\u250f\u2509") BLUE(" %s:\n"), header);
    printf(GRAY("\u2517 "));
    if (symbols == NULL) {
        printf(CYAN("<none>"));
    }
    while (symbols != NULL) {
        printf("%s ", symbols->symbol);
        symbols = symbols->next;
    }
    printf("\n");
}

// Returns `elem ∈ set`
bool symbol_in(const char* elem, const symbols_t* set) {
    while (set != NULL) {
        if (strcmp(set->symbol, elem) == 0) return true;
        set = set->next;
    }

    return false;
}

void print_symbols_diff(const char* header, const symbols_t* symbols, const symbols_t* diff_symbols) {
    printf(GRAY("\u250f\u2509") BLUE(" %s:\n"), header);
    printf(GRAY("\u2517 "));
    if (symbols == NULL) {
        printf(CYAN("<none>"));
    }
    while (symbols != NULL) {
        if (strcmp(symbols->symbol, "error") == 0) {
            printf(RED("%s "), symbols->symbol);
        } else if (symbol_in(symbols->symbol, diff_symbols)) {
            printf("%s ", symbols->symbol);
        } else {
            printf(GREEN("%s "), symbols->symbol);
        }
        symbols = symbols->next;
    }
    printf("\n");
}

void print_error(const rule_t* rule, const symbols_t* current_symbols) {
    printf(GRAY("\u250f\u2509") RED(" Error:\n"));
    printf(GRAY("\u2520 "));
    print_rule(rule);

    printf(GRAY("\u2503  ") BLUE("Current symbols:\n"));
    printf(GRAY("\u2517 "));

    while (current_symbols != NULL) {
        if (symbol_in(current_symbols->symbol, rule)) {
            printf(RED("%s "), current_symbols->symbol);
        } else {
            printf("%s ", current_symbols->symbol);
        }
        current_symbols = current_symbols->next;
    }
    printf("\n");
}

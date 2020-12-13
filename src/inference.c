#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
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
    printf("\u250f\u2509 %s:\n", header);
    printf("\u2517 ");
    while (symbols != NULL) {
        printf("%s ", symbols->symbol);
        symbols = symbols->next;
    }
    printf("\n");
}

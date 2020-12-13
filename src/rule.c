#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "rule.h"
#include "colors.h"

void free_rule(rule_t* rule) {
    if (rule != NULL) {
        free_rule(rule->next);
        free(rule);
    }
}

rule_t* new_rule(const char* symbol, rule_t* next) {
    assert(strlen(symbol) < SYMBOL_LEN);
    rule_t* res = calloc(SYMBOL_LEN, sizeof(char));
    assert(!!strcpy(res->symbol, symbol));
    res->next = next;
    return res;
}

// 1.1
rule_t* empty_rule() {
    return NULL;
}

#ifdef FAITHFUL_IMPLEMENTATION // Faithful implementation of push_symbol, slower

    // 1.2
    rule_t* push_symbol(rule_t* rule, char* symbol) {
        if (rule == NULL) {
            return new_rule(symbol, NULL);
        } else {
            // Get to the last element
            rule_t* tmp_rule = rule;
            while (tmp_rule->next != NULL) {
                tmp_rule = tmp_rule->next;
            }

            tmp_rule->next = new_rule(symbol, NULL);

            return rule;
        }
    }

    // 1.3
    rule_t* push_symbol_conclusion(rule_t* rule, char* symbol) {
        return push_symbol(rule, symbol);
    }

#else // Faster implementation of push_symbol, albeit unfaithful to the description given in the report

    rule_t* push_symbol(rule_t* rule, char* symbol) {
        return new_rule(symbol, rule);
    }

    rule_t* push_symbol_conclusion(rule_t* rule, char* symbol) {
        if (rule == NULL) {
            return new_rule(symbol, NULL);
        } else {
            // Get to the last element
            rule_t* tmp_rule = rule;
            while (tmp_rule->next != NULL) {
                tmp_rule = tmp_rule->next;
            }

            tmp_rule->next = new_rule(symbol, NULL);

            return rule;
        }
    }

#endif


// 1.4
bool is_symbol_in_condition(rule_t* rule, char* symbol) {
    if (rule == NULL) {
        // Empty rule
        return false;
    } else if (rule->next == NULL) {
        // Conclusion of the rule
        return false;
    } else if (strcmp(rule->symbol, symbol) == 0) {
        // Symbol found
        return true;
    } else {
        return is_symbol_in_condition(rule->next, symbol);
    }
}

// 1.5
rule_t* remove_symbol(rule_t* rule, char* symbol) {
    if (rule == NULL) {
        // Empty rule
        return NULL;
    } else if (rule->next == NULL) {
        // Conclusion of the rule
        return rule;
    } else {
        if (strcmp(rule->symbol, symbol) == 0) {
            // Return the list, minus this node (also frees the node)
            rule_t* res = remove_symbol(rule->next, symbol);
            free(rule);
            return res;
        } else {
            rule->next = remove_symbol(rule->next, symbol);
            return rule;
        }
    }
}

// 1.6
bool is_condition_empty(rule_t* rule) {
    if (rule == NULL) {
        // Empty rule: its condition is also empty
        return true;
    } else if (rule->next == NULL) {
        return true;
    } else {
        return false;
    }
}

// 1.7
char* rule_condition_head(rule_t* rule) {
    if (is_condition_empty(rule)) return NULL;
    else return rule->symbol;
}

// 1.8
char* rule_conclusion(rule_t* rule) {
    if (rule->next == NULL) return rule->symbol;
    else {
        rule_t* tmp_rule = rule;
        // Go to the last symbol in the list
        while (tmp_rule->next != NULL) tmp_rule = tmp_rule->next;
        return tmp_rule->symbol;
    }
}

void print_rule(const rule_t* rule) {
    if (rule == NULL) {
        printf("\n");
        return;
    }
    if (rule->next == NULL) printf("%s\n", rule->symbol);
    bool first = true;
    while (rule != NULL) {
        if (rule->next == NULL) printf(CYAN("=>") " %s\n", rule->symbol);
        else {
            if (first) printf("%s ", rule->symbol);
            else printf(CYAN("&") " %s ", rule->symbol);
            first = false;
        }
        rule = rule->next;
    }
}

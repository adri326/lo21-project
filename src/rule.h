#ifndef RULE_H
#define RULE_H

#include <stdlib.h>
#include <stdbool.h>

#define SYMBOL_LEN 256

struct rule {
    char symbol[SYMBOL_LEN];
    struct rule* next;
};

typedef struct rule rule_t;

/** Frees a rule and its symbol array (if it isn't a slice). Slices' borrowed memory will be unusable after that call. **/
void free_rule(rule_t* rule);

/** [1.1] Returns an empty rule, with a capacity of 1. **/
rule_t* empty_rule();

/** [1.2] Adds a symbol to the condition of a rule. **/
rule_t* push_symbol(rule_t* rule, char* symbol);

/** [1.3] Adds a symbol to the conclusion of a rule. **/
rule_t* push_symbol_conclusion(rule_t* rule, char* symbol);

/** [1.4] Returns whether or not `symbol` is a member of the condition of a rule. **/
bool is_symbol_in_condition(rule_t* rule, char* symbol);

/** [1.5] Deletes all occurences of a symbol from the condition of a rule. Frees the removed nodes. **/
rule_t* remove_symbol(rule_t* rule, char* symbol);

/** [1.6] Returns whether or not the condition of a rule is empty. **/
bool is_condition_empty(rule_t* rule);

/** [1.7] Returns the symbol at the head of the condition. **/
char* rule_condition_head(rule_t* rule);

/** [1.8] Returns the conclusion symbol of a rule. **/
char* rule_conclusion(rule_t* rule);

#endif // RULE_H

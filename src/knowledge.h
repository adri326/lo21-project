#ifndef KNOWLEDGE_H
#define KNOWLEDGE_H

#include "rule.h"
#define KB_REALLOC_FACT 2

struct knowledgebase {
    rule_t* rule;
    struct knowledgebase* next;
};

typedef struct knowledgebase knowledgebase_t;

/// Frees a base
void free_kb(knowledgebase_t* base);

/// Creates a new node with
knowledgebase_t* new_kb_node(knowledgebase_t* next, rule_t* rule);

/// [2.1] Create an empty base
knowledgebase_t* empty_kb();

/// [2.2] Pushes a rule to a knowledgebase to its head
knowledgebase_t* push_rule(knowledgebase_t* base, rule_t* rule);

/// [2.3] Get the rule at the head of the list
rule_t* kb_head(knowledgebase_t* base);

/// Prints out a visual representation of a knowledgebase
void print_kb(const knowledgebase_t* base);

#endif // KNOWLEDGE_H

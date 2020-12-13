#include <assert.h>
#include <stdio.h>
#include "knowledge.h"

void free_kb(knowledgebase_t* base) {
    if (base != NULL) {
        free_kb(base->next);
        free(base);
    }
}

knowledgebase_t* new_kb_node(knowledgebase_t* next, rule_t* rule) {
    knowledgebase_t* res = (knowledgebase_t*)malloc(sizeof(struct knowledgebase));

    assert(res != NULL);

    res->next = next;
    res->rule = rule;

    return res;
}

// 2.1
knowledgebase_t* empty_kb() {
    return NULL;
}

// 2.2
knowledgebase_t* push_rule(knowledgebase_t* base, rule_t* rule) {
    return new_kb_node(base, rule);
}

// 2.3
rule_t* kb_head(knowledgebase_t* base) {
    if (base == NULL) return NULL;
    return base->rule;
}

void print_kb(const knowledgebase_t* base) {
    printf("\u250f\u2509 Knowledge Base:\n");
    printf("\u2503\n");
    while (base != NULL) {
        printf("\u2520 ");
        print_rule(base->rule);
        base = base->next;
    }
    printf("\u2579\n");
}

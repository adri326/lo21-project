#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>
#include "rule.h"
#include "knowledge.h"
#include "inference.h"

/// Returns a === b
bool symbols_eq(const symbols_t* a, const symbols_t* b) {
    const symbols_t* tmp = a;
    while (tmp != NULL) {
        if (!symbols_contain(b, tmp->symbol)) return false;
        tmp = tmp->next;
    }

    tmp = b;
    while (tmp != NULL) {
        if (!symbols_contain(a, tmp->symbol)) return false;
        tmp = tmp->next;
    }

    return true;
}

// Test 1: A & B => C
void test_1() {
    printf("Test 1:\n");
    rule_t* rule = empty_rule();
    rule = push_symbol(rule, "A");
    rule = push_symbol(rule, "B");
    rule = push_symbol_conclusion(rule, "C");

    knowledgebase_t* kb = empty_kb();
    kb = push_rule(kb, rule);

    print_kb(kb);

    symbols_t* symbols = NULL;
    symbols = push_symbol(symbols, "A");
    symbols = push_symbol(symbols, "B");

    print_symbols("Input symbols", symbols);

    symbols_t* res_a = inference_engine(kb, symbols);

    print_symbols("Output symbols", res_a);

    symbols_t* res_b = inference_engine(kb, NULL);

    symbols = push_symbol(symbols, "C"); // symbols === [A, B, C]
    assert(symbols_eq(res_a, symbols)); // res_a === [A, B, C]
    assert(symbols_eq(res_b, NULL)); // res_b === []

    free_rule(rule);
    free_rule(symbols);
    free_rule(res_a);
    free_rule(res_b);
    free_kb(kb);
}

int main(int argc, char* argv[]) {
    test_1();
}

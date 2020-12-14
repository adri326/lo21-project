#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>
#include "rule.h"
#include "knowledge.h"
#include "inference.h"
#include "colors.h"

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
    printf(GRAY("\n==> ") "Test 1:\n\n");
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

    print_symbols_diff("Output symbols", res_a, symbols);

    symbols_t* res_b = inference_engine(kb, NULL);

    symbols = push_symbol(symbols, "C"); // symbols === [A, B, C]
    assert(symbols_eq(res_a, symbols)); // res_a === [A, B, C]
    assert(symbols_eq(res_b, NULL)); // res_b === []

    free_rule(rule);
    free_symbols(symbols);
    free_symbols(res_a);
    free_symbols(res_b);
    free_kb(kb);

    printf("\n");
}

// Test 2:
// A & B => D
// A & C => D
// B & C => D
void test_2() {
    printf(GRAY("\n==> ") "Test 2:\n\n");
    rule_t* rule_1 = empty_rule();
    rule_1 = push_symbol(rule_1, "A");
    rule_1 = push_symbol(rule_1, "B");
    rule_1 = push_symbol_conclusion(rule_1, "D");

    rule_t* rule_2 = empty_rule();
    rule_2 = push_symbol(rule_2, "A");
    rule_2 = push_symbol(rule_2, "C");
    rule_2 = push_symbol_conclusion(rule_2, "D");

    rule_t* rule_3 = empty_rule();
    rule_3 = push_symbol(rule_3, "B");
    rule_3 = push_symbol(rule_3, "C");
    rule_3 = push_symbol_conclusion(rule_3, "D");

    knowledgebase_t* kb = empty_kb();
    kb = push_rule(kb, rule_1);
    kb = push_rule(kb, rule_2);
    kb = push_rule(kb, rule_3);

    print_kb(kb);

    for (size_t n = 0; n < 3; n++) {
        printf(GRAY("\n => ") CYAN("Sub-test 2.%zu:\n\n"), n + 1);
        symbols_t* symbols = NULL;
        symbols = push_symbol(symbols, n < 2 ? "A" : "B");
        symbols = push_symbol(symbols, n == 0 ? "B" : "C");

        print_symbols("Input symbols", symbols);

        symbols_t* res = inference_engine(kb, symbols);

        print_symbols_diff("Output symbols", res, symbols);

        symbols = push_symbol(symbols, "D"); // symbols === [x, y, D]
        assert(symbols_eq(res, symbols)); // res === [x, y, D]
        free_symbols(res);
        free_symbols(symbols);
    }

    free_rule(rule_1);
    free_rule(rule_2);
    free_rule(rule_3);
    free_kb(kb);

    printf("\n");
}

int main(int argc, char* argv[]) {
    test_1();
    test_2();
}

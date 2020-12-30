#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
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

/** Test 1:
    A simple test, with as only rule "A & B => C"

    The inference engine is ran twice, once with as input "A" and "B", and once with no input.
    The expected results are:

    - A, B --> A, B, C
    - ∅ --> ø
**/
void test_1(bool verbose) {
    printf(GRAY("\n==> ") "Test 1:\n\n");

    // Rule 1: A & B => C
    rule_t* rule = empty_rule();
    rule = push_symbol(rule, "A");
    rule = push_symbol(rule, "B");
    rule = push_symbol_conclusion(rule, "C");

    knowledgebase_t* kb = empty_kb();
    kb = push_rule(kb, rule);

    print_kb(kb);

    // Input symbols: A, B
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

    printf(GRAY("\n-> ") "Test 1: " GREEN("success!") "\n\n");
}

// Test 2:
// A & B => D
// A & C => D
// B & C => D
/** Test 2
    A more complicated test, which sets "D" to true if any two of "A", "B" and "C" are true (ie. "A & B", "A & C" or "B & C").

    The small loop within this test tries each of these combinations and expects "D" to be set to true:

    - A, B --> A, B, D
    - A, C --> A, C, D
    - B, C --> B, C, D
**/
void test_2(bool verbose) {
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
        if (verbose) printf(GRAY("\n => ") CYAN("Sub-test 2.%zu:\n\n"), n + 1);
        symbols_t* symbols = NULL;
        symbols = push_symbol(symbols, n < 2 ? "A" : "B");
        symbols = push_symbol(symbols, n == 0 ? "B" : "C");

        if (verbose) print_symbols("Input symbols", symbols);

        symbols_t* res = inference_engine(kb, symbols);

        if (verbose) print_symbols_diff("Output symbols", res, symbols);

        symbols = push_symbol(symbols, "D"); // symbols === [x, y, D]
        assert(symbols_eq(res, symbols)); // res === [x, y, D]

        free_symbols(res);
        free_symbols(symbols);
    }

    free_rule(rule_1);
    free_rule(rule_2);
    free_rule(rule_3);
    free_kb(kb);

    printf(GRAY("\n-> ") "Test 2: " GREEN("success!") "\n\n");
}

// Test 3:
// Show that (A || B) & C => (A & C) || B
// <=>
// a)
// A => tmp
// B => tmp
// tmp & C => D
// b)
// A & C => E
// B => E
// c)
// If ∀A, ∀B, ∀C, D -> E, then (A || B) & C => (A & C) || B
/** Test 3:
    This test aims to prove a simple sentence: (A || B) & C => (A & C) || B

    To do this, the `&` is distributed over (A || B) and each possibility is coded as a simple rule.
    The left-hand side of the relationship yields the value "D", while the right-hand side yields the value "E".
    We obtain the following rules:

    - A => tmp
    - B => tmp
    - tmp & C => D
    - A & C => E
    - B => E

    We then check that for every combination of "A", "B" and "C", "D" is false or "E" is true.

    The loop generates the 8 different sets of symbols; the expected outputs are thus:

    - ∅ --> ∅
    - A --> A, tmp
    - B --> B, E, tmp
    - A, B --> A, B, E, tmp
    - C --> C,
    - A, C --> A, C, D, E, tmp
    - B, C --> B, C, D, E, tmp
    - A, B, C --> A, B, C, D, E, tmp

    You can see that for each result, either "D" is false or both "D" and "E" are true.
**/
void test_3(bool verbose) {
    printf(GRAY("\n==> ") "Test 3:\n\n");
    rule_t* rule_1 = empty_rule();
    rule_1 = push_symbol(rule_1, "A");
    rule_1 = push_symbol_conclusion(rule_1, "tmp");

    rule_t* rule_2 = empty_rule();
    rule_2 = push_symbol(rule_2, "B");
    rule_2 = push_symbol_conclusion(rule_2, "tmp");

    rule_t* rule_3 = empty_rule();
    rule_3 = push_symbol(rule_3, "tmp");
    rule_3 = push_symbol(rule_3, "C");
    rule_3 = push_symbol_conclusion(rule_3, "D");

    rule_t* rule_4 = empty_rule();
    rule_4 = push_symbol(rule_4, "A");
    rule_4 = push_symbol(rule_4, "C");
    rule_4 = push_symbol_conclusion(rule_4, "E");

    rule_t* rule_5 = empty_rule();
    rule_5 = push_symbol(rule_5, "B");
    rule_5 = push_symbol_conclusion(rule_5, "E");

    knowledgebase_t* kb = empty_kb();
    kb = push_rule(kb, rule_1);
    kb = push_rule(kb, rule_2);
    kb = push_rule(kb, rule_3);
    kb = push_rule(kb, rule_4);
    kb = push_rule(kb, rule_5);

    print_kb(kb);

    bool is_rule_coherent = true;
    for (size_t n = 0; n < 8; n++) {
        if (verbose) printf(GRAY("\n => ") CYAN("Sub-test 3.%zu:\n\n"), n + 1);
        symbols_t* symbols = NULL;
        if ((n & 1) == 0) symbols = push_symbol(symbols, "A");
        if (((n >> 1) & 1) == 0) symbols = push_symbol(symbols, "B");
        if (((n >> 2) & 1) == 0) symbols = push_symbol(symbols, "C");

        if (verbose) print_symbols("Input symbols", symbols);

        symbols_t* res = inference_engine(kb, symbols);

        if (verbose) print_symbols_diff("Output symbols", res, symbols);

        bool d_implies_e = !symbols_contain(res, "D") || symbols_contain(res, "E");
        is_rule_coherent = is_rule_coherent && d_implies_e;
        assert(d_implies_e);

        free_symbols(res);
        free_symbols(symbols);
    }

    printf(GRAY("\n\u250f\u2509") BLUE(" (A || B) & C -> (A & C) || B?\n"));
    printf(GRAY("\u2517 ") "%s\n", is_rule_coherent ? "True" : "False");
    assert(is_rule_coherent);

    free_rule(rule_1);
    free_rule(rule_2);
    free_rule(rule_3);
    free_rule(rule_4);
    free_rule(rule_5);
    free_kb(kb);

    printf(GRAY("\n-> ") "Test 3: " GREEN("success!") "\n\n");
}

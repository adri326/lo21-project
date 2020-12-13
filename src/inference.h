#ifndef INFERENCE_H
#define INFERENCE_H

#include "rule.h"
#include "knowledge.h"

/// rule_t is just a linked list of symbols
typedef rule_t symbols_t;

/// Returns `needle ∈ hay`
bool symbols_contain(const symbols_t* hay, const char* needle);

/// Returns whether or not the condition of `rule` is true, based on `symbols`
bool is_condition_true(rule_t* rule, const symbols_t* symbols);

/// [3]: The inference engine; infers which symbols are true from a list of initially true symbols and a knowledge base
symbols_t* inference_engine(knowledgebase_t* base, const symbols_t* input_symbols);

/// Prints out a list of symbols
void print_symbols(const char* header, const symbols_t* symbols);

#endif // INFERENCE_H

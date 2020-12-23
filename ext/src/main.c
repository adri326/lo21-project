#include <stdlib.h>
#include <stdio.h>
#include <colors.h>
#include <string.h>
#include <rule.h>
#include <knowledge.h>
#include "parse.h"
#include "cmd.h"
#include "inference.h"

#define COMMAND_LEN 2048

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("Expected filename argument!\n");
        return 1;
    }

    FILE* input_file = fopen(argv[argc - 1], "r");
    if (!input_file) {
        printf("Couldn't open file: %s\n", argv[argc - 1]);
        return 1;
    }

    fseek(input_file, 0, SEEK_END);
    size_t length = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);
    char* raw = malloc(sizeof(char) * length);
    fread(raw, length - 1, 1, input_file);

    // printf("%s\n", raw);
    VEC(expr_ast)* ast = ast_parse(raw);
    free(raw);

    VEC(expr_flat)* flat = flatten_expressions(ast);
    // expr_flat_vec_printf(flat);

    knowledgebase_t* kb = simplify_expressions(flat);
    print_kb(kb);

    expr_flat_vec_free(flat);

    while (true) {
        printf(GRAY("> "));
        fflush(stdout);
        char command_raw[COMMAND_LEN];
        if (fgets(command_raw, COMMAND_LEN, stdin) == NULL) break;
        command cmd = parse_command(command_raw);

        if (strcmp(cmd.name, "eval") == 0) {
            if (parameter_vec_length(cmd.parameters) == 0) {
                printf("Invalid command: expected at least one parameter!\n");
            } else {
                for (size_t n = 0; n < parameter_vec_length(cmd.parameters); n++) {
                    parameter* p = parameter_vec_get(cmd.parameters, n);
                    symbols_t* symbols = NULL;
                    if (p->type == PARAM_INT) {
                        printf("Invalid parameter: expected single symbol or symbol list\n");
                        continue;
                    } else if (p->type == PARAM_SYMBOL) {
                        symbols = push_symbol(symbols, p->value.param_symbol);
                    } else if (p->type == PARAM_SYMBOLS) {
                        ccl_symbol_vec_printf(p->value.param_symbols);
                        for (size_t o = 0; o < ccl_symbol_vec_length(p->value.param_symbols); o++) {
                            symbols = push_symbol(symbols, ccl_symbol_vec_get(p->value.param_symbols, o)->symbol);
                            printf("%s\n", symbols->symbol);
                        }
                    }

                    print_symbols("Input", symbols);
                    symbols_t* res = inference_engine(kb, symbols);
                    print_symbols_diff("Output", res, symbols);

                    free_symbols(symbols);
                    free_symbols(res);
                }
            }
        }

        free_command(cmd);
    }

}

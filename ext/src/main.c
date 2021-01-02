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
    VEC(expr_ast)* ast = ast_expand_eqv(ast_parse(raw));
    free(raw);

    #ifdef GENERATE_ERRORS
        VEC(expr_flat)* flat = generate_errors(flatten_expressions(ast));
    #else // GENERATE_ERRORS
        VEC(expr_flat)* flat = flatten_expressions(ast);
    #endif // GENERATE_ERRORS

    knowledgebase_t* kb = simplify_expressions(flat);
    print_kb(kb);

    expr_flat_vec_free(flat);

    while (true) {
        printf(GRAY("> "));
        fflush(stdout);
        char command_raw[COMMAND_LEN];
        if (fgets(command_raw, COMMAND_LEN, stdin) == NULL) break;
        command cmd = parse_command(command_raw);

        handle_command(cmd, kb);

        free_command(cmd);
    }

    free_kb(kb);
}

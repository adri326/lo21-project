#include <stdlib.h>
#include <stdio.h>
#include <colors.h>
#include <rule.h>
#include <knowledge.h>
#include "parse.h"
#include "cmd.h"

#define COMMAND_LEN 2048

int main(int argc, char* argv[]) {
    printf(GREEN("Hello, world!\n"));

    char* raw = "(\"A\" && !\"C\") || \"E\" => \"B\" && !\"D\";";
    // char* raw = "\"A\" => \"B\";";
    printf("%s\n", raw);
    VEC(expr_ast)* ast = ast_parse(raw);

    VEC(expr_flat)* flat = flatten_expressions(ast);
    expr_flat_vec_printf(flat);

    knowledgebase_t* kb = simplify_expressions(flat);
    print_kb(kb);

    expr_flat_vec_free(flat);

    printf("> ");
    fflush(stdout);
    char command_raw[COMMAND_LEN];
    fgets(command_raw, COMMAND_LEN, stdin);

    command cmd = parse_command(command_raw);
    printf("%s: ", cmd.name);
    parameter_vec_printf(cmd.parameters);
}

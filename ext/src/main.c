#include <stdlib.h>
#include <stdio.h>
#include <colors.h>
#include <rule.h>
#include <knowledge.h>
#include "parse.h"

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
}

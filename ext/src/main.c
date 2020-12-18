#include <stdlib.h>
#include <stdio.h>
#include <colors.h>
#include <rule.h>
#include "parse.h"

int main(int argc, char* argv[]) {
    printf(GREEN("Hello, world!\n"));

    VEC(expr_ast)* ast = ast_parse("(\"A\" && !\"C\") || \"D\" || !\"E\" => \"B\" && !\"D\";");
    VEC(cond_and)* flattened_condition = flatten_subexpr(expr_ast_vec_get(ast, 0)->condition, false);
    cond_and_vec_printf(flattened_condition);
}

#include <stdlib.h>
#include <stdio.h>
#include <colors.h>
#include <rule.h>
#include "parse.h"

int main(int argc, char* argv[]) {
    printf(GREEN("Hello, world!\n"));

    ast_parse("!(\"A\" && !\"C\") || \"D\" || !\"E\" => \"B\" && !\"D\";");
}

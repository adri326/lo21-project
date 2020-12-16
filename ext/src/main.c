#include <stdlib.h>
#include <stdio.h>
#include <colors.h>
#include <rule.h>

int main(int argc, char* argv[]) {
    rule_t* rule = NULL;
    rule = push_symbol(rule, "This");
    rule = push_symbol_conclusion(rule, "works");
    print_rule(rule);

    printf(GREEN("Hello, world!\n"));

    free_rule(rule);
}

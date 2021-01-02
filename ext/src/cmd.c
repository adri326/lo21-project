#include "cmd.h"
#include <inference.h>
#include <string.h>
#include <inttypes.h>
#include <colors.h>

DECL_VEC_SOURCES(parameter);
DECL_VEC_SOURCES_PRINTF_CUSTOM(parameter, {
    if (value.type == PARAM_INT) {
        printf("%d", value.value.param_int);
    } else if (value.type == PARAM_SYMBOL) {
        printf("%s", value.value.param_symbol);
    } else if (value.type == PARAM_SYMBOLS) {
        ccl_symbol_vec_printf(value.value.param_symbols);
    }
})

command parse_command(const char* raw) {
    command res;
    size_t index = 0;
    for (; *raw != ' ' && *raw != '\n' && *raw; index++, raw++) {
        res.name[index] = *raw;
        if (index >= CMD_LEN - 1) {
            printf("Invalid command name: too long!\n");
            exit(3);
        }
    }
    res.name[index] = 0; // Should not be necessary, but we never know
    VEC(parameter)* parameters = parameter_vec_new(1);

    while (*raw && *raw != '\n') {
        while (*raw == ' ') raw++;
        if (*raw == '[') { // symbol array
            raw++;
            VEC(ccl_symbol)* symbols = ccl_symbol_vec_new(1);
            while (*raw != ']' && *raw) {
                while (*raw == ' ') raw++;
                if (*raw == '"') {
                    ccl_symbol symbol = {
                        .negate = false
                    };
                    size_t index = 0;
                    for (raw++; *raw != '"' && *raw != '\n' && *raw; index++, raw++) {
                        symbol.symbol[index] = *raw;
                        if (index >= SYMBOL_LEN - 1) {
                            printf("Invalid symbol name: too long!\n");
                            exit(3);
                        }
                    }
                    ccl_symbol_vec_push(symbols, symbol);
                }
                raw++;
            }
            parameter_vec_push(parameters, (parameter){
                .type = PARAM_SYMBOLS,
                .value.param_symbols = symbols
            });
        } else if (*raw == '"') { // symbol
            size_t index = 0;
            parameter param = {
                .type = PARAM_SYMBOL
            };
            for (raw++; *raw != '"' && *raw != '\n' && *raw; index++, raw++) {
                param.value.param_symbol[index] = *raw;
                if (index >= SYMBOL_LEN - 1) {
                    printf("Invalid symbol name: too long!\n");
                    exit(3);
                }
            }
            param.value.param_symbol[index] = 0;
            parameter_vec_push(parameters, param);
        } else if (*raw >= '0' && *raw <= '9') { // number
            char number[16];
            size_t index = 0;
            for (; *raw != ' ' && *raw; index++, raw++) {
                number[index] = *raw;
            }
            number[index] = 0;
            parameter_vec_push(parameters, (parameter){
                .type = PARAM_INT,
                .value.param_int = (int)strtol(number, NULL, 10)
            });
        }
        raw++;
    }

    res.parameters = parameters;

    return res;
}

void free_command(command cmd) {
    for (size_t n = 0; n < parameter_vec_length(cmd.parameters); n++) {
        parameter* p = parameter_vec_get(cmd.parameters, n);
        if (p->type == PARAM_SYMBOLS) {
            ccl_symbol_vec_free(p->value.param_symbols);
        }
    }
    parameter_vec_free(cmd.parameters);
}

void handle_command(command cmd, knowledgebase_t* kb) {
    if (strcmp(cmd.name, "eval") == 0) { // `eval` command
        cmd_eval(cmd, kb);
    } else if (strcmp(cmd.name, "forall") == 0) {
        cmd_forall(cmd, kb);
    } else if (strcmp(cmd.name, "table") == 0) {
        cmd_table(cmd, kb);
    }
}

void cmd_eval(command cmd, knowledgebase_t* kb) {
    if (parameter_vec_length(cmd.parameters) == 0) {
        printf("Invalid command: expected at least one parameter!\n");
        return;
    }
    for (size_t n = 0; n < parameter_vec_length(cmd.parameters); n++) {
        parameter* p = parameter_vec_get(cmd.parameters, n);
        symbols_t* symbols = NULL;
        if (p->type == PARAM_INT) {
            printf("Invalid parameter: expected single symbol or symbol list\n");
            continue;
        } else if (p->type == PARAM_SYMBOL) {
            symbols = push_symbol(symbols, p->value.param_symbol);
        } else if (p->type == PARAM_SYMBOLS) {
            for (size_t o = 0; o < ccl_symbol_vec_length(p->value.param_symbols); o++) {
                symbols = push_symbol(symbols, ccl_symbol_vec_get(p->value.param_symbols, o)->symbol);
            }
        }

        print_symbols("Input", symbols);
        symbols_t* res = inference_engine(kb, symbols);
        print_symbols_diff("Output", res, symbols);
        printf("\n");

        free_symbols(symbols);
        free_symbols(res);
    }
}

void cmd_forall(command cmd, knowledgebase_t* kb) {
    if (parameter_vec_length(cmd.parameters) != 2 && parameter_vec_length(cmd.parameters) != 3) {
        printf("Invalid command: expected two or three parameters!\n");
        return;
    }

    VEC(ccl_symbol)* variable_symbols;
    parameter* variable_symbols_param = parameter_vec_get(cmd.parameters, 0);
    parameter* static_symbols_param = parameter_vec_get(cmd.parameters, 1);
    parameter* required_symbol_param = parameter_vec_get(cmd.parameters, parameter_vec_length(cmd.parameters) - 1);

    if (required_symbol_param->type != PARAM_SYMBOL) {
        printf("Invalid parameter: expected single symbol for the required symbol\n");
        return;
    }

    if (variable_symbols_param->type != PARAM_SYMBOLS) {
        printf("Invalid parameter: expected symbol list for the variable symbols\n");
        return;
    }

    variable_symbols = variable_symbols_param->value.param_symbols;
    if (ccl_symbol_vec_length(variable_symbols) == 0) {
        printf("Expected at least one variable symbol!\n");
        return;
    }

    VEC(ccl_symbol)* static_symbols;
    if (parameter_vec_length(cmd.parameters) == 3) {
        if (static_symbols_param->type != PARAM_SYMBOLS) {
            printf("Invalid parameter: expected symbol list for the static symbols\n");
            return;
        }
        static_symbols = static_symbols_param->value.param_symbols;
    }

    bool res = true;
    bool err = false;
    for (uintmax_t perm = 0; perm < (1 << ccl_symbol_vec_length(variable_symbols)); perm++) {
        symbols_t* input = NULL;
        if (parameter_vec_length(cmd.parameters) == 3) {
            for (size_t n = 0; n < ccl_symbol_vec_length(static_symbols); n++) {
                input = push_symbol(input, ccl_symbol_vec_get(static_symbols, n)->symbol);
            }
        }

        for (size_t n = 0; n < ccl_symbol_vec_length(variable_symbols); n++) {
            if ((perm & (1 << n)) > 0) {
                char symbol[SYMBOL_LEN];
                if (strlen(ccl_symbol_vec_get(variable_symbols, n)->symbol) >= SYMBOL_LEN - 1) {
                    printf("Too long symbol!");
                    return;
                }
                strcpy(symbol + 1, ccl_symbol_vec_get(variable_symbols, n)->symbol);
                *symbol = '!';
                input = push_symbol(input, symbol);
            } else {
                input = push_symbol(input, ccl_symbol_vec_get(variable_symbols, n)->symbol);
            }
        }

        symbols_t* output = inference_engine(kb, input);
        bool is_in = symbol_in(required_symbol_param->value.param_symbol, output);

        if (symbol_in("error", output)) {
            is_in = false;
            err = true;
        }

        if (!is_in) {
            print_symbols("Input", input);
            print_symbols_diff("Output", output, input);
            printf("\n");
        }

        res = res && is_in;
    }

    printf(
        GRAY("∴ ") CYAN("∀") GRAY(" -> ") "%s\n",
        err ? RED("error") : (
            res ? GREEN("true") : RED("false")
        )
    );
}

void cmd_table(command cmd, knowledgebase_t* kb) {
    #ifdef PRINT_ERRORS
    printf(GRAY("Warning: the PRINT_ERRORS compile-time option was enabled; the output will not look great if an error is encountered!\n"));
    #endif // PRINT_ERRORS
    if (parameter_vec_length(cmd.parameters) != 2 && parameter_vec_length(cmd.parameters) != 3) {
        printf("Invalid command: expected two or three parameters!\n");
        return;
    }

    VEC(ccl_symbol)* variable_symbols;
    parameter* variable_symbols_param = parameter_vec_get(cmd.parameters, 0);
    parameter* static_symbols_param = parameter_vec_get(cmd.parameters, 1);
    parameter* required_symbol_param = parameter_vec_get(cmd.parameters, parameter_vec_length(cmd.parameters) - 1);

    if (required_symbol_param->type != PARAM_SYMBOL) {
        printf("Invalid parameter: expected single symbol for the required symbol\n");
        return;
    }

    if (variable_symbols_param->type != PARAM_SYMBOLS) {
        printf("Invalid parameter: expected symbol list for the variable symbols\n");
        return;
    }

    variable_symbols = variable_symbols_param->value.param_symbols;
    if (ccl_symbol_vec_length(variable_symbols) == 0) {
        printf("Expected at least one variable symbol!\n");
        return;
    }

    VEC(ccl_symbol)* static_symbols;
    if (parameter_vec_length(cmd.parameters) == 3) {
        if (static_symbols_param->type != PARAM_SYMBOLS) {
            printf("Invalid parameter: expected symbol list for the static symbols\n");
            return;
        }
        static_symbols = static_symbols_param->value.param_symbols;
    }

    char required_symbol_false[SYMBOL_LEN];
    char required_symbol_true[SYMBOL_LEN];

    if (strlen(required_symbol_param->value.param_symbol) >= SYMBOL_LEN - 1) {
        printf("Too long symbol!");
        return;
    }
    strcpy(required_symbol_false + 1, required_symbol_param->value.param_symbol);
    strcpy(required_symbol_true, required_symbol_param->value.param_symbol);
    *required_symbol_false = '!';

    // Print header: v--v--v
    for (size_t n = 0; n < ccl_symbol_vec_length(variable_symbols) + 1; n++) {
        if (n == ccl_symbol_vec_length(variable_symbols)) {
            printf(GRAY("\u2532"));
            for (size_t o = 0; o < TABLE_SYMBOL_LEN + 2; o++) printf(GRAY("\u2501"));
        } else {
            if (n == 0) printf(GRAY("\u250c"));
            else printf(GRAY("\u252c"));
            for (size_t o = 0; o < TABLE_SYMBOL_LEN + 2; o++) printf(GRAY("\u2500"));
        }
    }
    printf(GRAY("\u2513") "\n");

    // Print header: a | b | c

    for (size_t n = 0; n < ccl_symbol_vec_length(variable_symbols); n++) {
        printf(GRAY("\u2502 ") "%*.*s ", TABLE_SYMBOL_LEN, TABLE_SYMBOL_LEN, ccl_symbol_vec_get(variable_symbols, n)->symbol);
    }
    printf(GRAY("\u2503 ") CYAN("%*.*s ") GRAY("\u2503") "\n", TABLE_SYMBOL_LEN, TABLE_SYMBOL_LEN, required_symbol_param->value.param_symbol);

    // Main loop:

    for (uintmax_t perm = 0; perm < (1 << ccl_symbol_vec_length(variable_symbols)); perm++) {
        symbols_t* input = NULL;
        if (parameter_vec_length(cmd.parameters) == 3) {
            for (size_t n = 0; n < ccl_symbol_vec_length(static_symbols); n++) {
                input = push_symbol(input, ccl_symbol_vec_get(static_symbols, n)->symbol);
            }
        }

        // Print row separator: +--+--+
        for (size_t n = 0; n < ccl_symbol_vec_length(variable_symbols) + 1; n++) {
            if (n == ccl_symbol_vec_length(variable_symbols)) {
                printf(GRAY("\u254a"));
                for (size_t o = 0; o < TABLE_SYMBOL_LEN + 2; o++) printf(GRAY("\u2501"));
            } else {
                if (n == 0) printf(GRAY("\u251c"));
                else printf(GRAY("\u253c"));
                for (size_t o = 0; o < TABLE_SYMBOL_LEN + 2; o++) printf(GRAY("\u2500"));
            }
        }
        printf(GRAY("\u252b") "\n");

        // Add the variable symbols and print their value in the table
        for (size_t n = 0; n < ccl_symbol_vec_length(variable_symbols); n++) {
            printf(GRAY("\u2502 "));
            if ((perm & (1 << n)) > 0) {
                char symbol[SYMBOL_LEN];
                if (strlen(ccl_symbol_vec_get(variable_symbols, n)->symbol) >= SYMBOL_LEN - 1) {
                    printf("Too long symbol!");
                    return;
                }
                strcpy(symbol + 1, ccl_symbol_vec_get(variable_symbols, n)->symbol);
                *symbol = '!';
                input = push_symbol(input, symbol);
                printf(MAGENTA("%*.*s "), TABLE_SYMBOL_LEN, TABLE_SYMBOL_LEN, "false");
            } else {
                input = push_symbol(input, ccl_symbol_vec_get(variable_symbols, n)->symbol);
                printf(GREEN("%*.*s "), TABLE_SYMBOL_LEN, TABLE_SYMBOL_LEN, "true");
            }
        }

        symbols_t* output = inference_engine(kb, input);
        bool is_true = symbol_in(required_symbol_true, output);
        bool is_false = symbol_in(required_symbol_false, output);
        bool is_error = symbol_in("error", output);

        printf(GRAY("\u2503 "));
        if (is_error) {
            printf(RED("%*.*s "), TABLE_SYMBOL_LEN, TABLE_SYMBOL_LEN, "error");
        } else if (is_true) {
            printf(GREEN("%*.*s "), TABLE_SYMBOL_LEN, TABLE_SYMBOL_LEN, "true");
        } else if (is_false) {
            printf(MAGENTA("%*.*s "), TABLE_SYMBOL_LEN, TABLE_SYMBOL_LEN, "false");
        } else {
            printf(GRAY("%*.*s "), TABLE_SYMBOL_LEN, TABLE_SYMBOL_LEN, "N/A");
        }
        printf(GRAY("\u2503") "\n");
        // Print the symbol
    }

    // Print footer: ^--^--^
    for (size_t n = 0; n < ccl_symbol_vec_length(variable_symbols) + 1; n++) {
        if (n == ccl_symbol_vec_length(variable_symbols)) {
            printf(GRAY("\u253a"));
            for (size_t o = 0; o < TABLE_SYMBOL_LEN + 2; o++) printf(GRAY("\u2501"));
        } else {
            if (n == 0) printf(GRAY("\u2514"));
            else printf(GRAY("\u2534"));
            for (size_t o = 0; o < TABLE_SYMBOL_LEN + 2; o++) printf(GRAY("\u2500"));
        }
    }
    printf(GRAY("\u251b") "\n");
}

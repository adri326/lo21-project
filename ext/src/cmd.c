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
    if (strcmp(cmd.name, "eval") == 0) {
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

            free_symbols(symbols);
            free_symbols(res);
        }
    } else if (strcmp(cmd.name, "forall") == 0) {
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

            if (!is_in) {
                print_symbols("Input", input);
                print_symbols_diff("Output", output, input);
            }

            res = res && is_in;
        }
        printf(GRAY("∴ ") CYAN("∀") GRAY(" -> ") "%s\n", res ? GREEN("true") : RED("false"));
    }
}

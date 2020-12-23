#include "cmd.h"

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

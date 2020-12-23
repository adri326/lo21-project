#ifndef CMD_H
#define CMD_H

#include <vec.h>
#include <knowledge.h>
#include "parse.h"
#define CMD_LEN 16

enum parameter_type {
    PARAM_INT,
    PARAM_SYMBOL,
    PARAM_SYMBOLS
};

union parameter_value {
    int param_int;
    char param_symbol[SYMBOL_LEN];
    VEC(ccl_symbol)* param_symbols;
};

struct parameter {
    enum parameter_type type;
    union parameter_value value;
};
typedef struct parameter parameter;
DECL_VEC(parameter);

struct command {
    char name[CMD_LEN];
    VEC(parameter)* parameters;
};
typedef struct command command;

/// Parses commands

command parse_command(const char* raw);
void free_command(command cmd);
void handle_command(command cmd, knowledgebase_t* kb);

#endif // CMD_H

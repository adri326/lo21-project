#ifndef CMD_H
#define CMD_H

#include <vec.h>
#include <knowledge.h>
#include "parse.h"
#define CMD_LEN 16
#define TABLE_SYMBOL_LEN 5

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

/** Parses a simple command, see the format in (the README)[../README.md].
    @param raw - The raw string to parse
    @returns The parsed command, UB on error
**/
command parse_command(const char* raw);

/** Frees the parameter vector within `cmd`.
    @param cmd - The command to free
**/
void free_command(command cmd);

/** Handles a command, given a knowledgebase. If the command is unknown, then nothing happens
    @param cmd - The command entered by the user
    @param kb - The knowledgebase to potentially run the inference engine on
**/
void handle_command(command cmd, knowledgebase_t* kb);

/** The "eval" command, evaluates the knowledgebase for each of the parameters.

    ## Format:

    ```
    eval <param1> [<param2> [...]]
    ```

    ## What it does:

    ```
    for (n from 1 to the number of commands), do
        evaluate the knowledgebase with as input (param<n>)
    done
    ```
**/
void cmd_eval(command cmd, knowledgebase_t* kb);

/** The "forall" command, asserts that a required symbol is true for any input combination.

    ## Format:

    ```
    forall <variables> [<static symbols>] <required symbol>
    ```

    ## Example:

    This would execute every combination of `A/!A`, `B/!B`, `C/!C` and check that `res` is true for all of them:
    ```
    forall ["A" "B" "C"] "res"
    ```

    This would execute every combination of `A/!A`, `B/!B`, put the `always_true` symbol and check that `res` is true for all of these:
    ```
    forall ["A" "B"] ["always_true"] "res"
    ```
**/
void cmd_forall(command cmd, knowledgebase_t* kb);

/** The "table" command, prints out a truth table with every input combination.

    ## Format:

    The format is the same as in `forall`.

    ## Example:

    On `test/test-4.kb`, the following command would produce:
    ```
    table ["A" "B"] "res"
    ┌───────┬───────┲━━━━━━━┓
    │     A │     B ┃   res ┃
    ├───────┼───────╊━━━━━━━┫
    │  true │  true ┃  true ┃
    ├───────┼───────╊━━━━━━━┫
    │ false │  true ┃  true ┃
    ├───────┼───────╊━━━━━━━┫
    │  true │ false ┃  true ┃
    ├───────┼───────╊━━━━━━━┫
    │ false │ false ┃  true ┃
    └───────┴───────┺━━━━━━━┛
    ```

    ## Notes

    If an error is encountered and the project was built with the PRINT_ERRORS flag, then the error will be printed amidst printing the table, messing up the formatting.

**/
void cmd_table(command cmd, knowledgebase_t* kb);

/** The "print" command, prints out the knowledgebase. **/
void cmd_print(command cmd, knowledgebase_t* kb);

/** The "help" command, prints out a help message and also lets you query the usage of another command. **/
void cmd_help(command cmd, knowledgebase_t* kb);

#endif // CMD_H

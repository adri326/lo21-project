# lo21-project-ext

This is an extension upon the base implementation, which introduces a dedicated language and a CLI interface.

## Building and running

After having followed [the steps to install the base project](../README.md#Installation), go to this directory and build it:

```sh
git submodule update --init --recursive # The extension relies on https://github.com/adri326/lists.c
cd ext
mkdir build
cd build
cmake .. && make
```

You can try the engine out by running it on the included datasets.
Run `./lo21-project-ext ../test/test-1.kb`; you will be prompted for a command.
Typing in `eval "A"` will make the engine run on the knowledgebase with as input symbol `A`.
You should then see the output being `A`, `B` and `C`.

## Language

The included language, `kb`, is a little language that is used to express boolean logic over an arbitrary set of symbols.

### Symbols

Symbols are enclosed with quotation marks (`"`), and may only contain letters, numbers and underscores.
For instance:

- `"hello"` is a valid symbol
- `"3_cats"` is a valid symbol
- `"3 cats"` is an **invalid** symbol

Symbols can be negated by preceding them with an exclamation mark (`!`):

- `!"hello"` is valid and means "not `hello`"
- `"!hello"` is **invalid**

### Sub-expressions

Symbols can be linked together using three operators: `&&` (and), `||` (or) and `!` (not).
There is no operator precedence and you must use parentheses if you want to combine different operators together.
For instance:

- `"A" && "B"` is valid and means "`A` and `B`"
- `"A" || "B"` is valid and means "`A` or `B`"
- `"A" || "B" || "C"` is valid and means "`A` or `B` or `C`"
- `!("A" && "B")` is valid and means "not (`A` and `B`)"
- `"A" && "B" || "C"` is **invalid**
- `("A" && "B") || "C"` is valid and means "(`A` and `B`) or `C`"

See [the grammar](./grammar.ebnf) for a detailed description of what is valid and what isn't valid.

### Expressions

Expressions are used to express the relationship between symbols. They are made up of a sub-expression (aka. "condition"), an relationship operator and a set of conclusion symbols.

The conclusion symbols are bundled together by infixing the `&&` operator between them.
The relationship operator may be either `=>` or `<=>`. If it is `<=>`, then there may only be one conclusion symbol.
If it is `=>`, then the conclusion symbols may be replaced with `error` and an error will be triggered if the condition yields true.

Here are some examples of expressions:

- `"A" => "B"` is valid and means "if `A`, then `B`"
- `"A" && "B" => "C"` is valid and means "if `A` and `B`, then `C`"
- `"C" || "D" => "E" && "F"` is valid and means "if `C` or `D`, then `E` and `F`"
- `"A" <=> "E"` is valid and means "if and only if `A`, then `E`". It is internally expanded as `"A" => "E"` and `!("A") => !"E"`; if opposites are enabled, then the engine will also infer that `"E" => "A"` and `!"E" => "A"`
- `"B" => "C" || "D"` is **invalid**
- `"A" <=> error` is **invalid**
- `"A" <=> "B" && "C"` is **invalid**

An expression must be terminated by a semicolon.
You may put several expressions in a file; for instance:

```kb
"A" => "B";
"B" => "C";
```

### Comments

Comments are enclosed within brackets (`{}`). You may not stack brackets (`{{A}}` results in a syntax error).

## Commands

The CLI interface expects you to type a valid command in and executes it.
A command's syntax is as follows:

```
<command_name> [param1 [param2 [...]]]
```

Where `paramN` may be:

- a number (`3`, `14`, ...)
- a symbol (`"A"`, `"res"`, ...)
- a list of symbols (`["A" "B" "C"]`, `[]`, ...)

The available commands are:

- `eval <symbol(s)> [<symbol(s)> [...]]`, for each parameter, evaluates the knowledgebase on it
- `forall <input_symbols> <output_symbol>`, takes each symbol and evaluates the knowledgebase on combinations of these symbols and their opposite (eg. `[A, B]`; `[!A, B]`; `[A, !B]`; `[!A, !B]`).
Returns true iff `output_symbol` is true in every case, returns `error` if an error occured in one of the cases

### Syntax highlight

Syntax highlight of this language is supported on VSCode and Atom (as long as they support legacy TextMate). You can find it [here](https://github.com/adri326/language-kb).

## Examples

A few examples can be found in the `/ext/test/` directory; they each include a description and a sample command to try them on.

## Build options

The following build options are available; you can turn them on/off by giving as argument to cmake `-DOPTION_NAME:BOOL=ON`, where `ON` can also be set to `OFF` and where `OPTION_NAME` can be any of the following:

- `FAITHFUL_IMPLEMENTATION`; set to OFF for a faster implementation of the required functions (the performance improvements should be minor)
- `GENERATE_OPPOSITE`; set to ON (default) to enable the generation of opposite rule (for instance, `"A" => "B"` will automatically generate `!"B" => !"A"`)
- `NO_COLOR`; set to ON (default on windows) to disable the colors, OFF by default on non-windows platforms
- `PRINT_ERRORS`; if ON (default), inference engine will print an error message whenever an `error` is triggered, useful for debugging
- `GENERATE_ERRORS`; if ON (default), then errors will be generated for the symbols in the input file (eg. `"A" && !"A" => error`)

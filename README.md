# lo21-project

Project for the LO21 class "Algorithmics and programming II".
You can have a read at a transcript of the subject here (FR): https://github.com/Juknum/Systeme-Expert/blob/master/README.md

## Installation

Clone this repository:

```sh
git clone https://github.com/adri326/lo21-project
cd lo21-project
```

Then build the project:

```sh
mkdir build && cd build
cmake ..
make
```

## Report

The report (written in french) of this project can be found in `.tex` format at [report_fr.tex](./report_fr.tex) and its compiled version in `.pdf` format at [report_fr.pdf](./report_fr.pdf).

The presentation slides are available [here (FR)](https://docs.google.com/presentation/d/14uA0y8mrEptq2UXIL1WiLnINxllMmEdR23VbAO0nL_Y/edit?usp=sharing).

## Notes on extensions

By default, this project builds as a faithful implementation of what is described in the report.
Some functions, however, can be rewritten in a more optimized way in C.
To enable such rewrite, call `cmake` with the following argument:

```sh
cmake -DFAITHFUL_IMPLEMENTATION:BOOL=OFF ..
```

You can also disable colors by passing the `-DNO_COLOR:BOOL=ON` argument. That argument is automatically set to true on windows.
You might also need to run the command `chcp 65001` on the windows DOS-like prompt before running this application to enable unicode support.

## Included tests

Sample datasets are included with this project and their source code can be found in `src/test.c`.

## Extension

This project includes an extension over the basic requirements of it, which can be found in the `ext/` directory. Check out its documentation [here](ext/README.md)!

# lo21-project

Project for the LO21 class "Algorithmics and programming II"

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

<!--
## Notes on extensions

By default, this project builds as a faithful implementation of what is described in the report.
Some functions, however, can be rewritten in a more optimized way in C.
To enable such rewrite, call `cmake` with the following argument:

```sh
cmake -DFAITHFUL_IMPLEMENTATION:BOOL=OFF ..
``` -->

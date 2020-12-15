#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include "test.c"

int main(int argc, char* argv[]) {
    bool verbose = false;
    for (size_t n = 1; n < argc; n++) {
        if (strcmp(argv[n], "-v") == 0) {
            verbose = true;
        }
    }

    test_1(verbose);
    test_2(verbose);
    test_3(verbose);
}

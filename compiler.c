#include "compiler.h"
#include "rlib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void rrex_compiler_repl() {
    rclear();
    printf("Type expression to convert bytecode to human readable format.\n");
    while (true) {
        rprintb("> ");
        char line[8096];
        rreadline(line, sizeof(line), true);
        if (!line)
            continue;
        char bdata[sizeof(line) * 2];
        rprint("\\t");
        rrex_compile(line, bdata);
        rprinty("< ");
        print_bc(bdata);
        rprint("\n");
    }
}

int main() {
    rrex_compiler_tests();
    printf("%s\n", "Executed all compiler tests at boot of this application.");
    rrex_compiler_repl();
    return 0;
}
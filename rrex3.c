#define RREX3_DEBUG 0
#include "rrex3.h"
#include "rlib.h"

#include <regex.h>

void benchmark(int times, char *str, char *expr) {

    regmatch_t matches[10];
    printf("Matching \"%s\" with \"%s\".\n", str, expr);
    regex_t regex;
    if (regcomp(&regex, expr, REG_EXTENDED)) {
        printf("Creg: error in regular expression.\n");
        exit(1);
    }
    printf("creg: ");
    RBENCH(times, {
        if (regexec(&regex, str, 0, matches, 0)) {
            printf("Creg: error executing regular expression.\n");
        }
    })
    regfree(&regex);
    ;
    rrex3_t *rrex = rrex3_compile(NULL, expr);
    printf("rrex3 (%s): ", rrex->compiled);
    RBENCH(times, {
        if (rrex3(rrex, str, NULL)) {

        } else {
            printf("Rrex3: error\n");
            exit(0);
        }
    });
    rrex3_free(rrex);
    printf("\n");
}

int main() {
    rrex3_test();
    int times = 1;
    benchmark(times, "\"stdio.h\"\"string.h\"\"sys/time.h\"",
              "\".*\"\".*\"\".*\"");

    benchmark(times, "abcdefghijklmnopqrstuvwxyz",
              "abcdefghijklmnopqrstuvwxyz$");
    benchmark(times, "aaaaaaaaaaaaaaaaaaaaaaaaaa",
              "aaaaaaaaaaaaaaaaaaaaaaaaaa$");
    benchmark(times, "abcdefghijklmnopqrstuvwxyz",
              "..........................$");

    // [abcm] failed
    benchmark(times, "abcdefghijklmnopqrstuvwxyz", ".*z");
    benchmark(times, "abcde", ".*e");
    benchmark(times, "abcdef", ".*f");

    benchmark(times, "abcdefghijklmnopqrstuvwxyz",
              "[a]b*c+d\\w[f-g][g][h-i][i][^a][abcdefgk][l][m][n][o][p][a-z][r]"
              "[s][t][u][v][w].*z$");
    benchmark(times, "zzz",
              "[abcdefghijklmnopqrstuvwxyz][abcdefghijklmnopqrstuvwxyz]["
              "abcdefghijklmnopqrstuvwxyz]$");

    benchmark(times, "7245 Sr", "[0-9][0-9][0-9][0-9] ?\\w\\w$");

    benchmark(times,
              "abcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmn"
              "opqrstuvwxyzesting",
              "[z-z][e-e]");
    benchmark(times,
              "abcdefghijklmnopqrstuvwxyabcdefghijklmnopqrstuvwxyabcdefghijklmn"
              "opqrstuvwxyzesting",
              "zesting");
    benchmark(times, "\"stdio.h\"\"string.h\"\"sys/time.h\"",
              "\"(.*)\"\"(.*)\"\"(.*)\"");
    benchmark(times, "          \"stdio.h\"\"string.h\"\"sys/time.h\"",
              "\".+\"\".+\"\".+\"");
    benchmark(times, "          \"stdio.h\"\"string.h\"\"sys/time.h\"",
              "\"(.+)\"\"(.+)\"\"(.+)\"");
}
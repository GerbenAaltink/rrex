#include "rrex2.h"
#include <math.h>
#include <regex.h>

void cregex_repeat(char *s, char *r) {
    // Get object from shared data object. rbf is session variable of current
    // bench function.
    regex_t *regex = (regex_t *)rbf->data;
    // Only get's executed at beginning of the benchmark. Executed once.
    if (rbf->first) {
        // Set session data
        regex = (regex_t *)malloc(sizeof(regex_t));
        rbf->data = regex;
        regcomp(regex, r, REG_EXTENDED);
    }
    // The code to benchmark
    rassert(!regexec(regex, s, 0, NULL, 0))
        // Is executed only once at end of benchmark
        if (rbf->last) regfree(regex);
}
void rrex_repeat(char *s, char *r) {
    char *bdata = (char *)rbf->data;

    if (rbf->first) {
        bdata = (char *)malloc(4096);
        rrex_compile(r, bdata);
        rbf->data = bdata;
    }

    rassert(rrex_match(s, bdata));

    if (rbf->last) {
        free(rbf->data);
    }
}

int wins_rrex = 0;
int loss_rrex = 0;
nsecs_t total_execution_time = 0;
long total_times = 0;

bool validate_dutch_zipcode_c(char *code) {
    if (strlen(code) != 7)
        return false;
    for (int i = 0; i < 4; i++) {
        if (!isdigit(code[i]))
            return false;
    }
    if (!iswhitespace(code[4])) {
        return false;
    }
    for (int i = 6; i < 7; i++) {
        if (!isalpha(code[i]))
            return false;
    }
    return true;
}
bool validate_dutch_zipcode_c_literal(char *code) {
    if (strlen(code) != 7)
        return false;
    return isdigit(code[0]) && isdigit(code[1]) && isdigit(code[2]) &&
           isdigit(code[3]) && iswhitespace(code[4]) && isalpha(code[5]) &&
           isalpha(code[6]);
}

void validate_dutch_zipcode_creg(char *s) {
    regex_t regex;
    char *pattern = "\\d{4} [a-zA-Z]{2}";
    int ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret) {
        printf("cregex comp error\b");
        exit(0);
    }
    ret = regexec(&regex, s, 0, NULL, 0);
    if (!ret) {
        printf("cregex exec error\b");
        exit(0);
    }
    regfree(&regex);
}

void validate_dutch_zipcode_creg_precompiled(char *s) {
    regex_t *regex = (regex_t *)rbf->data;
    // Only get's executed at beginning of the benchmark. Executed once.
    if (rbf->first) {
        // Set session data
        regex = (regex_t *)malloc(sizeof(regex_t));
        rbf->data = regex;
        char *pattern = "\\d{4} [a-zA-Z]{2}";
        regcomp(regex, pattern, REG_EXTENDED);
    }
    // The code to benchmark
    int ret = regexec(regex, s, 0, NULL, 0);
    if (!ret) {
        printf("cregex exec error\b");
        exit(0);
    }
    // Is executed only once at end of benchmark
    if (rbf->last)
        regfree(regex);
}
void validate_dutch_zipcode_rrex_precompiled(char *s) {
    char *bcode = (char *)rbf->data;
    if (rbf->first) {
        bcode = (char *)malloc(20);
        rrex_compile("\\d{4} [a-zA-Z]{2}", bcode);
        rbf->data = bcode;
    }
    rrex_match(s, bcode);
    if (rbf->last) {
        free(bcode);
    }
}

bool validate_dutch_zipcode_rrex(char *s) {
    return rrex(s, "\\d{4} [a-zA-Z]{2}");
}

void benchmark_dutch_zipcode(long times, char *s) {
    rbench_t *r = rbench_new();
    r->show_progress = false;
    r->stdout = false;
    r->add_function(r, "rrex", "zipcode", (void *)validate_dutch_zipcode_rrex);
    r->add_function(r, "rrex compiled", "zipcode",
                    (void *)validate_dutch_zipcode_rrex_precompiled);
    r->add_function(r, "creg", "zipcode", (void *)validate_dutch_zipcode_creg);
    r->add_function(r, "creg compiled", "zipcode",
                    (void *)validate_dutch_zipcode_creg_precompiled);
    r->add_function(r, "native c", "zipcode c",
                    (void *)validate_dutch_zipcode_c);
    r->add_function(r, "native c literal", "zipcode c",
                    (void *)validate_dutch_zipcode_c_literal);
    printf("Benchmarking validation of %s with rrex and native c code.\n", s);
    r->execute1(r, times, s);
    rbench_free(r);
}

void benchmark(long times, char *s, char *e) {
    rprint("Benchmark \\l string:<%s> expr:<%s>\t\n", s, e);
    rbench_t *r;
    r = rbench_new();
    r->show_progress = false;
    r->stdout = false;
    r->add_function(r, "executor", "rrex", (void *)rrex_repeat);
    r->add_function(r, "executor", "clib", (void *)cregex_repeat);
    if (r->execute2(r, times, s, e)->winner == 1) {
        wins_rrex++;
    } else {
        loss_rrex++;
    }
    total_execution_time += r->execution_time;
    total_times += times * 2;
    rbench_free(r);
}

void rrex_benchmark_tests(long times) {
    benchmark_dutch_zipcode(times / 10, "7245 SR");
    benchmark_dutch_zipcode(times / 10, "A245 SR");
    benchmark_dutch_zipcode(times / 10, "7245 S3");
    benchmark(times, "abababc", "^(ab)+c$");
    // c regex does not support:
    // benchmark(times,"123a33","\\d+a\\d+$");
    benchmark(times, "9-3", "([3-9]-[3-9])");
    benchmark(times, "1234A", "[1-4]{4}A");
    benchmark(times, "abcdef", "abcd?ef");
    benchmark(times, "ce", "(a|b|c|d)e");
    benchmark(times, "a", "(a)");
    benchmark(times, "aa", "(a){2}");
    // benchmark(times, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaq", "[^xyzv]+q$");
    benchmark(times, "abcabcabcabcabcabc", "[acb][acb]{4}");
    benchmark(times, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
              "[1A-Z0-9a12345]{33}");
    benchmark(times, "abcd", "abcd");
    benchmark(times, "aaaaaaaaa", "a{9}");
    benchmark(times, "a", "[abc]");
    benchmark(times, "aa", "[abc]{2}");
    benchmark(times, "ab", "[abc]{2}");
    benchmark(times, "ac", "[abc]{2}");
    benchmark(times, "c", "[abc]");
    benchmark(times, "123", "[0-9][0-9][0-9]");
    benchmark(times, "ab*", "[a-z]b.");
    benchmark(times, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", ".{33}");
    // benchmark(times, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaq", "[dbac]+q$");
    benchmark(times, "#include \"test.h\"", "^#include *\"[a-z\\.]+\"$");
    benchmark(times, "abcdefgh", "^.*gh$");
    benchmark(times, "randomtextbeforeabcdefgh", "^random.*gh$");
    benchmark(times, "abcdefg", "a?bcdf?ef?g");
    printf("Times won: %d / %d\n", wins_rrex, wins_rrex + loss_rrex);
    printf("Total execution time: %s\n", format_time(total_execution_time));
    printf("Total times: %s\n", rformat_number(total_times));
}

void repl() {

    while (true) {
        char s[4096];
        char e[4096];
        rprint("%s", "Write a string to parse:\n");
        rreadline(s, 1024, true);
        rprint("Write a reqular expression:\n");
        rreadline(e, 1024, true);
        rprint("\\t");
        bool valid = rrex(s, e);
        if (valid) {
            rprintgf(stdout, "\\T %s", valid ? "valid\n" : "invalid\n");
        } else {
            rprintrf(stdout, "\\T %s", valid ? "valid\n" : "invalid\n");
        }
    }
}

void rrex_tests() {
    rrex_functions_test();
    rrex_compiler_tests();
    rrex_executor_tests();
    __attribute__((unused)) int res = rtest_end("");
    rprintg("Tests passed.\n\n");
    sleep(1);
}

int main(int argc, char *argv[]) {
    /*
       20000000 for 140s (1 billion times)
       16000000 for 100s
       32000000 for 200s
       4000000 for 30s
       2000000 for 15s -> this is minimum to get consistent result
       1000000 for 7.5s
   */
    long times = 2000000;
    if (argc > 1) {
        if (!strcmp(argv[1], "cli")) {
            repl();
            return 0;
        } else if (!strcmp(argv[1], "test")) {
            times = 20;
        }
    }
    rrex_tests();
    rrex_benchmark_tests(times);
    return 0;
}
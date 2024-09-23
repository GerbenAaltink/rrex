#define R4_DEBUG

#include "rrex4.h"
#include <regex.h>
#include <rlib.h>
#include <search.h>

bool bench_r4(unsigned int times, char *str, char *expr) {
    RBENCH(times, {
        r4_t *r = r4(str, expr);

        if (r->valid == false) {

            printf("Bench r4 error\n");
            exit(1);
        }

        r4_free(r);
    });
    return true;
}

void bench_c(unsigned int times, char *str, char *expr) {
    regex_t regex;
    if (regcomp(&regex, expr, REG_EXTENDED)) {
        printf("Creg: error in regular expression.\n");
        exit(1);
    }
    RBENCH(times, {
        if (regexec(&regex, str, 0, NULL, 0)) {
            printf("Creg: error executing regular expression.\n");
            exit(1);
        }
    });

    regfree(&regex);
}

bool bench(unsigned int times, char *str, char *expr) {
    printf("%d:(%s)<%s>\n", times, str, expr);
    printf("c:");
    bench_c(times, str, expr);
    printf("r:");
    bench_r4(times, str, expr);
    return true;
}

void test_r4_next() {
    r4_t *r = r4_new();
    char *str = "abcdefghijklmnop";
    char *reg = "(\\w\\w\\w\\w)";
    r = r4(str, reg);
    assert(r->valid);
    assert(r->match_count == 1);
    assert(!strcmp(r->matches[0], "abcd"));
    // Again with same regex as parameter
    r = r4_next(r, reg);
    assert(r->valid);
    assert(r->match_count == 1);
    assert(!strcmp(r->matches[0], "efgh"));
    // Again with same regex as parameter
    r = r4_next(r, reg);
    assert(r->valid);
    assert(r->match_count == 1);
    assert(!strcmp(r->matches[0], "ijkl"));
    // Reuse expression, NULL parameter
    r = r4_next(r, NULL);
    assert(r->valid);
    assert(r->match_count == 1);
    assert(!strcmp(r->matches[0], "mnop"));
    // No results using r4_next
    r = r4_next(r, NULL);
    assert(r->valid);
    assert(r->match_count == 0);
    // Again no results using r4_next, Shouldn't crash
    r = r4_next(r, NULL);
    assert(r->valid);
    assert(r->match_count == 0);
    r4_free(r);
}

void bench_all(unsigned int times) {
    assert(bench(times, "suvw",
                 "[abcdefghijklmnopqrstuvw][abcdefghijklmnopqrstuvw]["
                 "abcdefghijklmnopqrstuvw][abcdefghijklmnopqrstuvw]"));
    assert(bench(times, "ponyyy", "^p+o.*yyy$$$$"));
    assert(bench(times, "                   ponyyzd", "p+o.*yyzd$$$$"));
    assert(bench(times, "abc", "def|gek|abc"));
    assert(bench(times, "abc", "def|a?b?c|def"));
    assert(bench(times, "NL18RABO0322309700",
                 "([A-Z]{2})([0-9]{2})([A-Z]{4}[0-9])([0-9]+)$"));
    assert(bench(times, "a 1 b 2 c 3 d 4 ", "([A-Z0-9 ]+)"));
}

bool r4_match_stats(char *str, char *expr) {
    r4_t *r = r4(str, expr);
    bool result = r->valid;
    printf("%d:(%s)<%s>\n", r->validation_count, r->_str, r->_expr);
    for (unsigned i = 0; i < r->match_count; i++) {
        printf(" - match: \"%s\"\n", r->matches[i]);
    }
    r4_free(r);
    return result;
}

char test_r4_bug_check_capture_overflow(){
    // This is a former bug in r4.

    // Case one
    r4_t * r = r4("test","(test)+");
    assert(r->match_count == 1);
    r4_free(r);

    // Case two
    r = r4("tester","(t\\est\\e\\r)+");
    assert(r->match_count == 1);
    printf("%s\n", r->matches[0]);
    r4_free(r);

    // Case three
    r = r4("test","(t\\est\\e\\r)+");
    assert(r->match_count == 0);
    r4_free(r);
}

char test_r4_capture_dynamic_amount(){

    r4_t * r = r4("testtesttest","(test)+");
    assert(!strcmp(r->matches[0], "test"));
    assert(!strcmp(r->matches[1], "test"));
    assert(!strcmp(r->matches[2], "test"));

}

int main() {
    test_r4_capture_dynamic_amount();
    exit(0);
    // Group testing
    assert(r4_match_stats("aaadddd", "(a+)(d+)$"));
    assert(r4_match_stats("aaa", "(a+)$"));
    assert(r4_match_stats("aaadddd", "(d+)$"));
    assert(r4_match_stats("aaadddd", "(d+)"));
    assert(r4_match_stats("aaa\"dddd\"", "\"(d+)\""));
    assert(r4_match_stats("aaadddd", "(a*)(d+)$"));
    assert(r4_match_stats("aaa", "(a*)$"));
    assert(r4_match_stats("aaadddd", "(d*)$"));
    assert(r4_match_stats("aaadddd", "(d*)"));
    assert(r4_match_stats("aaa\"dddd\" ", "\"(d*)\"\\s*"));

    // Boundaries
    assert(r4_match_stats("a", "\\b"));
    assert(r4_match_stats("a", "\\ba$"));
    assert(r4_match_stats("a", "^\\ba$"));
    assert(r4_match_stats("aa", "\\b"));
    assert(!r4_match_stats("aa", "\\b$"));
    assert(r4_match_stats("a", "\\B"));
    assert(r4_match_stats("a", "\\Ba$"));
    assert(r4_match_stats("a", "^\\Ba$"));
    assert(r4_match_stats("aa", "\\B"));
    assert(!r4_match_stats("aa", "^\\B"));

    // Optional
    assert(!r4_match_stats("a", "?"));
    assert(r4_match_stats("a", "a?"));
    assert(r4_match_stats("a", "b?"));
    assert(r4_match_stats("a", "^b?"));
    assert(r4_match_stats("a", "a?$"));
    assert(!r4_match_stats("a", "b?$"));
    assert(r4_match_stats("a", "[def]?a$"));

    // Next tests
    test_r4_next();

    // Check if former known bugs are still fixed
    test_r4_bug_check_capture_overflow();

    char *c_function_regex =
        "(\\w[\\w\\d]*[\\s\\*]*)\\s*\\w[\\w\\d]*\\s*\\((.*)\\)\\s*\\{";
    r4_match_stats("int **main() {}", c_function_regex);
    r4_match_stats("int main(int argc, char *argv[],(void *)aaa) {}",
                   c_function_regex);

    assert(r4_match_stats("NL18RABO0322309700",
                          "(\\w{2})(\\d{2})(\\w{4}\\d)(\\d{10})"));

    // exit(0);
    unsigned int times = 1;
    bench_all(times);

    RBENCH(1, {
        assert(r4_match_stats("#define DEFINETEST 1",
                              "#define\\s(+[\\w\\d_]+)\\s+[\\w\\d_]+"));
        //    assert(r4_match_stats("#define DEFINETEST 1\n",
        // s "#define\\s+\\w[\\d\\w_]+\\s+[\\w\\d_]\\s*"));

        assert(!r4_match_stats("aa", "aaaa"));
        assert(r4_match_stats("ponyyy", "^p+o.*yyy$$$$"));
        assert(!r4_match_stats("ponyyy", "p%+o.*yyy$$$$"));
        assert(!r4_match_stats("ponyyyd", "^p+o.*yyz$$$$"));
        assert(r4_match_stats("123", "[0-2][2-2][1-3]$"));
        assert(r4_match_stats("aaaabC5", "(a)(\\w)a*(a)\\w[A-Z][0-9]$"));
        assert(r4_match_stats("abcdeeeeee", "ab(cdeee)e"));
        assert(r4_match_stats("1234567", "12(.*)67$"));
        assert(r4_match_stats("12111678993", "12(.*)67(.*)3$"));
        assert(r4_match_stats("NL17RABO0322309700", "NL(.*)R(.*)0(.*)0(.*)0$"));

        assert(r4_match_stats("NL18RABO0322309700",
                              "(\\w{2})(\\d{2})(\\w{4}\\d)(\\d+)$"));
        assert(r4_match_stats("NL18RABO0322309700garbage",
                              "(\\w{2})(\\d{2})(\\w{4}\\d)(\\d+)"));
        assert(r4_match_stats("NL18RABO0322309700",
                              "(\\w{2})(\\d{2})(\\w{4}\\d)(\\d+)$"));
        assert(r4_match_stats(" NL18RABO0322309700",
                              "(\\w{2})(\\d{2})(\\w{4}\\d)(\\d+)$"));
        assert(r4_match_stats("  NL18RABO0322309700",
                              "(\\w{2})(\\d{2})(\\w{4}\\d)(\\d+)$"));
        assert(
            r4_match_stats("NL18RABO0", "(\\w\\w)(\\d\\d)(\\w\\w\\w\\w\\d)$"));
        assert(r4_match_stats("q", "\\q$"));
        assert(r4_match_stats("ab123", "[a-z0-9]+$"));
        assert(r4_match_stats("ppppony", "p*pppony"));
        assert(r4_match_stats("aa", "a{2}$"));
        assert(r4_match_stats("A23", "[0-2A-z][2-2][1-3]$"));
        assert(r4_match_stats("z23", "[0-2A-z][2-2][1-3]$"));
        assert(r4_match_stats("r23", "[0-2Ar][2-2][1-3]$"));
        assert(r4_match_stats("test", "\\w\\w\\w\\w$"));
        assert(!r4_match_stats("test", "\\W\\w\\w\\w$"));
        assert(r4_match_stats("1est", "\\W\\w\\w\\w$"));
        assert(r4_match_stats("1est", "\\d\\w\\w\\w$"));
        assert(r4_match_stats("Aest", "\\D\\w\\w\\w$"));
        assert(r4_match_stats("abc", "[ab]+"));
        assert(!r4_match_stats("abc", "[ab]+$"));
        assert(r4_match_stats("abc", "[abc]+$"));
        assert(!r4_match_stats("a", "[^ba]"));
        assert(!r4_match_stats("a", "[^ab]"));
        assert(r4_match_stats("                     ponyyzd", "p+o.*yyzd$$$$"));
        assert(r4_match_stats("abc", "def|gek|abc"));
        assert(!r4_match_stats("abc", "def|gek|abd"));
        assert(r4_match_stats("abc", "def|abc|def"));
        assert(r4_match_stats(
            "suwv", "[abcdesfghijklmnopqrtuvw][abcdefghijklmnopqrstuvw]["
                    "abcdefghijklmnopqrstuvw][abcdefghijklmnopqrstuvw]"));

        assert(r4_match_stats("123", "(.*)(.*)(.*)"));

        assert(r4_match_stats("#include \"test.c\"", "#include\\s+\"(.*)\""));
        assert(r4_match_stats("#define TEST_JE VALUE",
                              "#define\\s+([A-Za-z_0-9]+)\\s+([A-Za-z_0-9]+)"));
        //
        
    });

    return 0;
}
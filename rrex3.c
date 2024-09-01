#include "rlib.h"
#include "rrex.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#define DEBUG 1

struct rrex3_t;

typedef void (*rrex3_function)(struct rrex3_t *);

typedef struct rrex3_t {
    void (*functions[256])(struct rrex3_t *);
    void (*slash_functions[256])(struct rrex3_t *);
    bool valid;
    bool exit;
    char _expr[4096];
    char _str[4096];
    char *expr;
    char *str;
    bool inside_brackets;
    bool pattern_error;
    bool match_from_start;
    char bytecode;
    rrex3_function function;
    struct {
        void (*function)(struct rrex3_t *);
        char *expr;
        char *str;
        char bytecode;
    } before_previous;
    struct {
        void (*function)(struct rrex3_t *);
        char *expr;
        char *str;
        char bytecode;
    } previous;
    struct {
        void (*function)(struct rrex3_t *);
        char *expr;
        char *str;
        char bytecode;
    } failed;
} rrex3_t;

bool rrex3_move(rrex3_t *, bool);
bool rrex3_repeat(rrex3_t *);

void rrex3_cmp_literal_range(rrex3_t *rrex3) {
#if DEBUG == 1
    printf("Range check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    rrex3_set_previous(rrex3);

    char *start = *rrex3->expr;
    rrex3->expr++;
    rrex3->expr++;
    char *end = *rrex3->expr;
    if (*rrex3->str >= start && *rrex3->str <= end) {
        rrex3->str++;
        rrex3->valid = true;
    } else {
        rrex3->valid = false;
    }
    rrex3->expr++;
}

void rrex3_cmp_literal(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    if (rrex3->inside_brackets) {
        if (isalpharange(rrex3->expr) || isdigitrange(rrex3->expr)) {
            rrex3_cmp_literal_range(rrex3);
            return;
        }
    }
#if DEBUG == 1
    printf("Literal check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
           rrex3->valid);
#endif
    if (*(rrex3)->expr == *(rrex3)->str) {
        rrex3->expr++;
        rrex3->str++;
        rrex3->valid = true;
        return;
    }
    rrex3->expr++;
    rrex3->valid = false;
}

void rrex3_cmp_dot(rrex3_t *rrex3) {
#if DEBUG == 1
    printf("Dot check (any char): %c:%c:%d\n", *rrex3->expr, *rrex3->str,
           rrex3->valid);
#endif
    rrex3_set_previous(rrex3);

    if (*rrex3->str != '\n') {
        rrex3->str++;
    } else {
        rrex3->valid = false;
    }
    rrex3->expr++;
}

void rrex3_cmp_question_mark(rrex3_t *rrex3) {
#if DEBUG == 1
    printf("Question mark check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
           rrex3->valid);
#endif
    rrex3_set_previous(rrex3);

    if (rrex3->valid == false)
        rrex3->valid = true;
    rrex3->expr++;
}

void rrex3_cmp_whitespace(rrex3_t *rrex3) {
#if DEBUG == 1
    printf("Whitespace check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
           rrex3->valid);
#endif
    rrex3_set_previous(rrex3);

    char c = *rrex3->expr;
    rrex3->valid = c == ' ' || c == '\n' || c == '\t';
    if (rrex3->valid) {
        rrex3->str++;
    }
    rrex3->expr++;
}

void rrex3_cmp_whitespace_upper(rrex3_t *rrex3) {
#if DEBUG == 1
    printf("Non whitespace check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
           rrex3->valid);
#endif
    rrex3_set_previous(rrex3);

    char c = *rrex3->expr;
    rrex3->valid = !(c == ' ' || c == '\n' || c == '\t');
    if (rrex3->valid) {
        rrex3->str++;
    }
    rrex3->expr++;
}

/*


  [+ operator matches first a on the left]
  [+ operator matches first a on the right]
  [does this a few times]
  [in the end, both don't match, but it matched before, so it sets pointer after
  the last match and valid = true] [it comes to dollar sign and checks if it's
  on
*/

void rrex3_cmp_plus(rrex3_t *rrex3) {
#if DEBUG == 1
    printf("Plus check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    rrex3_set_previous(rrex3);

    if (rrex3->valid) {
        rrex3->str--;
    } else {
        return;
    }
    char *original_expr = rrex3->expr;
    char *next = original_expr + 1;
    char *loop_expr = rrex3->previous.expr - 1;
    if (*loop_expr == '+') {
        rrex3->valid = false;
        rrex3->pattern_error = true;
        rrex3->expr++;
        return;
    }
    bool success_next = false;
    bool success_next_once = false;
    bool success_current = false;
    char *next_next = NULL;
    char *next_str = original_expr;
    while (*rrex3->str) {
        // Check if next matches
        char *original_str = rrex3->str;
        rrex3->expr = next;
        rrex3->valid = true;
        if (rrex3_move(rrex3, false)) {
            success_next = true;
            next_next = rrex3->expr;
            next_str = rrex3->str;
            success_next_once = true;
        } else {
            success_next = false;
        }
        if (success_next_once && !success_next) {
            break;
        }
        // Check if current matches
        rrex3->str = original_str;
        rrex3->expr = loop_expr;
        rrex3->valid = true;
        if (!rrex3_move(rrex3, false)) {
            success_current = false;
        } else {
            success_current = true;
            if (!success_next) {
                next_next = rrex3->expr + 1; // +1 is the * itself
                next_str = rrex3->str;
            }
        }
        if (success_next && !success_current) {
            break;
        }
    }
    if (!next_next)
        rrex3->expr = next;
    else {
        rrex3->expr = next_next;
    }
    rrex3->str = next_str;
    rrex3->valid = true;
}

void rrex3_cmp_asterisk(rrex3_t *rrex3) {
#if DEBUG == 1
    printf("Asterisk check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
           rrex3->valid);
#endif

    if (rrex3->valid) {
        rrex3->str--;
    } else {
        rrex3->valid = true;
        rrex3->expr++;
        return;
    }
    char *original_expr = rrex3->expr;
    char *next = original_expr + 1;
    char *loop_expr = original_expr - 1;
    if (*rrex3->previous.expr == '[') {
        loop_expr = rrex3->previous.expr;
        // printf("%s\n", loop_expr);
        // exit(0);
    }

    // return;
    //  rrex3_set_previous(rrex3);

    if (*loop_expr == '*') {
        rrex3->valid = false;
        rrex3->pattern_error = true;
        rrex3->expr++;
        return;
    }
    bool success_next = false;
    bool success_next_once = false;
    bool success_current = false;
    char *next_next = NULL;
    char *next_str = original_expr;
    int invalid_count;
    while (*rrex3->str) {
        // Check if next matches
        char *original_str = rrex3->str;
        rrex3->expr = next;
        invalid_count = 0;
        rrex3->valid = true;
        printf("HIERR\n");
        if (rrex3_move(rrex3, false)) {
            success_next = true;
            next_next = rrex3->expr;
            next_str = rrex3->str;
            success_next_once = true;
        } else {
            invalid_count++;
            success_next = false;
        }
        if (success_next_once && !success_next) {
            break;
        }
        // Check if current matches
        rrex3->str = original_str;
        rrex3->expr = loop_expr;
        rrex3->valid = true;
        if (!rrex3_move(rrex3, false)) {
            success_current = false;
            invalid_count++;
        } else {
            success_current = true;
            if (!success_next) {
                next_next = rrex3->expr + 1; // +1 is the * itself
                next_str = rrex3->str;
            }
        }

        if (success_next && !success_current || invalid_count == 2) {
            break;
        }
    }
    if (!next_next)
        rrex3->expr = next;
    else {
        rrex3->expr = next_next;
    }
    rrex3->str = next_str;
    rrex3->valid = true;
}

void rrex3_cmp_roof(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

#if DEBUG == 1
    printf("<Roof check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    rrex3->valid = rrex3->str == rrex3->str;
    rrex3->match_from_start = true;
    rrex3->expr++;
}
void rrex3_cmp_dollar(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

#if DEBUG == 1
    printf("Dollar check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (*rrex3->str) {
        rrex3->valid = false;
    }
    rrex3->expr++;
}

void rrex3_cmp_w(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    rrex3->expr++;
#if DEBUG == 1
    printf("Word check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (isalpha(*rrex3->str)) {
        rrex3->str++;
    } else {
        rrex3->valid = false;
    }
}
void rrex3_cmp_w_upper(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    rrex3->expr++;
#if DEBUG == 1
    printf("!Word check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (!isalpha(*rrex3->str)) {
        rrex3->str++;
    } else {
        rrex3->valid = false;
    }
}

void rrex3_cmp_d(rrex3_t *rrex3) {

    rrex3_set_previous(rrex3);

    rrex3->expr++;
#if DEBUG == 1
    printf("Digit check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (isdigit(*rrex3->str)) {
        rrex3->str++;
    } else {
        rrex3->valid = false;
    }
}
void rrex3_cmp_d_upper(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    rrex3->expr++;
#if DEBUG == 1
    printf("!Digit check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (!isdigit(*rrex3->str)) {
        rrex3->str++;
    } else {
        rrex3->valid = false;
    }
}

void rrex3_cmp_slash(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    rrex3->expr++;

    rrex3->bytecode = *rrex3->expr;
    rrex3->function = rrex3->slash_functions[rrex3->bytecode];
    rrex3->function(rrex3);
}

int collect_digits(rrex3_t *rrex3) {
    char _output[20];
    char *output = _output;
    while (isdigit(*rrex3->expr)) {

        *output = *rrex3->expr;
        rrex3->expr++;
        *output++;
    }
    *output = 0;
    return atoi(_output);
}

void rrex3_cmp_range(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    char *loop_code = rrex3->before_previous.expr;
    char *expr_original = rrex3->expr;
    rrex3->expr++;
    char srange_start[10];
    int range_start = collect_digits(rrex3) - 1;
    int range_end = 0;
    if (*rrex3->expr == ',') {
        rrex3->expr++;
        range_end = collect_digits(rrex3);
    }
    rrex3->expr++;
    int times_valid = 0;
    while (*rrex3->str) {
        rrex3->expr = loop_code;
        rrex3_move(rrex3, false);
        if (rrex3->valid = false) {
            break;
        } else {
            times_valid++;
        }
        if (range_end) {
            if (times_valid >= range_start && times_valid == range_end - 1) {
                rrex3->valid = true;
            } else {
                rrex3->valid = false;
            }
            break;
        } else if (range_start) {
            if (times_valid == range_start) {
                rrex3->valid = true;
                break;
            }
        }
    }
    rrex3->expr = strchr(expr_original, '}') + 1;
}

void rrex3_cmp_word_start_or_end(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);
    bool valid = false;
    if (isalpha(*rrex3->str)) {
        if (rrex3->_str != rrex3->str) {
            if (!isalpha(*(rrex3->str - 1))) {
                valid = true;
            }
        } else {
            valid = true;
        }
    } else if (isalpha(isalpha(*rrex3->str) && !isalpha(*rrex3->str + 1))) {
        valid = true;
    }
    rrex3->expr++;
    rrex3->valid = valid;
}
void rrex3_cmp_word_not_start_or_end(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    rrex3_cmp_word_start_or_end(rrex3);
    rrex3->valid = !rrex3->valid;
}

void rrex3_cmp_brackets(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);
    char *original_expr = rrex3->expr;
    rrex3->expr++;
    rrex3->inside_brackets = true;
    bool valid_once = false;
    bool reversed = false;
    if (*rrex3->expr == '^') {
        reversed = true;
        rrex3->expr++;
    }
    bool valid = false;
    while (*rrex3->expr != ']' && *rrex3->expr != 0) {
        rrex3->valid = true;
        valid = rrex3_move(rrex3, false);
        if (reversed) {
            valid = !valid;
        }
        if (valid) {
            valid_once = true;
            if (!reversed) {
                valid_once = true;
                break;
            }
        } else {
            if (reversed) {
                valid_once = false;
                break;
            }
        }
    }
    if (valid_once && reversed) {
        rrex3->str++;
    }
    while (*rrex3->expr != ']' && rrex3->expr != 0)
        rrex3->expr++;
    if (*rrex3->expr != 0)
        rrex3->expr++;

    rrex3->valid = valid_once;
    rrex3->inside_brackets = false;
    char *previous_expr = rrex3->expr;
    rrex3->expr = original_expr;
    rrex3_set_previous(rrex3);
    rrex3->expr = previous_expr;
}

void rrex3_cmp_pipe(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

#if DEBUG == 1
    printf("Pipe check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (rrex3->valid == true) {
        rrex3->exit = true;
    } else {
        rrex3->valid = true;
    }
    rrex3->expr++;
}

void rrex3_init(rrex3_t *rrex3) {
    memset(rrex3->_str, 0, sizeof(rrex3->_str));
    for (__uint8_t i = 0; i < 255; i++) {
        rrex3->functions[i] = rrex3_cmp_literal;
        rrex3->slash_functions[i] == rrex3_cmp_literal;
    }
    rrex3->functions['?'] = rrex3_cmp_question_mark;
    rrex3->functions['^'] = rrex3_cmp_roof;
    rrex3->functions['$'] = rrex3_cmp_dollar;
    rrex3->functions['.'] = rrex3_cmp_dot;
    rrex3->functions['*'] = rrex3_cmp_asterisk;
    rrex3->functions['+'] = rrex3_cmp_plus;
    rrex3->functions['|'] = rrex3_cmp_pipe;
    rrex3->functions['\\'] = rrex3_cmp_slash;
    rrex3->functions['{'] = rrex3_cmp_range;
    rrex3->functions['['] = rrex3_cmp_brackets;
    rrex3->slash_functions['w'] = rrex3_cmp_w;
    rrex3->slash_functions['W'] = rrex3_cmp_w_upper;
    rrex3->slash_functions['d'] = rrex3_cmp_d;
    rrex3->slash_functions['D'] = rrex3_cmp_d_upper;
    rrex3->slash_functions['s'] = rrex3_cmp_whitespace;
    rrex3->slash_functions['S'] = rrex3_cmp_whitespace_upper;
    rrex3->slash_functions['b'] = rrex3_cmp_word_start_or_end;
    rrex3->slash_functions['B'] = rrex3_cmp_word_not_start_or_end;
    rrex3->valid = true;
    rrex3->pattern_error = false;
    rrex3->inside_brackets = false;
    rrex3->exit = false;
    rrex3->before_previous.expr = NULL;
    rrex3->before_previous.str = NULL;
    rrex3->before_previous.bytecode = 0;
    rrex3->previous.expr = NULL;
    rrex3->previous.str = NULL;
    rrex3->previous.bytecode = 0;
    rrex3->failed.expr = NULL;
    rrex3->failed.str = NULL;
    rrex3->failed.bytecode = 0;
    rrex3->match_from_start = false;
}

bool rrex3_repeat(rrex3_t *rrex3) {
    rrex3->expr = rrex3->previous.expr - 1; // WORKAROUND
    rrex3->bytecode = rrex3->previous.bytecode;
    return rrex3_move(rrex3, false);
}

void rrex3_set_previous(rrex3_t *rrex3) {
    // memcpy(&rrex3->before_previous,
    // &rrex3->previous,sizeof(rrex3->previous));
    rrex3->before_previous.function = rrex3->previous.function;
    rrex3->before_previous.expr = rrex3->previous.expr;
    rrex3->before_previous.str = rrex3->previous.str;
    rrex3->before_previous.bytecode = rrex3->previous.bytecode;

    rrex3->previous.function = rrex3->function;
    rrex3->previous.expr = rrex3->expr;
    rrex3->previous.str = rrex3->str;
    rrex3->previous.bytecode = rrex3->bytecode;
}

bool rrex3_move(rrex3_t *rrex3, bool resume_on_fail) {
    char *original_expr = rrex3->expr;
    char *original_str = rrex3->str;
    rrex3->bytecode = *rrex3->expr;
    rrex3->function = rrex3->functions[rrex3->bytecode];
    rrex3->function(rrex3);
    if (rrex3->pattern_error)
        return false;
    if (resume_on_fail && !rrex3->valid && *rrex3->expr) {
        // rrex3_set_previous(rrex3);
        rrex3->failed.bytecode = rrex3->bytecode;
        rrex3->failed.function = rrex3->function;
        rrex3->failed.expr = original_expr;
        rrex3->failed.str = original_str;
        rrex3->bytecode = *rrex3->expr;
        rrex3->function = rrex3->functions[rrex3->bytecode];
        rrex3->function(rrex3);
        if (!rrex3->valid && !rrex3->pattern_error) {

            if (*rrex3->str) {
                char *pipe_position = strstr(rrex3->expr, "|");
                if (pipe_position != NULL) {
                    rrex3->expr = pipe_position + 1;
                    rrex3->str = rrex3->_str;
                    rrex3->valid = true;
                    return true;
                } else {
                    printf("IS NULL\n");
                }
            }
            if (rrex3->match_from_start) {
                rrex3->valid = false;
                return rrex3->valid;
            }
            if (!*rrex3->str++) {
                rrex3->valid = false;
                return rrex3->valid;
            }
            rrex3->expr = rrex3->_expr;
            if (rrex3->str)
                rrex3->valid = true;
        }
    }
    return rrex3->valid;
}

bool rrex3(char *str, char *expr) {
#if DEBUG == 1
    printf("Regex check: %s:%s:%d\n", expr, str, 1);
#endif
    rrex3_t rrex3;
    rrex3_init(&rrex3);
    strcpy(rrex3._str, str);
    strcpy(rrex3._expr, expr);
    rrex3.str = rrex3._str;
    rrex3.expr = rrex3._expr;
    while (*rrex3.expr && !rrex3.exit) {
        if (!rrex3_move(&rrex3, true))
            return false;
    }
    return rrex3.valid;
}
int main() {

    rassert(rrex3("aaaaaaa", "a*a$"));
    rassert(rrex3("ababa", "a*b*a*b*a$"));
    rassert(rrex3("#include\"test.h\"a", "#include.*\".*\"a$"));
    rassert(rrex3("aaaaaad", "a*d$"));
    rassert(rrex3("abcdef", "abd?cdef"));
    rassert(!rrex3("abcdef", "abd?def"));
    rassert(rrex3("abcdef", "def"));
    rassert(!rrex3("abcdef", "^def"));
    rassert(rrex3("abcdef", "def$"));
    rassert(!rrex3("abcdef", "^abc$"));
    rassert(rrex3("aB!.#1", "......"));
    rassert(!rrex3("aB!.#\n", "      ......"));
    rassert(!rrex3("aaaaaad", "q+d$"));
    rassert(rrex3("aaaaaaa", "a+a$"));
    rassert(rrex3("aaaaaad", "q*d$"));
    rassert(!rrex3("aaaaaad", "^q*d$"));

    // Asterisk function
    rassert(rrex3("pony", "p*ony"));
    rassert(rrex3("pppony", "p*ony"));
    rassert(rrex3("ppony", "p*pony"));
    rassert(rrex3("pppony", "pp*pony"));
    rassert(rrex3("pppony", ".*pony"));
    rassert(rrex3("pony", ".*ony"));
    rassert(rrex3("pony", "po*ny"));

    // Plus function
    rassert(rrex3("pony", "p+ony"));
    rassert(!rrex3("ony", "p+ony"));
    rassert(rrex3("ppony", "p+pony"));
    rassert(rrex3("pppony", "pp+pony"));
    rassert(rrex3("pppony", ".+pony"));
    rassert(rrex3("pony", ".+ony"));
    rassert(rrex3("pony", "po+ny"));

    // Slash functions
    rassert(rrex3("a", "\\w"));
    rassert(!rrex3("1", "\\w"));
    rassert(rrex3("1", "\\W"));
    rassert(!rrex3("a", "\\W"));
    rassert(rrex3("a", "\\S"));
    rassert(!rrex3(" ", "\\s"));
    rassert(!rrex3("\t", "\\s"));
    rassert(!rrex3("\n", "\\s"));
    rassert(rrex3("1", "\\d"));
    rassert(!rrex3("a", "\\d"));
    rassert(rrex3("a", "\\D"));
    rassert(!rrex3("1", "\\D"));
    rassert(rrex3("abc", "\\b"));

    rassert(rrex3("abc", "\\babc"));
    rassert(!rrex3("abc", "a\\b"));
    rassert(!rrex3("abc", "ab\\b"));
    rassert(!rrex3("abc", "abc\\b"));
    rassert(rrex3("abc", "a\\Bbc"));
    rassert(rrex3("abc", "ab\\B")) rassert(rrex3("1ab", "1\\Bab"));
    rassert(rrex3("abc", "a\\Bbc"));

    // Pipe
    // rassert(rrex3("abc","abc|def"));
    rassert(rrex3("abc", "def|jkl|abc"));
    rassert(rrex3("abc", "abc|def"));

    rassert(rrex3("rhq", "def|rhq|rha"));
    rassert(rrex3("abc", "abc|def"));

    // Repeat
    rassert(rrex3("aaaaa", "a{4}"));

    rassert(rrex3("aaaa", "a{1,3}a"));

    // Range
    rassert(rrex3("abc", "[abc][abc][abc]$"))
        rassert(rrex3("def", "[^abc][^abc][^abc]$"));
    rassert(rrex3("defa", "[^abc][^abc][^abc][abc]$"));
    rassert(rrex3("0-9", "0-9"));
    rassert(rrex3("55-9", "[^6-9]5-9$"));
    rassert(rrex3("a", "[a-z]$"));
    rassert(rrex3("A", "[A-Z]$"));
    rassert(rrex3("5", "[0-9]$"));
    rassert(!rrex3("a", "[^a-z]$"));
    rassert(!rrex3("A", "[^A-Z]$"));
    rassert(!rrex3("5", "[^0-9]$"));
    rassert(rrex3("123123", "[0-9]*123"));

    return rtest_end("");
}
#include "rlib.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define DEBUG 1

struct rrex3_t;

typedef void (*rrex3_function)(struct rrex3_t *);

typedef struct rrex3_t {
    void (*functions[256])(struct rrex3_t *);
    bool valid;
    char _expr[4096];
    char _str[4096];
    char *expr;
    char *str;
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

void rrex3_cmp_literal(rrex3_t *rrex3) {
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
    if (rrex3->valid == false)
        rrex3->valid = true;
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
    printf("Plus mark check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
           rrex3->valid);
#endif
    bool validated_once = rrex3->valid;
    if (rrex3->valid == false) {
        return;
    }
    char *original_expr = rrex3->expr;
    char *next = original_expr + 1;
    char *loop_expr = rrex3->before_previous.expr - 1;
    while (true) {
        rrex3->expr = next;
        if (rrex3_move(rrex3, false)) {
            break;
        }
        rrex3->valid = true;
        rrex3->expr = loop_expr;
        if (!rrex3_move(rrex3, false)) {
            break;
        }
    }
    rrex3->valid = validated_once;
}

void rrex3_cmp_asterisk(rrex3_t *rrex3) {
#if DEBUG == 1
    printf("Asterisk check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
           rrex3->valid);
#endif
    if (rrex3->valid) {
        rrex3->str--;
    }
    char *original_expr = rrex3->expr;
    char *next = original_expr + 1;
    char *loop_expr = rrex3->previous.expr - 1;
    bool success = false;
    char *next_next = NULL;
    bool fail_on_no_match = false;
    char *next_str = original_expr;
    while (true) {
        rrex3->expr = next;
        char *original_str = rrex3->str;
        rrex3->valid = true;
        if (rrex3_move(rrex3, false)) {
            success = true;
            next_next = rrex3->expr;
            next_str = rrex3->str;
        } else {
            success = false;
            next_next = NULL;
        }
        rrex3->str = original_str;
        rrex3->expr = loop_expr;
        rrex3->valid = true;
        if (!rrex3_move(rrex3, false)) {
            break;
        }else if(!success){
            next_next = rrex3->expr;
            next_str = rrex3->str;
        }
        if (success) {
            break;
        }
    }
    if (!next_next)
        rrex3->expr = next;
    else {
        rrex3->expr = next_next;
        rrex3->str = next_str;
    }

    rrex3->valid = true;
}

void rrex3_cmp_roof(rrex3_t *rrex3) {
#if DEBUG == 1
    printf("<Roof check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    rrex3->valid = rrex3->str == rrex3->str;
    rrex3->match_from_start = true;
    rrex3->expr++;
}
void rrex3_cmp_dollar(rrex3_t *rrex3) {
#if DEBUG == 1
    printf("Dollar check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (*rrex3->str) {
        rrex3->valid = false;
    }
    rrex3->expr++;
}

void rrex3_init(rrex3_t *rrex3) {
    for (__uint8_t i = 0; i < 255; i++) {
        rrex3->functions[i] = rrex3_cmp_literal;
    }
    rrex3->functions['?'] = rrex3_cmp_question_mark;
    rrex3->functions['^'] = rrex3_cmp_roof;
    rrex3->functions['$'] = rrex3_cmp_dollar;
    rrex3->functions['.'] = rrex3_cmp_dot;
    rrex3->functions['*'] = rrex3_cmp_asterisk;
    rrex3->functions['+'] = rrex3_cmp_plus;
    rrex3->valid = true;
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
    rrex3_set_previous(rrex3);
    rrex3->bytecode = *rrex3->expr;
    rrex3->function = rrex3->functions[rrex3->bytecode];
    rrex3->function(rrex3);
    if (resume_on_fail && !rrex3->valid && *rrex3->expr) {
        rrex3_set_previous(rrex3);
        rrex3->failed.bytecode = rrex3->bytecode;
        rrex3->failed.function = rrex3->function;
        rrex3->failed.expr = original_expr;
        rrex3->failed.str = original_str;
        rrex3->bytecode = *rrex3->expr;
        rrex3->function = rrex3->functions[rrex3->bytecode];
        rrex3->function(rrex3);
        if (!rrex3->valid) {
            if (rrex3->match_from_start) {
                rrex3->valid = false;
                return false;
            }
            if (!*rrex3->str++) {
                printf("HIER UUIT!!\n");
                rrex3->valid = false;
                return false;
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
    while (*rrex3.expr) {
        if (!rrex3_move(&rrex3, true))
            return false;
    }
    return rrex3.valid;
}
int main() {

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

    return 0;
}
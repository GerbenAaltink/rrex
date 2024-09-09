// RETOOR - Sep  9 2024
#define RREX3_DEBUG 0
#ifndef RREX3_H
#define RREX3_H
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef RREX3_DEBUG
#define RREX3_DEBUG 0
#endif

struct rrex3_t;

typedef void (*rrex3_function)(struct rrex3_t *);

typedef struct rrex3_t {
    void (*functions[254])(struct rrex3_t *);
    void (*slash_functions[254])(struct rrex3_t *);
    bool valid;
    int match_count;
    int match_capacity;
    char **matches;
    bool exit;
    char *__expr;
    char *__str;
    char *_expr;
    char *_str;
    char *expr;
    char *str;
    char *compiled;
    bool inside_brackets;
    bool inside_parentheses;
    bool pattern_error;
    bool match_from_start;
    char bytecode;
    rrex3_function function;
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

static bool isdigitrange(char *s) {
    if (!isdigit(*s)) {
        return false;
    }
    if (*(s + 1) != '-') {
        return false;
    }
    return isdigit(*(s + 2));
}

static bool isalpharange(char *s) {
    if (!isalpha(*s)) {
        return false;
    }
    if (*(s + 1) != '-') {
        return false;
    }
    return isalpha(*(s + 2));
}

void rrex3_free_matches(rrex3_t *rrex3) {
    if (!rrex3->matches)
        return;
    for (int i = 0; i < rrex3->match_count; i++) {
        free(rrex3->matches[i]);
    }
    free(rrex3->matches);
    rrex3->matches = NULL;
    rrex3->match_count = 0;
    rrex3->match_capacity = 0;
}

void rrex3_free(rrex3_t *rrex3) {
    if (!rrex3)
        return;
    if (rrex3->compiled) {
        free(rrex3->compiled);
        rrex3->compiled = NULL;
    }
    rrex3_free_matches(rrex3);
    free(rrex3);
    rrex3 = NULL;
}
static bool rrex3_move(rrex3_t *, bool);
static void rrex3_set_previous(rrex3_t *);
inline static void rrex3_cmp_asterisk(rrex3_t *);
void rrex3_cmp_literal_range(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    printf("Range check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    rrex3_set_previous(rrex3);

    char start = *rrex3->expr;
    rrex3->expr++;
    rrex3->expr++;
    char end = *rrex3->expr;
    if (*rrex3->str >= start && *rrex3->str <= end) {
        rrex3->str++;
        rrex3->valid = true;
    } else {
        rrex3->valid = false;
    }
    rrex3->expr++;
}

bool rrex3_is_function(char chr) {
    if (chr == ']' || chr == ')' || chr == '\\' || chr == '?' || chr == '+' ||
        chr == '*')
        return true;
    return false;
}

inline static void rrex3_cmp_literal(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    if (rrex3->inside_brackets) {
        if (isalpharange(rrex3->expr) || isdigitrange(rrex3->expr)) {
            rrex3_cmp_literal_range(rrex3);
            return;
        }
    }
#if RREX3_DEBUG == 1
    printf("Literal check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
           rrex3->valid);

#endif
    if (*rrex3->expr == 0 && !*rrex3->str) {
        printf("ERROR, EMPTY CHECK\n");
        // exit(1);
    }
    if (rrex3->valid == false) {
        rrex3->expr++;
        return;
    }

    if (*rrex3->expr == *rrex3->str) {
        rrex3->expr++;
        rrex3->str++;
        rrex3->valid = true;
        // if(*rrex3->expr &&rrex3->functions[(int)*rrex3->expr] ==
        // rrex3_cmp_literal && !rrex3->inside_brackets &&
        //! rrex3_is_function(*rrex3->expr)){ rrex3_cmp_literal(rrex3);
        //   if(rrex3->valid == false){
        //  rrex3->expr--;
        // rrex3->valid = true;
        // }
        // }
        return;
    }
    rrex3->expr++;
    rrex3->valid = false;
}

inline static void rrex3_cmp_dot(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    printf("Dot check (any char): %c:%c:%d\n", *rrex3->expr, *rrex3->str,
           rrex3->valid);
#endif
    rrex3_set_previous(rrex3);
    rrex3->expr++;
    if (!rrex3->valid) {
        return;
    }
    if (*rrex3->str && *rrex3->str != '\n') {
        rrex3->str++;
        if (*rrex3->expr && *rrex3->expr == '.') {
            rrex3_cmp_dot(rrex3);
            return;
        } /*else if(*rrex3->expr && (*rrex3->expr == '*' || *rrex3->expr ==
         '+')){ char * next = strchr(rrex3->str,*(rrex3->expr + 1)); char *
         space = strchr(rrex3->str,'\n'); if(next && (!space || space > next)){
                 rrex3->str = next;
             }
         }*/
    } else {
        rrex3->valid = false;
    }
}

inline static void rrex3_cmp_question_mark(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    printf("Question mark check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
           rrex3->valid);
#endif
    rrex3_set_previous(rrex3);

    if (rrex3->valid == false)
        rrex3->valid = true;
    rrex3->expr++;
}

inline static void rrex3_cmp_whitespace(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
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

inline static void rrex3_cmp_whitespace_upper(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
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

inline static void rrex3_cmp_plus2(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
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
    char *next_str = rrex3->str;
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
        if (!*rrex3->str || !rrex3_move(rrex3, false)) {
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

inline static void rrex3_cmp_plus(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    rprintg("Asterisk start check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
            rrex3->valid);
#endif
    if (!rrex3->valid) {
        rrex3->expr++;
        return;
    }

    char *left = rrex3->previous.expr;
    // printf("%s\n",rrex3->str);
    char *right = rrex3->expr + 1;
    if (*right == ')') {
        right++;
    }
    int right_valid = 0;
    bool right_valid_once = false;
    char *expr = right;
    char *right_str = rrex3->str;
    ;
    char *right_expr = NULL;
    char *str = rrex3->str;
    bool first_time = true;
    bool left_valid = true;
    char *str_prev = NULL;
    bool valid_from_start = true;
    ;
    while (*rrex3->str) {
        if (!left_valid && !right_valid) {
            break;
        }
        if (right_valid && !left_valid) {
            str = right_str;
            break;
        }

        rrex3->expr = right;
        rrex3->str = str;
#if RREX3_DEBUG == 1
        printf("r");
#endif
        if (*rrex3->str && rrex3_move(rrex3, false)) {
            right_valid++;
            right_str = rrex3->str;
            expr = rrex3->expr;
            if (!right_valid_once) {
                right_expr = rrex3->expr;
                right_valid_once = true;
            }
        } else {
            right_valid = 0;
        }
        if (first_time) {
            first_time = false;
            valid_from_start = right_valid;
        }

        if (right_valid && !valid_from_start && right_valid > 0) {
            expr = right_expr - 1;
            ;
            if (*(right - 1) == ')') {
                expr = right - 1;
            }
            break;
        }

        if ((!right_valid && right_valid_once)) {
            expr = right_expr;
            if (*(right - 1) == ')') {
                str = str_prev;
                expr = right - 1;
            }
            break;
        }

        str_prev = str;
        rrex3->valid = true;
        rrex3->str = str;
        rrex3->expr = left;
#if RREX3_DEBUG == 1
        printf("l");
#endif
        if (rrex3_move(rrex3, false)) {
            left_valid = true;

            str = rrex3->str;
        } else {
            left_valid = false;
        }
    }

    rrex3->expr = expr;
    rrex3->str = str;
    rrex3->valid = true;

#if RREX3_DEBUG == 1
    rprintg("Asterisk end check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
            rrex3->valid);
#endif
}

inline static void rrex3_cmp_asterisk(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    rprintg("Asterisk start check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
            rrex3->valid);
#endif
    if (!rrex3->valid) {
        rrex3->valid = true;
        rrex3->expr++;
        return;
    }

    rrex3->str = rrex3->previous.str;
    char *left = rrex3->previous.expr;
    // printf("%s\n",rrex3->str);
    char *right = rrex3->expr + 1;
    if (*right == ')') {
        right++;
    }
    int right_valid = 0;
    bool right_valid_once = false;
    char *expr = right;
    char *right_str = rrex3->str;
    ;
    char *right_expr = NULL;
    char *str = rrex3->str;
    bool first_time = true;
    bool left_valid = true;
    char *str_prev = NULL;
    bool valid_from_start = true;
    ;
    while (*rrex3->str) {
        if (!left_valid && !right_valid) {
            break;
        }
        if (right_valid && !left_valid) {
            str = right_str;
            break;
        }

        rrex3->expr = right;
        rrex3->str = str;
#if RREX3_DEBUG == 1
        printf("r");
#endif
        if (*rrex3->str && rrex3_move(rrex3, false)) {
            right_valid++;
            right_str = rrex3->str;
            expr = rrex3->expr;
            if (!right_valid_once) {
                right_expr = rrex3->expr;
                right_valid_once = true;
            }
        } else {
            right_valid = 0;
        }
        if (first_time) {
            first_time = false;
            valid_from_start = right_valid;
        }

        if (right_valid && !valid_from_start && right_valid > 0) {
            expr = right_expr - 1;
            if (*(right - 1) == ')') {
                expr = right - 1;
            }
            break;
        }

        if ((!right_valid && right_valid_once)) {
            expr = right_expr;
            if (*(right - 1) == ')') {
                str = str_prev;
                expr = right - 1;
            }
            break;
        }

        str_prev = str;
        rrex3->valid = true;
        rrex3->str = str;
        rrex3->expr = left;
#if RREX3_DEBUG == 1
        printf("l");
#endif
        if (rrex3_move(rrex3, false)) {
            left_valid = true;
            str = rrex3->str;
        } else {
            left_valid = false;
        }
    }

    rrex3->expr = expr;
    rrex3->str = str;
    rrex3->valid = true;

#if RREX3_DEBUG == 1
    rprintg("Asterisk end check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
            rrex3->valid);
#endif
}

inline static void rrex3_cmp_asterisk2(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    rprintg("Asterisk start check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
            rrex3->valid);
#endif
    if (!rrex3->valid) {
        rrex3->valid = true;
        rrex3->expr++;
        return;
    }
    if (*rrex3->previous.expr == '*') {
        // Support for **
        rrex3->valid = false;
        // rrex3->pattern_error = true;
        rrex3->expr++;
        return;
    }
    rrex3->str = rrex3->previous.str;
    ;
    char *next = rrex3->expr + 1;
    char *next_original = NULL;
    if (*next == '*') {
        next++;
    }
    if (*next == ')' && *(next + 1)) {
        next_original = next;
        next++;
    }
    char *loop_expr = rrex3->previous.expr;
    bool success_next = false;
    bool success_next_once = false;
    bool success_current = false;
    char *right_next = NULL;
    char *right_str = rrex3->str;
    while (*rrex3->str && *rrex3->expr && *rrex3->expr != ')') {
        // Remember original_str because it's modified
        // by checking right and should be restored
        // for checking left so they're matching the
        // same value.
        char *original_str = rrex3->str;
        // Check if right matches.
        // if(*next != ')'){
        rrex3->expr = next;
        rrex3->valid = true;
        if (rrex3_move(rrex3, false)) {
            // Match rright.
            success_next = true;
            if (!next_original) {
                if (!success_next_once) {
                    right_next = rrex3->expr;
                }

            } else {
                right_next = next_original;
                break;
            }
            right_str = rrex3->str;
            success_next_once = true;
        } else {
            // No match Right.
            success_next = false;
        }
        //}
        if (success_next_once && !success_next) {
            // Matched previous time but now doesn't.
            break;
        }
        // Check if left matches.
        rrex3->str = original_str;
        rrex3->expr = loop_expr;
        rrex3->valid = true;
        if (!rrex3_move(rrex3, false)) {
            // No match left.
            success_current = false;
        } else {
            // Match left.
            success_current = true;
            // NOT SURE< WITHOUT DOET HETZELFDE:
            // original_str = rrex3->str;
            if (!success_next) {
                right_str = rrex3->str;
                if (*rrex3->expr != ')') {
                    right_next = rrex3->expr + 1; // +1 is the * itself

                } else {

                    // break;
                }
            }
        }

        if ((success_next && !success_current) ||
            (!success_next && !success_current)) {
            break;
        }
    }
    rrex3->expr = right_next;
    rrex3->str = right_str;
    rrex3->valid = true;
#if RREX3_DEBUG == 1
    rprintg("Asterisk end check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
            rrex3->valid);
#endif
}

inline static void rrex3_cmp_roof(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);
#if RREX3_DEBUG == 1
    printf("<Roof check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    rrex3->valid = rrex3->str == rrex3->_str;
    rrex3->match_from_start = true;
    rrex3->expr++;
}
inline static void rrex3_cmp_dollar(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

#if RREX3_DEBUG == 1
    printf("Dollar check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (*rrex3->str || !rrex3->valid) {
        rrex3->valid = false;
    }
    rrex3->expr++;
}

inline static void rrex3_cmp_w(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    rrex3->expr++;
#if RREX3_DEBUG == 1
    printf("Word check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (isalpha(*rrex3->str)) {
        rrex3->str++;
    } else {
        rrex3->valid = false;
    }
}
inline static void rrex3_cmp_w_upper(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    rrex3->expr++;
#if RREX3_DEBUG == 1
    printf("!Word check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (!isalpha(*rrex3->str)) {
        rrex3->str++;
    } else {
        rrex3->valid = false;
    }
}

inline static void rrex3_cmp_d(rrex3_t *rrex3) {

    rrex3_set_previous(rrex3);

    rrex3->expr++;
#if RREX3_DEBUG == 1
    printf("Digit check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (isdigit(*rrex3->str)) {
        rrex3->str++;
    } else {
        rrex3->valid = false;
    }
}
inline static void rrex3_cmp_d_upper(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    rrex3->expr++;
#if RREX3_DEBUG == 1
    printf("!Digit check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (!isdigit(*rrex3->str)) {
        rrex3->str++;
    } else {
        rrex3->valid = false;
    }
}

inline static void rrex3_cmp_slash(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    rrex3->expr++;

    rrex3->bytecode = *rrex3->expr;
    rrex3->function = rrex3->slash_functions[(int)rrex3->bytecode];
    rrex3->function(rrex3);
}

inline static int collect_digits(rrex3_t *rrex3) {
    char output[20];
    unsigned int digit_count = 0;
    while (isdigit(*rrex3->expr)) {

        output[digit_count] = *rrex3->expr;
        rrex3->expr++;
        digit_count++;
    }
    output[digit_count] = 0;
    return atoi(output);
}

inline static void rrex3_cmp_range(rrex3_t *rrex3) {
    char *loop_code = rrex3->previous.expr;
    char *expr_original = rrex3->expr;
    rrex3->expr++;
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
        if (rrex3->valid == false) {
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
    rrex3->valid = times_valid >= range_start;
    if (rrex3->valid && range_end) {
        rrex3->valid = times_valid <= range_end;
    }
    rrex3->expr = strchr(expr_original, '}') + 1;
}

inline static void rrex3_cmp_word_start_or_end(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    if (*rrex3->expr != 'B') {
        printf("Check word start or end: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
               rrex3->valid);
    }

#endif
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
inline static void rrex3_cmp_word_not_start_or_end(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    printf("Check word NOT start or end: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
           rrex3->valid);

#endif
    rrex3_set_previous(rrex3);

    rrex3_cmp_word_start_or_end(rrex3);
    rrex3->valid = !rrex3->valid;
}

inline static void rrex3_cmp_brackets(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    rprintb("\\l Brackets start: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
            rrex3->valid);
#endif
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
    while (*rrex3->expr != ']' && *rrex3->expr != 0)
        rrex3->expr++;
    if (*rrex3->expr != 0)
        rrex3->expr++;

    rrex3->valid = valid_once;
    rrex3->inside_brackets = false;
    char *previous_expr = rrex3->expr;
    rrex3->expr = original_expr;
    rrex3_set_previous(rrex3);
    rrex3->expr = previous_expr;
#if RREX3_DEBUG == 1
    rprintb("\\l Brackets end: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
            rrex3->valid);
#endif
}

inline static void rrex3_cmp_pipe(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

#if RREX3_DEBUG == 1
    printf("Pipe check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (rrex3->valid == true) {
        rrex3->exit = true;
    } else {
        rrex3->valid = true;
    }
    rrex3->expr++;
}
inline static void rrex3_cmp_parentheses(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    rprinty("\\l Parentheses start check: %c:%c:%d\n", *rrex3->expr,
            *rrex3->str, rrex3->valid);
#endif

    rrex3_set_previous(rrex3);
    if (!rrex3->valid) {
        rrex3->expr++;
        return;
    }
    if (rrex3->match_count == rrex3->match_capacity) {

        rrex3->match_capacity++;
        rrex3->matches = (char **)realloc(
            rrex3->matches, rrex3->match_capacity * sizeof(char *));
    }
    rrex3->matches[rrex3->match_count] = (char *)malloc(strlen(rrex3->str) + 1);
    strcpy(rrex3->matches[rrex3->match_count], rrex3->str);
    char *original_expr = rrex3->expr;
    char *original_str = rrex3->str;
    rrex3->expr++;
    rrex3->inside_parentheses = true;
    while (*rrex3->expr != ')' && !rrex3->exit) {
        rrex3_move(rrex3, false);
    }
    while (*rrex3->expr != ')') {
        rrex3->expr++;
    }
    rrex3->expr++;
    rrex3->inside_parentheses = false;

    char *previous_expr = rrex3->expr;
    rrex3->expr = original_expr;
    rrex3_set_previous(rrex3);
    rrex3->expr = previous_expr;
    if (rrex3->valid == false) {
        rrex3->str = original_str;
        free(rrex3->matches[rrex3->match_count]);
    } else {
        rrex3->matches[rrex3->match_count]
                      [strlen(rrex3->matches[rrex3->match_count]) -
                       strlen(rrex3->str)] = 0;
        rrex3->match_count++;
    }
#if RREX3_DEBUG == 1
    rprinty("\\l Parentheses end: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
            rrex3->valid);
#endif
}

inline static void rrex3_reset(rrex3_t *rrex3) {
    rrex3_free_matches(rrex3);
    rrex3->valid = true;
    rrex3->pattern_error = false;
    rrex3->inside_brackets = false;
    rrex3->inside_parentheses = false;
    rrex3->exit = false;
    rrex3->previous.expr = NULL;
    rrex3->previous.str = NULL;
    rrex3->previous.bytecode = 0;
    rrex3->failed.expr = NULL;
    rrex3->failed.str = NULL;
    rrex3->failed.bytecode = 0;
    rrex3->match_from_start = false;
}

void rrex3_init(rrex3_t *rrex3) {
    for (__uint8_t i = 0; i < 254; i++) {
        rrex3->functions[i] = rrex3_cmp_literal;
        rrex3->slash_functions[i] = rrex3_cmp_literal;
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
    rrex3->functions['('] = rrex3_cmp_parentheses;
    rrex3->slash_functions['w'] = rrex3_cmp_w;
    rrex3->slash_functions['W'] = rrex3_cmp_w_upper;
    rrex3->slash_functions['d'] = rrex3_cmp_d;
    rrex3->slash_functions['D'] = rrex3_cmp_d_upper;
    rrex3->slash_functions['s'] = rrex3_cmp_whitespace;
    rrex3->slash_functions['S'] = rrex3_cmp_whitespace_upper;
    rrex3->slash_functions['b'] = rrex3_cmp_word_start_or_end;
    rrex3->slash_functions['B'] = rrex3_cmp_word_not_start_or_end;
    rrex3->match_count = 0;
    rrex3->match_capacity = 0;
    rrex3->matches = NULL;
    rrex3->compiled = NULL;

    rrex3_reset(rrex3);
}

rrex3_t *rrex3_new() {
    rrex3_t *rrex3 = (rrex3_t *)malloc(sizeof(rrex3_t));

    rrex3_init(rrex3);

    return rrex3;
}

rrex3_t *rrex3_compile(rrex3_t *rrex, char *expr) {

    rrex3_t *rrex3 = rrex ? rrex : rrex3_new();

    char *compiled = (char *)malloc(strlen(expr) + 1);
    unsigned int count = 0;
    while (*expr) {
        if (*expr == '[' && *(expr + 2) == ']') {
            *compiled = *(expr + 1);
            expr++;
            expr++;
        } else if (*expr == '[' && *(expr + 1) == '0' && *(expr + 2) == '-' &&
                   *(expr + 3) == '9' && *(expr + 4) == ']') {
            *compiled = '\\';
            compiled++;
            *compiled = 'd';
            count++;
            expr++;
            expr++;
            expr++;
            expr++;
        } else {
            *compiled = *expr;
        }
        if (*compiled == '[') {
            // in_brackets = true;

        } else if (*compiled == ']') {
            // in_brackets = false;
        }
        expr++;
        compiled++;
        count++;
    }
    *compiled = 0;
    compiled -= count;
    rrex3->compiled = compiled;
    return rrex3;
}

inline static void rrex3_set_previous(rrex3_t *rrex3) {
    rrex3->previous.function = rrex3->function;
    rrex3->previous.expr = rrex3->expr;
    rrex3->previous.str = rrex3->str;
    rrex3->previous.bytecode = *rrex3->expr;
}

static bool rrex3_move(rrex3_t *rrex3, bool resume_on_fail) {
    char *original_expr = rrex3->expr;
    char *original_str = rrex3->str;
    rrex3->bytecode = *rrex3->expr;
    rrex3->function = rrex3->functions[(int)rrex3->bytecode];
    rrex3->function(rrex3);
    if (!*rrex3->expr && !*rrex3->str) {
        rrex3->exit = true;
        return rrex3->valid;
    } else if (!*rrex3->expr) {
        // rrex3->valid = true;
        return rrex3->valid;
    }
    if (rrex3->pattern_error) {
        rrex3->valid = false;
        return rrex3->valid;
    }
    if (resume_on_fail && !rrex3->valid && *rrex3->expr) {

        // rrex3_set_previous(rrex3);
        rrex3->failed.bytecode = rrex3->bytecode;
        rrex3->failed.function = rrex3->function;
        rrex3->failed.expr = original_expr;
        rrex3->failed.str = original_str;
        rrex3->bytecode = *rrex3->expr;
        rrex3->function = rrex3->functions[(int)rrex3->bytecode];
        rrex3->function(rrex3);

        if (!rrex3->valid && !rrex3->pattern_error) {

            if (*rrex3->str) {
                char *pipe_position = strstr(rrex3->expr, "|");
                if (pipe_position != NULL) {
                    rrex3->expr = pipe_position + 1;
                    rrex3->str = rrex3->_str;
                    rrex3->valid = true;
                    return true;
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
            if (*rrex3->str)
                rrex3->valid = true;
        }
    } else {
    }
    return rrex3->valid;
}

rrex3_t *rrex3(rrex3_t *rrex3, char *str, char *expr) {
#if RREX3_DEBUG == 1
    printf("Regex check: %s:%s:%d\n", expr, str, 1);
#endif
    bool self_initialized = false;
    if (rrex3 == NULL) {
        self_initialized = true;
        rrex3 = rrex3_new();
    } else {
        rrex3_reset(rrex3);
    }

    rrex3->_str = str;
    rrex3->_expr = rrex3->compiled ? rrex3->compiled : expr;
    rrex3->str = rrex3->_str;
    rrex3->expr = rrex3->_expr;
    while (*rrex3->expr && !rrex3->exit) {
        if (!rrex3_move(rrex3, true))
            return NULL;
    }
    rrex3->expr = rrex3->_expr;
    if (rrex3->valid) {

        return rrex3;
    } else {
        if (self_initialized) {
            rrex3_free(rrex3);
        }
        return NULL;
    }
}

void rrex3_test() {
    rrex3_t *rrex = rrex3_new();

    assert(rrex3(rrex, "\"stdio.h\"\"string.h\"\"sys/time.h\"",
                 "\"(.*)\"\"(.*)\"\"(.*)\""));

    assert(rrex3(rrex, "aaaaaaa", "a*a$"));

    // assert(rrex3("ababa", "a*b*a*b*a$"));
    assert(rrex3(rrex, "#include\"test.h\"a", "#include.*\".*\"a$"));
    assert(rrex3(rrex, "#include \"test.h\"a", "#include.*\".*\"a$"));
    assert(rrex3(rrex, "aaaaaad", "a*d$"));
    assert(rrex3(rrex, "abcdef", "abd?cdef"));
    assert(!rrex3(rrex, "abcdef", "abd?def"));
    assert(rrex3(rrex, "abcdef", "def"));
    assert(!rrex3(rrex, "abcdef", "^def"));
    assert(rrex3(rrex, "abcdef", "def$"));
    assert(!rrex3(rrex, "abcdef", "^abc$"));
    assert(rrex3(rrex, "aB!.#1", "......"));
    assert(!rrex3(rrex, "aB!.#\n", "      ......"));
    assert(!rrex3(rrex, "aaaaaad", "q+d$"));
    assert(rrex3(rrex, "aaaaaaa", "a+a$"));
    assert(rrex3(rrex, "aaaaaad", "q*d$"));
    assert(!rrex3(rrex, "aaaaaad", "^q*d$"));

    // Asterisk function
    assert(rrex3(rrex, "123321", "123*321"));
    assert(rrex3(rrex, "pony", "p*ony"));
    assert(rrex3(rrex, "pppony", "p*ony"));
    assert(rrex3(rrex, "ppony", "p*pony"));
    assert(rrex3(rrex, "pppony", "pp*pony"));
    assert(rrex3(rrex, "pppony", ".*pony"));
    assert(rrex3(rrex, "pony", ".*ony"));
    assert(rrex3(rrex, "pony", "po*ny"));
    // assert(rrex3(rrex,"ppppony", "p*pppony"));

    // Plus function
    assert(rrex3(rrex, "pony", "p+ony"));
    assert(!rrex3(rrex, "ony", "p+ony"));
    assert(rrex3(rrex, "ppony", "p+pony"));
    assert(rrex3(rrex, "pppony", "pp+pony"));
    assert(rrex3(rrex, "pppony", ".+pony"));
    assert(rrex3(rrex, "pony", ".+ony"));
    assert(rrex3(rrex, "pony", "po+ny"));

    // Slash functions
    assert(rrex3(rrex, "a", "\\w"));
    assert(!rrex3(rrex, "1", "\\w"));
    assert(rrex3(rrex, "1", "\\W"));
    assert(!rrex3(rrex, "a", "\\W"));
    assert(rrex3(rrex, "a", "\\S"));
    assert(!rrex3(rrex, " ", "\\s"));
    assert(!rrex3(rrex, "\t", "\\s"));
    assert(!rrex3(rrex, "\n", "\\s"));
    assert(rrex3(rrex, "1", "\\d"));
    assert(!rrex3(rrex, "a", "\\d"));
    assert(rrex3(rrex, "a", "\\D"));
    assert(!rrex3(rrex, "1", "\\D"));
    assert(rrex3(rrex, "abc", "\\b"));

    assert(rrex3(rrex, "abc", "\\babc"));
    assert(!rrex3(rrex, "abc", "a\\b"));
    assert(!rrex3(rrex, "abc", "ab\\b"));
    assert(!rrex3(rrex, "abc", "abc\\b"));
    assert(rrex3(rrex, "abc", "a\\Bbc"));
    assert(rrex3(rrex, "abc", "ab\\B"));
    assert(!rrex3(rrex, "1ab", "1\\Bab"));
    assert(rrex3(rrex, "abc", "a\\Bbc"));

    // Escaping of special chars
    assert(rrex3(rrex, "()+*.\\", "\\(\\)\\+\\*\\.\\\\"));

    // Pipe
    // assert(rrex3(rrex,"abc","abc|def"));
    assert(rrex3(rrex, "abc", "def|jkl|abc"));
    assert(rrex3(rrex, "abc", "abc|def"));

    assert(rrex3(rrex, "rhq", "def|rhq|rha"));
    assert(rrex3(rrex, "abc", "abc|def"));

    // Repeat
    assert(rrex3(rrex, "aaaaa", "a{4}"));

    assert(rrex3(rrex, "aaaa", "a{1,3}a"));

    // Range
    assert(rrex3(rrex, "abc", "[abc][abc][abc]$"));
    assert(rrex3(rrex, "def", "[^abc][^abc][^abc]$"));
    assert(rrex3(rrex, "defabc", "[^abc][^abc][^abc]abc"));
    assert(rrex3(rrex, "0-9", "0-9"));
    assert(rrex3(rrex, "55-9", "[^6-9]5-9$"));
    assert(rrex3(rrex, "a", "[a-z]$"));
    assert(rrex3(rrex, "A", "[A-Z]$"));
    assert(rrex3(rrex, "5", "[0-9]$"));
    assert(!rrex3(rrex, "a", "[^a-z]$"));
    assert(!rrex3(rrex, "A", "[^A-Z]$"));
    assert(!rrex3(rrex, "5", "[^0-9]$"));
    assert(rrex3(rrex, "123abc", "[0-9]*abc$"));
    assert(rrex3(rrex, "123123", "[0-9]*$"));

    // Parentheses

    assert(rrex3(rrex, "datadata", "(data)*"));

    assert(rrex3(rrex, "datadatapony", "(data)*pony$"));

    assert(!rrex3(rrex, "datadatapony", "(d*p*ata)*pond$"));
    assert(rrex3(rrex, "datadatadato", "(d*p*ata)*dato"));
    assert(rrex3(rrex, "datadatadato", "(d*p*ata)*dato$"));
    assert(!rrex3(rrex, "datadatadato", "(d*p*a*ta)*gato$"));

    // Matches
    assert(rrex3(rrex, "123", "(123)"));
    assert(!strcmp(rrex->matches[0], "123"));

    assert(rrex3(rrex, "123321a", "(123)([0-4][2]1)a$"));
    assert(!strcmp(rrex->matches[1], "321"));

    assert(rrex3(rrex, "123321a", "(123)([0-4][2]1)a$"));
    assert(!strcmp(rrex->matches[1], "321"));

    assert(rrex3(rrex, "aaaabc", "(.*)c"));

    assert(rrex3(rrex, "abcde", ".....$"));

    assert(rrex3(rrex, "abcdefghijklmnopqrstuvwxyz",
                 "..........................$"));
    // printf("(%d)\n", rrex->valid);

    assert(rrex3(rrex, "#include <stdio.h>", "#include.*<(.*)>"));
    assert(!strcmp(rrex->matches[0], "stdio.h"));
    assert(rrex3(rrex, "#include \"stdlib.h\"", "#include.\"(.*)\""));
    assert(!strcmp(rrex->matches[0], "stdlib.h"));
    assert(rrex3(rrex, "\"stdio.h\"\"string.h\"\"sys/time.h\"",
                 "\"(.*)\"\"(.*)\"\"(.*)\""));
    assert(!strcmp(rrex->matches[0], "stdio.h"));
    assert(!strcmp(rrex->matches[1], "string.h"));
    assert(!strcmp(rrex->matches[2], "sys/time.h"));

    assert(rrex3(rrex, "    #include <stdio.h>", "#include.+<(.+)>"));
    assert(!strcmp(rrex->matches[0], "stdio.h"));
    assert(rrex3(rrex, "    #include \"stdlib.h\"", "#include.+\"(.+)\""));
    assert(!strcmp(rrex->matches[0], "stdlib.h"));

    assert(rrex3(rrex, "    \"stdio.h\"\"string.h\"\"sys/time.h\"",
                 "\"(.+)\"\"(.+)\"\"(.+)\""));
    assert(!strcmp(rrex->matches[0], "stdio.h"));
    assert(!strcmp(rrex->matches[1], "string.h"));
    assert(!strcmp(rrex->matches[2], "sys/time.h"));

    assert(rrex3(rrex, "int abc ", "int (.*)[; ]?$"));
    assert(!strcmp(rrex->matches[0], "abc"));
    assert(rrex3(rrex, "int abc;", "int (.*)[; ]?$"));
    assert(!strcmp(rrex->matches[0], "abc"));
    assert(rrex3(rrex, "int abc", "int (.*)[; ]?$"));
    assert(!strcmp(rrex->matches[0], "abc"));

    assert(rrex3(rrex, "#define abc", "#define (.*)"));
    assert(!strcmp(rrex->matches[0], "abc"));
    assert(rrex3(rrex, "#define abc", "#define (.*)$"));
    assert(!strcmp(rrex->matches[0], "abc"));
    assert(rrex3(rrex, "#define abc 1", "#define (.*) (.*)$"));
    assert(!strcmp(rrex->matches[0], "abc"));
    assert(!strcmp(rrex->matches[1], "1"));

    assert(rrex3(rrex, "#define abc 1  ", "#define (.*) (.*) *$"));
    assert(!strcmp(rrex->matches[0], "abc"));
    printf("<<%s>>\n", rrex->matches[1]);
    assert(!strcmp(rrex->matches[1], "1"));

    assert(rrex3(rrex, "#define abc \"test with spaces\"  ", "#define (.*) *\"(.*)\" *$"));
    assert(!strcmp(rrex->matches[0], "abc"));
    printf("<<%s>>\n", rrex->matches[1]);
    assert(!strcmp(rrex->matches[1], "test with spaces"));

    rrex3_free(rrex);
}
#endif
// RETOOR - Sep  3 2024
// MIT License
// ===========

// Copyright (c) 2024 Retoor

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#ifndef RLIB_H
#define RLIB_H
// BEGIN OF RLIB
#ifndef RPRINT_H
#define RPRINT_H

#ifndef RLIB_TIME
#define RLIB_TIME

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

typedef unsigned long long msecs_t;
typedef uint64_t nsecs_t;

nsecs_t nsecs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000000LL + (int64_t)ts.tv_nsec;
}

msecs_t rnsecs_to_msecs(nsecs_t nsecs) { return nsecs / 1000 / 1000; }

nsecs_t rmsecs_to_nsecs(msecs_t msecs) { return msecs * 1000 * 1000; }

msecs_t usecs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)(tv.tv_sec) * 1000000 + (long long)(tv.tv_usec);
}

msecs_t msecs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)(tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
}
char *msecs_strs(msecs_t ms) {
    static char str[22];
    str[0] = 0;
    sprintf(str, "%f", ms * 0.001);
    for (int i = strlen(str); i > 0; i--) {
        if (str[i] > '0')
            break;
        str[i] = 0;
    }
    return str;
}
char *msecs_strms(msecs_t ms) {
    static char str[22];
    str[0] = 0;
    sprintf(str, "%lld", ms);
    return str;
}
char *msecs_str(long long ms) {
    static char result[30];
    result[0] = 0;
    if (ms > 999) {
        char *s = msecs_strs(ms);
        sprintf(result, "%ss", s);
    } else {
        char *s = msecs_strms(ms);
        sprintf(result, "%sMs", s);
    }
    return result;
}

void nsleep(nsecs_t nanoseconds) {
    long seconds = 0;
    int factor = 0;
    while (nanoseconds > 1000000000) {
        factor++;
        nanoseconds = nanoseconds / 10;
    }
    if (factor) {
        seconds = 1;
        factor--;
        while (factor) {
            seconds = seconds * 10;
            factor--;
        }
    }

    struct timespec req = {seconds, nanoseconds};
    struct timespec rem;

    if (nanosleep(&req, &rem) == -1) {
        if (errno == EINTR) {
            printf("Sleep was interrupted. Remaining time: %ld.%09ld seconds\n",
                   rem.tv_sec, rem.tv_nsec);
        } else {
            perror("nanosleep");
        }
    } else {
        // printf("Slept for %ld.%09ld seconds\n", req.tv_sec, req.tv_nsec);
    }
}

void ssleep(double s) {
    long nanoseconds = (long)(1000000000 * s);

    long seconds = 0;

    struct timespec req = {seconds, nanoseconds};
    struct timespec rem;

    if (nanosleep(&req, &rem) == -1) {
        if (errno == EINTR) {
            printf("Sleep was interrupted. Remaining time: %ld.%09ld seconds\n",
                   rem.tv_sec, rem.tv_nsec);
        } else {
            perror("nanosleep");
        }
    } else {
        // printf("Slept for %ld.%09ld seconds\n", req.tv_sec, req.tv_nsec);
    }
}
void msleep(long miliseonds) {
    long nanoseconds = miliseonds * 1000000;
    nsleep(nanoseconds);
}

char *format_time(int64_t nanoseconds) {
    static char output[1024];
    size_t output_size = sizeof(output);
    output[0] = 0;
    if (nanoseconds < 1000) {
        // Less than 1 microsecond
        snprintf(output, output_size, "%ldns", nanoseconds);
    } else if (nanoseconds < 1000000) {
        // Less than 1 millisecond
        double us = nanoseconds / 1000.0;
        snprintf(output, output_size, "%.2fµs", us);
    } else if (nanoseconds < 1000000000) {
        // Less than 1 second
        double ms = nanoseconds / 1000000.0;
        snprintf(output, output_size, "%.2fms", ms);
    } else {
        // 1 second or more
        double s = nanoseconds / 1000000000.0;
        snprintf(output, output_size, "%.2fs", s);
    }
    return output;
}

#endif
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

long rpline_number = 0;
nsecs_t rprtime = 0;

int8_t _env_rdisable_colors = -1;
bool _rprint_enable_colors = true;

bool rprint_is_color_enabled() {
    if (_env_rdisable_colors == -1) {
        _env_rdisable_colors = getenv("RDISABLE_COLORS") != NULL;
    }
    if (_env_rdisable_colors) {
        _rprint_enable_colors = false;
    }
    return _rprint_enable_colors;
}

void rprint_disable_colors() { _rprint_enable_colors = false; }
void rprint_enable_colors() { _rprint_enable_colors = true; }
void rprint_toggle_colors() { _rprint_enable_colors = !_rprint_enable_colors; }

void rclear() { printf("\033[2J"); }

void rprintpf(FILE *f, const char *prefix, const char *format, va_list args) {
    char *pprefix = (char *)prefix;
    char *pformat = (char *)format;
    bool reset_color = false;
    bool press_any_key = false;
    char new_format[4096];
    bool enable_color = rprint_is_color_enabled();
    memset(new_format, 0, 4096);
    int new_format_length = 0;
    char temp[1000];
    memset(temp, 0, 1000);
    if (enable_color && pprefix[0]) {
        strcat(new_format, pprefix);
        new_format_length += strlen(pprefix);
        reset_color = true;
    }
    while (true) {
        if (pformat[0] == '\\' && pformat[1] == 'i') {
            strcat(new_format, "\e[3m");
            new_format_length += strlen("\e[3m");
            reset_color = true;
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 'u') {
            strcat(new_format, "\e[4m");
            new_format_length += strlen("\e[4m");
            reset_color = true;
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 'b') {
            strcat(new_format, "\e[1m");
            new_format_length += strlen("\e[1m");
            reset_color = true;
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 'C') {
            press_any_key = true;
            rpline_number++;
            pformat++;
            pformat++;
            reset_color = false;
        } else if (pformat[0] == '\\' && pformat[1] == 'k') {
            press_any_key = true;
            rpline_number++;
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 'c') {
            rpline_number++;
            strcat(new_format, "\e[2J\e[H");
            new_format_length += strlen("\e[2J\e[H");
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 'L') {
            rpline_number++;
            temp[0] = 0;
            sprintf(temp, "%ld", rpline_number);
            strcat(new_format, temp);
            new_format_length += strlen(temp);
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 'l') {
            rpline_number++;
            temp[0] = 0;
            sprintf(temp, "%.5ld", rpline_number);
            strcat(new_format, temp);
            new_format_length += strlen(temp);
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 'T') {
            nsecs_t nsecs_now = nsecs();
            nsecs_t end = rprtime ? nsecs_now - rprtime : 0;
            temp[0] = 0;
            sprintf(temp, "%s", format_time(end));
            strcat(new_format, temp);
            new_format_length += strlen(temp);
            rprtime = nsecs_now;
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 't') {
            rprtime = nsecs();
            pformat++;
            pformat++;
        } else {
            new_format[new_format_length] = *pformat;
            new_format_length++;
            if (!*pformat)
                break;

            // printf("%c",*pformat);
            pformat++;
        }
    }
    if (reset_color) {
        strcat(new_format, "\e[0m");
        new_format_length += strlen("\e[0m");
    }

    new_format[new_format_length] = 0;
    vfprintf(f, new_format, args);

    fflush(stdout);
    if (press_any_key) {
        nsecs_t s = nsecs();
        fgetc(stdin);
        rprtime += nsecs() - s;
    }
}

void rprintp(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(stdout, "", format, args);
    va_end(args);
}

void rprintf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "", format, args);
    va_end(args);
}
void rprint(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(stdout, "", format, args);
    va_end(args);
}
#define printf rprint

// Print line
void rprintlf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\\l", format, args);
    va_end(args);
}
void rprintl(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(stdout, "\\l", format, args);
    va_end(args);
}

// Black
void rprintkf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[30m", format, args);
    va_end(args);
}
void rprintk(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(stdout, "\e[30m", format, args);
    va_end(args);
}

// Red
void rprintrf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[31m", format, args);
    va_end(args);
}
void rprintr(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(stdout, "\e[31m", format, args);
    va_end(args);
}

// Green
void rprintgf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[32m", format, args);
    va_end(args);
}
void rprintg(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(stdout, "\e[32m", format, args);
    va_end(args);
}

// Yellow
void rprintyf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[33m", format, args);
    va_end(args);
}
void rprinty(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(stdout, "\e[33m", format, args);
    va_end(args);
}

// Blue
void rprintbf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[34m", format, args);
    va_end(args);
}

void rprintb(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(stdout, "\e[34m", format, args);
    va_end(args);
}

// Magenta
void rprintmf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[35m", format, args);
    va_end(args);
}
void rprintm(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(stdout, "\e[35m", format, args);
    va_end(args);
}

// Cyan
void rprintcf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[36m", format, args);
    va_end(args);
}
void rprintc(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(stdout, "\e[36m", format, args);
    va_end(args);
}

// White
void rprintwf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[37m", format, args);
    va_end(args);
}
void rprintw(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(stdout, "\e[37m", format, args);
    va_end(args);
}
#endif
#ifndef RMATH_H
#define RMATH_H
#include <math.h>

#ifndef ceil
double ceil(double x) {
    if (x == (double)(long long)x) {
        return x;
    } else if (x > 0.0) {
        return (double)(long long)x + 1.0;
    } else {
        return (double)(long long)x;
    }
}
#endif

#ifndef floor
double floor(double x) {
    if (x >= 0.0) {
        return (double)(long long)x;
    } else {
        double result = (double)(long long)x;
        return (result == x) ? result : result - 1.0;
    }
}
#endif

#ifndef modf
double modf(double x, double *iptr) {
    double int_part = (x >= 0.0) ? floor(x) : ceil(x);
    *iptr = int_part;
    return x - int_part;
}
#endif
#endif
#ifndef RMALLOC_H
#define RMALLOC_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long long rmalloc_count = 0;
unsigned long long rmalloc_alloc_count = 0;
unsigned long long int rmalloc_free_count = 0;

void *rmalloc(size_t size) {
    rmalloc_count++;
    rmalloc_alloc_count++;
    return malloc(size);
}
void *rrealloc(void *obj, size_t size) {
    if (obj == NULL) {
        rmalloc_count++;
        rmalloc_alloc_count++;
    }
    return realloc(obj, size);
}
void *rfree(void *obj) {
    rmalloc_count--;
    rmalloc_free_count++;
    free(obj);
    return NULL;
}

#define malloc rmalloc
#define realloc rrealloc
#define free rfree

char *rmalloc_stats() {
    static char res[100] = {0};
    sprintf(res, "Memory usage: %lld allocated, %lld freed, %lld in use.",
            rmalloc_alloc_count, rmalloc_free_count, rmalloc_count);
    return res;
}

char *rstrdup(char *str) {

    char *res = (char *)strdup(str);
    rmalloc_alloc_count++;
    rmalloc_count++;
    return res;
}

#endif

#ifndef RTEST_H
#define RTEST_H
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#define debug(fmt, ...) printf("%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__);

char *rcurrent_banner;
int rassert_count = 0;
unsigned short rtest_is_first = 1;
unsigned int rtest_fail_count = 0;

int rtest_end(char *content) {
    // Returns application exit code. 0 == success
    printf("%s", content);
    printf("\n@assertions: %d\n", rassert_count);
    printf("@memory: %s\n", rmalloc_stats());

    if (rmalloc_count != 0) {
        printf("MEMORY ERROR\n");
        return rtest_fail_count > 0;
    }
    return rtest_fail_count > 0;
}

void rtest_test_banner(char *content, char *file) {
    if (rtest_is_first == 1) {
        char delimiter[] = ".";
        char *d = delimiter;
        char f[2048];
        strcpy(f, file);
        printf("%s tests", strtok(f, d));
        rtest_is_first = 0;
        setvbuf(stdout, NULL, _IONBF, 0);
    }
    printf("\n - %s ", content);
}

bool rtest_test_true_silent(char *expr, int res, int line) {
    rassert_count++;
    if (res) {
        return true;
    }
    rprintrf(stderr, "\nERROR on line %d: %s", line, expr);
    rtest_fail_count++;
    return false;
}

bool rtest_test_true(char *expr, int res, int line) {
    rassert_count++;
    if (res) {
        fprintf(stdout, ".");
        return true;
    }
    rprintrf(stderr, "\nERROR on line %d: %s", line, expr);
    rtest_fail_count++;
    return false;
}
bool rtest_test_false_silent(char *expr, int res, int line) {
    return rtest_test_true_silent(expr, !res, line);
}
bool rtest_test_false(char *expr, int res, int line) {
    return rtest_test_true(expr, !res, line);
}
void rtest_test_skip(char *expr, int line) {
    rprintgf(stderr, "\n @skip(%s) on line %d\n", expr, line);
}
bool rtest_test_assert(char *expr, int res, int line) {
    if (rtest_test_true(expr, res, line)) {
        return true;
    }
    rtest_end("");
    exit(40);
}

#define rtest_banner(content)                                                  \
    rcurrent_banner = content;                                                 \
    rtest_test_banner(content, __FILE__);
#define rtest_true(expr) rtest_test_true(#expr, expr, __LINE__);
#define rtest_assert(expr)                                                     \
    {                                                                          \
        int __valid = expr ? 1 : 0;                                            \
        rtest_test_true(#expr, __valid, __LINE__);                             \
    };                                                                         \
    ;

#define rassert(expr)                                                          \
    {                                                                          \
        int __valid = expr ? 1 : 0;                                            \
        rtest_test_true(#expr, __valid, __LINE__);                             \
    };                                                                         \
    ;
#define rtest_asserts(expr)                                                    \
    {                                                                          \
        int __valid = expr ? 1 : 0;                                            \
        rtest_test_true_silent(#expr, __valid, __LINE__);                      \
    };
#define rasserts(expr)                                                         \
    {                                                                          \
        int __valid = expr ? 1 : 0;                                            \
        rtest_test_true_silent(#expr, __valid, __LINE__);                      \
    };
#define rtest_false(expr)                                                      \
    rprintf(" [%s]\t%s\t\n", expr == 0 ? "OK" : "NOK", #expr);                 \
    assert_count++;                                                            \
    assert(#expr);
#define rtest_skip(expr) rtest_test_skip(#expr, __LINE__);

FILE *rtest_create_file(char *path, char *content) {
    FILE *fd = fopen(path, "wb");

    char c;
    int index = 0;

    while ((c = content[index]) != 0) {
        fputc(c, fd);
        index++;
    }
    fclose(fd);
    fd = fopen(path, "rb");
    return fd;
}

void rtest_delete_file(char *path) { unlink(path); }
#endif
#ifndef RREX3_H
#define RREX3_H
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef RREX3_DEBUG
#define RREX3_DEBUG 0
#endif

struct rrex3_t;

typedef void (*rrex3_function)(struct rrex3_t *);

typedef struct rrex3_t {
    void (*functions[254])(struct rrex3_t *);
    void (*slash_functions[254])(struct rrex3_t *);
    bool valid;
    int match_count;
    int match_capacity;
    char **matches;
    bool exit;
    char *__expr;
    char *__str;
    char *_expr;
    char *_str;
    char *expr;
    char *str;
    char *compiled;
    bool inside_brackets;
    bool inside_parentheses;
    bool pattern_error;
    bool match_from_start;
    char bytecode;
    rrex3_function function;
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

static bool isdigitrange(char *s) {
    if (!isdigit(*s)) {
        return false;
    }
    if (*(s + 1) != '-') {
        return false;
    }
    return isdigit(*(s + 2));
}

static bool isalpharange(char *s) {
    if (!isalpha(*s)) {
        return false;
    }
    if (*(s + 1) != '-') {
        return false;
    }
    return isalpha(*(s + 2));
}

void rrex3_free_matches(rrex3_t *rrex3) {
    if (!rrex3->matches)
        return;
    for (int i = 0; i < rrex3->match_count; i++) {
        free(rrex3->matches[i]);
    }
    free(rrex3->matches);
    rrex3->matches = NULL;
    rrex3->match_count = 0;
    rrex3->match_capacity = 0;
}

void rrex3_free(rrex3_t *rrex3) {
    if (!rrex3)
        return;
    if (rrex3->compiled) {
        free(rrex3->compiled);
        rrex3->compiled = NULL;
    }
    rrex3_free_matches(rrex3);
    free(rrex3);
    rrex3 = NULL;
}
static bool rrex3_move(rrex3_t *, bool);
static void rrex3_set_previous(rrex3_t *);
inline static void rrex3_cmp_asterisk(rrex3_t *);
void rrex3_cmp_literal_range(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    printf("Range check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    rrex3_set_previous(rrex3);

    char start = *rrex3->expr;
    rrex3->expr++;
    rrex3->expr++;
    char end = *rrex3->expr;
    if (*rrex3->str >= start && *rrex3->str <= end) {
        rrex3->str++;
        rrex3->valid = true;
    } else {
        rrex3->valid = false;
    }
    rrex3->expr++;
}

bool rrex3_is_function(char chr) {
    if (chr == ']' || chr == ')' || chr == '\\' || chr == '?' || chr == '+' ||
        chr == '*')
        return true;
    return false;
}

inline static void rrex3_cmp_literal(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);
    if (*rrex3->expr == 0 && !*rrex3->str) {
        printf("ERROR, EMPTY CHECK");
        exit(1);
    }
    if (rrex3->valid == false) {
        rrex3->expr++;
        return;
    }
    if (rrex3->inside_brackets) {
        if (isalpharange(rrex3->expr) || isdigitrange(rrex3->expr)) {
            rrex3_cmp_literal_range(rrex3);
            return;
        }
    }
#if RREX3_DEBUG == 1
    printf("Literal check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
           rrex3->valid);
#endif
    if (*rrex3->expr == *rrex3->str) {
        rrex3->expr++;
        rrex3->str++;
        rrex3->valid = true;
        // if(*rrex3->expr &&rrex3->functions[(int)*rrex3->expr] ==
        // rrex3_cmp_literal && !rrex3->inside_brackets &&
        //! rrex3_is_function(*rrex3->expr)){ rrex3_cmp_literal(rrex3);
        //   if(rrex3->valid == false){
        //  rrex3->expr--;
        // rrex3->valid = true;
        // }
        // }
        return;
    }
    rrex3->expr++;
    rrex3->valid = false;
}

inline static void rrex3_cmp_dot(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    printf("Dot check (any char): %c:%c:%d\n", *rrex3->expr, *rrex3->str,
           rrex3->valid);
#endif
    rrex3_set_previous(rrex3);
    rrex3->expr++;
    if (!rrex3->valid) {
        return;
    }
    if (*rrex3->str && *rrex3->str != '\n') {
        rrex3->str++;
        if (*rrex3->expr && *rrex3->expr == '.') {
            rrex3_cmp_dot(rrex3);
            return;
        } /*else if(*rrex3->expr && (*rrex3->expr == '*' || *rrex3->expr ==
         '+')){ char * next = strchr(rrex3->str,*(rrex3->expr + 1)); char *
         space = strchr(rrex3->str,'\n'); if(next && (!space || space > next)){
                 rrex3->str = next;
             }
         }*/
    } else {
        rrex3->valid = false;
    }
}

inline static void rrex3_cmp_question_mark(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    printf("Question mark check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
           rrex3->valid);
#endif
    rrex3_set_previous(rrex3);

    if (rrex3->valid == false)
        rrex3->valid = true;
    rrex3->expr++;
}

inline static void rrex3_cmp_whitespace(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
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

inline static void rrex3_cmp_whitespace_upper(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
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

inline static void rrex3_cmp_plus(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
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
    char *next_str = rrex3->str;
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
        if (!*rrex3->str || !rrex3_move(rrex3, false)) {
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

inline static void rrex3_cmp_asterisk(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    rprintg("Asterisk start check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
            rrex3->valid);
#endif
    if (!rrex3->valid) {
        rrex3->valid = true;
        rrex3->expr++;
        return;
    }
    if (*rrex3->previous.expr == '*') {
        // Support for **
        rrex3->valid = false;
        // rrex3->pattern_error = true;
        rrex3->expr++;
        return;
    }
    rrex3->str = rrex3->previous.str;
    ;
    char *next = rrex3->expr + 1;
    char *next_original = NULL;
    if (*next == '*') {
        next++;
    }
    if (*next == ')' && *(next + 1)) {
        next_original = next;
        next++;
    }
    char *loop_expr = rrex3->previous.expr;
    bool success_next = false;
    bool success_next_once = false;
    bool success_current = false;
    char *right_next = NULL;
    char *right_str = rrex3->str;
    while (*rrex3->str && *rrex3->expr && *rrex3->expr != ')') {
        // Remember original_str because it's modified
        // by checking right and should be restored
        // for checking left so they're matching the
        // same value.
        char *original_str = rrex3->str;
        // Check if right matches.
        // if(*next != ')'){
        rrex3->expr = next;
        rrex3->valid = true;
        if (rrex3_move(rrex3, false)) {
            // Match rright.
            success_next = true;
            if (!next_original) {
                right_next = rrex3->expr;
            } else {
                right_next = next_original;
                break;
            }
            right_str = rrex3->str;
            success_next_once = true;
        } else {
            // No match Right.
            success_next = false;
        }
        //}
        if (success_next_once && !success_next) {
            // Matched previous time but now doesn't.
            break;
        }
        // Check if left matches.
        rrex3->str = original_str;
        rrex3->expr = loop_expr;
        rrex3->valid = true;
        if (!rrex3_move(rrex3, false)) {
            // No match left.
            success_current = false;
        } else {
            // Match left.
            success_current = true;
            // NOT SURE< WITHOUT DOET HETZELFDE:
            // original_str = rrex3->str;
            if (!success_next) {
                right_str = rrex3->str;
                if (*rrex3->expr != ')') {
                    right_next = rrex3->expr + 1; // +1 is the * itself

                } else {

                    // break;
                }
            }
        }

        if ((success_next && !success_current) ||
            (!success_next && !success_current)) {
            break;
        }
    }
    rrex3->expr = right_next;
    rrex3->str = right_str;
    rrex3->valid = true;
#if RREX3_DEBUG == 1
    rprintg("Asterisk end check: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
            rrex3->valid);
#endif
}

inline static void rrex3_cmp_roof(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);
#if RREX3_DEBUG == 1
    printf("<Roof check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    rrex3->valid = rrex3->str == rrex3->_str;
    rrex3->match_from_start = true;
    rrex3->expr++;
}
inline static void rrex3_cmp_dollar(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

#if RREX3_DEBUG == 1
    printf("Dollar check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (*rrex3->str || !rrex3->valid) {
        rrex3->valid = false;
    }
    rrex3->expr++;
}

inline static void rrex3_cmp_w(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    rrex3->expr++;
#if RREX3_DEBUG == 1
    printf("Word check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (isalpha(*rrex3->str)) {
        rrex3->str++;
    } else {
        rrex3->valid = false;
    }
}
inline static void rrex3_cmp_w_upper(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    rrex3->expr++;
#if RREX3_DEBUG == 1
    printf("!Word check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (!isalpha(*rrex3->str)) {
        rrex3->str++;
    } else {
        rrex3->valid = false;
    }
}

inline static void rrex3_cmp_d(rrex3_t *rrex3) {

    rrex3_set_previous(rrex3);

    rrex3->expr++;
#if RREX3_DEBUG == 1
    printf("Digit check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (isdigit(*rrex3->str)) {
        rrex3->str++;
    } else {
        rrex3->valid = false;
    }
}
inline static void rrex3_cmp_d_upper(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    rrex3->expr++;
#if RREX3_DEBUG == 1
    printf("!Digit check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (!isdigit(*rrex3->str)) {
        rrex3->str++;
    } else {
        rrex3->valid = false;
    }
}

inline static void rrex3_cmp_slash(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    rrex3->expr++;

    rrex3->bytecode = *rrex3->expr;
    rrex3->function = rrex3->slash_functions[(int)rrex3->bytecode];
    rrex3->function(rrex3);
}

inline static int collect_digits(rrex3_t *rrex3) {
    char output[20];
    unsigned int digit_count = 0;
    while (isdigit(*rrex3->expr)) {

        output[digit_count] = *rrex3->expr;
        rrex3->expr++;
        digit_count++;
    }
    output[digit_count] = 0;
    return atoi(output);
}

inline static void rrex3_cmp_range(rrex3_t *rrex3) {
    char *loop_code = rrex3->previous.expr;
    char *expr_original = rrex3->expr;
    rrex3->expr++;
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
        if (rrex3->valid == false) {
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
    rrex3->valid = times_valid >= range_start;
    if (rrex3->valid && range_end) {
        rrex3->valid = times_valid <= range_end;
    }
    rrex3->expr = strchr(expr_original, '}') + 1;
}

inline static void rrex3_cmp_word_start_or_end(rrex3_t *rrex3) {
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
inline static void rrex3_cmp_word_not_start_or_end(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

    rrex3_cmp_word_start_or_end(rrex3);
    rrex3->valid = !rrex3->valid;
}

inline static void rrex3_cmp_brackets(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    rprintb("\\l Brackets start: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
            rrex3->valid);
#endif
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
    while (*rrex3->expr != ']' && *rrex3->expr != 0)
        rrex3->expr++;
    if (*rrex3->expr != 0)
        rrex3->expr++;

    rrex3->valid = valid_once;
    rrex3->inside_brackets = false;
    char *previous_expr = rrex3->expr;
    rrex3->expr = original_expr;
    rrex3_set_previous(rrex3);
    rrex3->expr = previous_expr;
#if RREX3_DEBUG == 1
    rprintb("\\l Brackets end: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
            rrex3->valid);
#endif
}

inline static void rrex3_cmp_pipe(rrex3_t *rrex3) {
    rrex3_set_previous(rrex3);

#if RREX3_DEBUG == 1
    printf("Pipe check: %c:%c:%d\n", *rrex3->expr, *rrex3->str, rrex3->valid);
#endif
    if (rrex3->valid == true) {
        rrex3->exit = true;
    } else {
        rrex3->valid = true;
    }
    rrex3->expr++;
}
inline static void rrex3_cmp_parentheses(rrex3_t *rrex3) {
#if RREX3_DEBUG == 1
    rprinty("\\l Parentheses start check: %c:%c:%d\n", *rrex3->expr,
            *rrex3->str, rrex3->valid);
#endif
    if (!rrex3->valid) {
        rrex3->expr++;
        return;
    }
    rrex3_set_previous(rrex3);
    if (rrex3->match_count == rrex3->match_capacity) {

        rrex3->match_capacity++;
        rrex3->matches = (char **)realloc(
            rrex3->matches, rrex3->match_capacity * sizeof(char *));
    }
    rrex3->matches[rrex3->match_count] = (char *)malloc(strlen(rrex3->str) + 1);
    strcpy(rrex3->matches[rrex3->match_count], rrex3->str);
    char *original_expr = rrex3->expr;
    char *original_str = rrex3->str;
    rrex3->expr++;
    rrex3->inside_parentheses = true;
    while (*rrex3->expr != ')' && !rrex3->exit) {
        rrex3_move(rrex3, false);
    }
    while (*rrex3->expr != ')') {
        rrex3->expr++;
    }
    rrex3->expr++;
    rrex3->inside_parentheses = false;

    char *previous_expr = rrex3->expr;
    rrex3->expr = original_expr;
    rrex3_set_previous(rrex3);
    rrex3->expr = previous_expr;
    if (rrex3->valid == false) {
        rrex3->str = original_str;
        free(rrex3->matches[rrex3->match_count]);
    } else {
        rrex3->matches[rrex3->match_count]
                      [strlen(rrex3->matches[rrex3->match_count]) -
                       strlen(rrex3->str)] = 0;
        rrex3->match_count++;
    }
#if RREX3_DEBUG == 1
    rprinty("\\l Parentheses end: %c:%c:%d\n", *rrex3->expr, *rrex3->str,
            rrex3->valid);
#endif
}

inline static void rrex3_reset(rrex3_t *rrex3) {
    rrex3_free_matches(rrex3);
    rrex3->valid = true;
    rrex3->pattern_error = false;
    rrex3->inside_brackets = false;
    rrex3->inside_parentheses = false;
    rrex3->exit = false;
    rrex3->previous.expr = NULL;
    rrex3->previous.str = NULL;
    rrex3->previous.bytecode = 0;
    rrex3->failed.expr = NULL;
    rrex3->failed.str = NULL;
    rrex3->failed.bytecode = 0;
    rrex3->match_from_start = false;
}

void rrex3_init(rrex3_t *rrex3) {
    for (__uint8_t i = 0; i < 254; i++) {
        rrex3->functions[i] = rrex3_cmp_literal;
        rrex3->slash_functions[i] = rrex3_cmp_literal;
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
    rrex3->functions['('] = rrex3_cmp_parentheses;
    rrex3->slash_functions['w'] = rrex3_cmp_w;
    rrex3->slash_functions['W'] = rrex3_cmp_w_upper;
    rrex3->slash_functions['d'] = rrex3_cmp_d;
    rrex3->slash_functions['D'] = rrex3_cmp_d_upper;
    rrex3->slash_functions['s'] = rrex3_cmp_whitespace;
    rrex3->slash_functions['S'] = rrex3_cmp_whitespace_upper;
    rrex3->slash_functions['b'] = rrex3_cmp_word_start_or_end;
    rrex3->slash_functions['B'] = rrex3_cmp_word_not_start_or_end;
    rrex3->match_count = 0;
    rrex3->match_capacity = 0;
    rrex3->matches = NULL;
    rrex3->compiled = NULL;

    rrex3_reset(rrex3);
}

rrex3_t *rrex3_new() {
    rrex3_t *rrex3 = (rrex3_t *)malloc(sizeof(rrex3_t));

    rrex3_init(rrex3);

    return rrex3;
}

rrex3_t *rrex3_compile(rrex3_t *rrex, char *expr) {

    rrex3_t *rrex3 = rrex ? rrex : rrex3_new();

    char *compiled = (char *)malloc(strlen(expr) + 1);
    unsigned int count = 0;
    while (*expr) {
        if (*expr == '[' && *(expr + 2) == ']') {
            *compiled = *(expr + 1);
            expr++;
            expr++;
        } else if (*expr == '[' && *(expr + 1) == '0' && *(expr + 2) == '-' &&
                   *(expr + 3) == '9' && *(expr + 4) == ']') {
            *compiled = '\\';
            compiled++;
            *compiled = 'd';
            count++;
            expr++;
            expr++;
            expr++;
            expr++;
        } else {
            *compiled = *expr;
        }
        if (*compiled == '[') {
            // in_brackets = true;

        } else if (*compiled == ']') {
            // in_brackets = false;
        }
        expr++;
        compiled++;
        count++;
    }
    *compiled = 0;
    compiled -= count;
    rrex3->compiled = compiled;
    return rrex3;
}

inline static void rrex3_set_previous(rrex3_t *rrex3) {
    rrex3->previous.function = rrex3->function;
    rrex3->previous.expr = rrex3->expr;
    rrex3->previous.str = rrex3->str;
    rrex3->previous.bytecode = *rrex3->expr;
}

static bool rrex3_move(rrex3_t *rrex3, bool resume_on_fail) {
    char *original_expr = rrex3->expr;
    char *original_str = rrex3->str;
    rrex3->bytecode = *rrex3->expr;
    rrex3->function = rrex3->functions[(int)rrex3->bytecode];
    rrex3->function(rrex3);
    if (!*rrex3->expr && !*rrex3->str) {

        rrex3->exit = true;
        return rrex3->valid;
    }
    if (rrex3->pattern_error) {
        rrex3->valid = false;
        return rrex3->valid;
    }
    if (resume_on_fail && !rrex3->valid && *rrex3->expr) {
        // rrex3_set_previous(rrex3);
        rrex3->failed.bytecode = rrex3->bytecode;
        rrex3->failed.function = rrex3->function;
        rrex3->failed.expr = original_expr;
        rrex3->failed.str = original_str;
        rrex3->bytecode = *rrex3->expr;
        rrex3->function = rrex3->functions[(int)rrex3->bytecode];
        rrex3->function(rrex3);

        if (!rrex3->valid && !rrex3->pattern_error) {

            if (*rrex3->str) {
                char *pipe_position = strstr(rrex3->expr, "|");
                if (pipe_position != NULL) {
                    rrex3->expr = pipe_position + 1;
                    rrex3->str = rrex3->_str;
                    rrex3->valid = true;
                    return true;
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

rrex3_t *rrex3(rrex3_t *rrex3, char *str, char *expr) {
#if RREX3_DEBUG == 1
    printf("Regex check: %s:%s:%d\n", expr, str, 1);
#endif
    bool self_initialized = false;
    if (rrex3 == NULL) {
        self_initialized = true;
        rrex3 = rrex3_new();
    } else {
        rrex3_reset(rrex3);
    }

    rrex3->_str = str;
    rrex3->_expr = rrex3->compiled ? rrex3->compiled : expr;
    rrex3->str = rrex3->_str;
    rrex3->expr = rrex3->_expr;
    while (*rrex3->expr && !rrex3->exit) {
        if (!rrex3_move(rrex3, true))
            return NULL;
    }
    if (rrex3->valid) {
        return rrex3;
    } else {
        if (self_initialized) {
            rrex3_free(rrex3);
        }
        return NULL;
    }
}

void rrex3_test() {
    rrex3_t *rrex = rrex3_new();

    assert(rrex3(rrex, "aaaaaaa", "a*a$"));

    // assert(rrex3("ababa", "a*b*a*b*a$"));
    assert(rrex3(rrex, "#include\"test.h\"a", "#include.*\".*\"a$"));
    assert(rrex3(rrex, "#include \"test.h\"a", "#include.*\".*\"a$"));
    assert(rrex3(rrex, "aaaaaad", "a*d$"));
    assert(rrex3(rrex, "abcdef", "abd?cdef"));
    assert(!rrex3(rrex, "abcdef", "abd?def"));
    assert(rrex3(rrex, "abcdef", "def"));
    assert(!rrex3(rrex, "abcdef", "^def"));
    assert(rrex3(rrex, "abcdef", "def$"));
    assert(!rrex3(rrex, "abcdef", "^abc$"));
    assert(rrex3(rrex, "aB!.#1", "......"));
    assert(!rrex3(rrex, "aB!.#\n", "      ......"));
    assert(!rrex3(rrex, "aaaaaad", "q+d$"));
    assert(rrex3(rrex, "aaaaaaa", "a+a$"));
    assert(rrex3(rrex, "aaaaaad", "q*d$"));
    assert(!rrex3(rrex, "aaaaaad", "^q*d$"));

    // Asterisk function
    assert(rrex3(rrex, "123321", "123*321"));
    assert(rrex3(rrex, "pony", "p*ony"));
    assert(rrex3(rrex, "pppony", "p*ony"));
    assert(rrex3(rrex, "ppony", "p*pony"));
    assert(rrex3(rrex, "pppony", "pp*pony"));
    assert(rrex3(rrex, "pppony", ".*pony"));
    assert(rrex3(rrex, "pony", ".*ony"));
    assert(rrex3(rrex, "pony", "po*ny"));
    // assert(rrex3(rrex,"ppppony", "p*pppony"));

    // Plus function
    assert(rrex3(rrex, "pony", "p+ony"));
    assert(!rrex3(rrex, "ony", "p+ony"));
    assert(rrex3(rrex, "ppony", "p+pony"));
    assert(rrex3(rrex, "pppony", "pp+pony"));
    assert(rrex3(rrex, "pppony", ".+pony"));
    assert(rrex3(rrex, "pony", ".+ony"));
    assert(rrex3(rrex, "pony", "po+ny"));

    // Slash functions
    assert(rrex3(rrex, "a", "\\w"));
    assert(!rrex3(rrex, "1", "\\w"));
    assert(rrex3(rrex, "1", "\\W"));
    assert(!rrex3(rrex, "a", "\\W"));
    assert(rrex3(rrex, "a", "\\S"));
    assert(!rrex3(rrex, " ", "\\s"));
    assert(!rrex3(rrex, "\t", "\\s"));
    assert(!rrex3(rrex, "\n", "\\s"));
    assert(rrex3(rrex, "1", "\\d"));
    assert(!rrex3(rrex, "a", "\\d"));
    assert(rrex3(rrex, "a", "\\D"));
    assert(!rrex3(rrex, "1", "\\D"));
    assert(rrex3(rrex, "abc", "\\b"));

    assert(rrex3(rrex, "abc", "\\babc"));
    assert(!rrex3(rrex, "abc", "a\\b"));
    assert(!rrex3(rrex, "abc", "ab\\b"));
    assert(!rrex3(rrex, "abc", "abc\\b"));
    assert(rrex3(rrex, "abc", "a\\Bbc"));
    assert(rrex3(rrex, "abc", "ab\\B"));
    assert(!rrex3(rrex, "1ab", "1\\Bab"));
    assert(rrex3(rrex, "abc", "a\\Bbc"));

    // Escaping of special characters test.
    assert(rrex3(rrex, "()+*.\\", "\\(\\)\\+\\*\\.\\\\"));

    // Pipe
    // assert(rrex3(rrex,"abc","abc|def"));
    assert(rrex3(rrex, "abc", "def|jkl|abc"));
    assert(rrex3(rrex, "abc", "abc|def"));

    assert(rrex3(rrex, "rhq", "def|rhq|rha"));
    assert(rrex3(rrex, "abc", "abc|def"));

    // Repeat
    assert(rrex3(rrex, "aaaaa", "a{4}"));

    assert(rrex3(rrex, "aaaa", "a{1,3}a"));

    // Range
    assert(rrex3(rrex, "abc", "[abc][abc][abc]$"));
    assert(rrex3(rrex, "def", "[^abc][^abc][^abc]$"));
    assert(rrex3(rrex, "defabc", "[^abc][^abc][^abc]abc"));
    assert(rrex3(rrex, "0-9", "0-9"));
    assert(rrex3(rrex, "55-9", "[^6-9]5-9$"));
    assert(rrex3(rrex, "a", "[a-z]$"));
    assert(rrex3(rrex, "A", "[A-Z]$"));
    assert(rrex3(rrex, "5", "[0-9]$"));
    assert(!rrex3(rrex, "a", "[^a-z]$"));
    assert(!rrex3(rrex, "A", "[^A-Z]$"));
    assert(!rrex3(rrex, "5", "[^0-9]$"));
    assert(rrex3(rrex, "123abc", "[0-9]*abc$"));
    assert(rrex3(rrex, "123123", "[0-9]*$"));

    // Parentheses

    assert(rrex3(rrex, "datadata", "(data)*"));

    assert(rrex3(rrex, "datadatapony", "(data)*pony$"));

    assert(!rrex3(rrex, "datadatapony", "(d*p*ata)*pond$"));
    assert(rrex3(rrex, "datadatadato", "(d*p*ata)*dato"));
    assert(rrex3(rrex, "datadatadato", "(d*p*ata)*dato$"));
    assert(!rrex3(rrex, "datadatadato", "(d*p*a*ta)*gato$"));

    // Matches
    assert(rrex3(rrex, "123", "(123)"));
    assert(!strcmp(rrex->matches[0], "123"));

    assert(rrex3(rrex, "123321a", "(123)([0-4][2]1)a$"));
    assert(!strcmp(rrex->matches[1], "321"));

    assert(rrex3(rrex, "123321a", "(123)([0-4][2]1)a$"));
    assert(!strcmp(rrex->matches[1], "321"));

    assert(rrex3(rrex, "aaaabc", "(.*)c"));

    assert(rrex3(rrex, "abcde", ".....$"));

    assert(rrex3(rrex, "abcdefghijklmnopqrstuvwxyz",
                 "..........................$"));
    // printf("(%d)\n", rrex->valid);

    assert(rrex3(rrex, "    #include <stdio.h>", "#include.*<(.*)>"));
    assert(!strcmp(rrex->matches[0], "stdio.h"));
    assert(rrex3(rrex, "    #include \"stdlib.h\"", "#include.\"(.*)\""));
    assert(!strcmp(rrex->matches[0], "stdlib.h"));
    assert(rrex3(rrex, "    \"stdio.h\"\"string.h\"\"sys/time.h\"",
                 "\"(.*)\"\"(.*)\"\"(.*)\""));
    assert(!strcmp(rrex->matches[0], "stdio.h"));
    assert(!strcmp(rrex->matches[1], "string.h"));
    assert(!strcmp(rrex->matches[2], "sys/time.h"));
    /*
    assert(rrex3(rrex, "    #include <stdio.h>", "#include.+<(.+)>"));
    assert(!strcmp(rrex->matches[0], "stdio.h"));
    assert(rrex3(rrex, "    #include \"stdlib.h\"", "#include.+\"(.+)\""));
    assert(!strcmp(rrex->matches[0], "stdlib.h"));

     assert(rrex3(rrex, "    \"stdio.h\"\"string.h\"\"sys/time.h\"",
                "\"(.+)\"\"(.+)\"\"(.+)\""));
    assert(!strcmp(rrex->matches[0], "stdio.h"));
    assert(!strcmp(rrex->matches[1], "string.h"));
    assert(!strcmp(rrex->matches[2], "sys/time.h"));
    */
    // assert(rrex3(rrex,"char pony() {
    // }","\\b\\w+(\\s+\\*+)?\\s+\\w+\\s*\\([^)]*\\)\s*\\{[^{}]*\\}"));

    rrex3_free(rrex);
}
#endif
#ifndef RARENA_H
#define RARENA_H

#include <stdlib.h>
#include <string.h>

typedef struct arena_t {
    unsigned char *memory;
    unsigned int pointer;
    unsigned int size;
} arena_t;

arena_t *arena_construct() {
    arena_t *arena = (arena_t *)rmalloc(sizeof(arena_t));
    arena->memory = NULL;
    arena->pointer = 0;
    arena->size = 0;
    return arena;
}

arena_t *arena_new(size_t size) {
    arena_t *arena = arena_construct();
    arena->memory = (unsigned char *)rmalloc(size);
    arena->size = size;
    return arena;
}

void *arena_alloc(arena_t *arena, size_t size) {
    if (arena->pointer + size > arena->size) {
        return NULL;
    }
    void *p = arena->memory + arena->pointer;
    arena->pointer += size;
    return p;
}

void arena_free(arena_t *arena) {
    // Just constructed and unused arena memory is NULL so no free needed
    if (arena->memory) {
        rfree(arena->memory);
    }
    rfree(arena);
}

void arena_reset(arena_t *arena) { arena->pointer = 0; }
#endif
#ifndef RLIB_RIO
#define RLIB_RIO
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>

bool rfile_exists(char *path) {
    struct stat s;
    return !stat(path, &s);
}

void rjoin_path(char *p1, char *p2, char *output) {
    output[0] = 0;
    strcpy(output, p1);

    if (output[strlen(output) - 1] != '/') {
        char slash[] = "/";
        strcat(output, slash);
    }
    if (p2[0] == '/') {
        p2++;
    }
    strcat(output, p2);
}

int risprivatedir(const char *path) {
    struct stat statbuf;

    if (stat(path, &statbuf) != 0) {
        perror("stat");
        return -1;
    }

    if (!S_ISDIR(statbuf.st_mode)) {
        return -2;
    }

    if ((statbuf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)) == S_IRWXU) {
        return 1; // Private (owner has all permissions, others have none)
    }

    return 0;
}
bool risdir(const char *path) { return !risprivatedir(path); }

void rforfile(char *path, void callback(char *)) {
    if (!rfile_exists(path))
        return;
    DIR *dir = opendir(path);
    struct dirent *d;
    while ((d = readdir(dir)) != NULL) {
        if (!d)
            break;

        if ((d->d_name[0] == '.' && strlen(d->d_name) == 1) ||
            d->d_name[1] == '.') {
            continue;
        }
        char full_path[4096];
        rjoin_path(path, d->d_name, full_path);

        if (risdir(full_path)) {
            callback(full_path);
            rforfile(full_path, callback);
        } else {
            callback(full_path);
        }
    }
    closedir(dir);
}

bool rfd_wait(int fd, int ms) {
    fd_set read_fds;
    struct timeval timeout;

    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 1000 * ms; // 100 milliseconds timeout

    int ret = select(fd + 1, &read_fds, NULL, NULL, &timeout);
    return ret > 0 && FD_ISSET(fd, &read_fds);
}

bool rfd_wait_forever(int fd) {
    while ((!rfd_wait(fd, 10))) {
    }
    return true;
}

size_t rfile_size(char *path) {
    struct stat s;
    stat(path, &s);
    return s.st_size;
}

size_t rfile_readb(char *path, void *data, size_t size) {
    FILE *fd = fopen(path, "r");
    if (!fd) {
        return 0;
    }
    __attribute__((unused)) size_t bytes_read =
        fread(data, size, sizeof(char), fd);

    fclose(fd);
    return size;
}

#endif
#ifndef RSTRING_H
#define RSTRING_H
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long _r_generate_key_current = 0;

char *_rcat_int_int(int a, int b) {
    static char res[20];
    res[0] = 0;
    sprintf(res, "%d%d", a, b);
    return res;
}
char *_rcat_int_double(int a, double b) {
    static char res[20];
    res[0] = 0;
    sprintf(res, "%d%f", a, b);
    return res;
}

char *_rcat_charp_int(char *a, int b) {
    char res[20];
    sprintf(res, "%c", b);
    return strcat(a, res);
}

char *_rcat_charp_double(char *a, double b) {
    char res[20];
    sprintf(res, "%f", b);
    return strcat(a, res);
}

char *_rcat_charp_charp(char *a, char *b) {
    ;
    return strcat(a, b);
}
char *_rcat_charp_char(char *a, char b) {
    char extra[] = {b, 0};
    return strcat(a, extra);
}
char *_rcat_charp_bool(char *a, bool *b) {
    if (b) {
        return strcat(a, "true");
    } else {
        return strcat(a, "false");
    }
}

#define rcat(x, y)                                                             \
    _Generic((x),   \
    int: _Generic((y),     \
        int: _rcat_int_int,\
        double: _rcat_int_double,\
        char*: _rcat_charp_charp),\
    char*: _Generic((y),\
        int: _rcat_charp_int, \
        double: _rcat_charp_double,\
        char*: _rcat_charp_charp, \
        char: _rcat_charp_char, \
        bool: _rcat_charp_bool))((x),(y))

char *rgenerate_key() {
    _r_generate_key_current++;
    static char key[100];
    key[0] = 0;
    sprintf(key, "%ld", _r_generate_key_current);
    return key;
}

char *rformat_number(long lnumber) {
    static char formatted[1024];

    char number[1024];
    sprintf(number, "%ld", lnumber);

    int len = strlen(number);
    int commas_needed = (len - 1) / 3;
    int new_len = len + commas_needed;

    formatted[new_len] = '\0';

    int i = len - 1;
    int j = new_len - 1;
    int count = 0;

    while (i >= 0) {
        if (count == 3) {
            formatted[j--] = '.';
            count = 0;
        }
        formatted[j--] = number[i--];
        count++;
    }
    return formatted;
}

bool rstrextractdouble(char *str, double *d1) {
    for (size_t i = 0; i < strlen(str); i++) {
        if (isdigit(str[i])) {
            str += i;
            sscanf(str, "%lf", d1);
            return true;
        }
    }
    return false;
}

void rstrstripslashes(const char *content, char *result) {
    size_t content_length = strlen((char *)content);
    unsigned int index = 0;
    for (unsigned int i = 0; i < content_length; i++) {
        char c = content[i];
        if (c == '\\') {
            i++;
            c = content[i];
            if (c == 'r') {
                c = '\r';
            } else if (c == 't') {
                c = '\t';
            } else if (c == 'b') {
                c = '\b';
            } else if (c == 'n') {
                c = '\n';
            } else if (c == 'f') {
                c = '\f';
            } else if (c == '\\') {
                // No need tbh
                c = '\\';
            }
        }
        result[index] = c;
        index++;
    }
    result[index] = 0;
}

int rstrstartswith(const char *s1, const char *s2) {
    if (s1 == NULL)
        return s2 == NULL;
    if (s1 == s2 || s2 == NULL || *s2 == 0)
        return true;
    size_t len_s2 = strlen(s2);
    size_t len_s1 = strlen(s1);
    if (len_s2 > len_s1)
        return false;
    return !strncmp(s1, s2, len_s2);
}

bool rstrendswith(const char *s1, const char *s2) {
    if (s1 == NULL)
        return s2 == NULL;
    if (s1 == s2 || s2 == NULL || *s2 == 0)
        return true;
    size_t len_s2 = strlen(s2);
    size_t len_s1 = strlen(s1);
    if (len_s2 > len_s1) {
        return false;
    }
    s1 += len_s1 - len_s2;
    return !strncmp(s1, s2, len_s2);
}

void rstraddslashes(const char *content, char *result) {
    size_t content_length = strlen((char *)content);
    unsigned int index = 0;
    for (unsigned int i = 0; i < content_length; i++) {
        if (content[i] == '\r') {
            result[index] = '\\';
            index++;
            result[index] = 'r';
            index++;
            continue;
        } else if (content[i] == '\t') {
            result[index] = '\\';
            index++;
            result[index] = 't';
            index++;
            continue;
        } else if (content[i] == '\n') {
            result[index] = '\\';
            index++;
            result[index] = 'n';
            index++;
            continue;
        } else if (content[i] == '\\') {
            result[index] = '\\';
            index++;
            result[index] = '\\';
            index++;
            continue;
        } else if (content[i] == '\b') {
            result[index] = '\\';
            index++;
            result[index] = 'b';
            index++;
            continue;
        } else if (content[i] == '\f') {
            result[index] = '\\';
            index++;
            result[index] = 'f';
            index++;
            continue;
        }
        result[index] = content[i];
        index++;
    }
    result[index] = 0;
}

int rstrip_whitespace(char *input, char *output) {
    output[0] = 0;
    int count = 0;
    size_t len = strlen(input);
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '\t' || input[i] == ' ') {
            continue;
        }
        count = i;
        size_t j;
        for (j = 0; j < len - count; j++) {
            output[j] = input[j + count];
        }
        output[j] = '\0';
        break;
    }
    return count;
}

void rstrtocstring(const char *input, char *output) {
    int index = 0;
    char clean_input[strlen(input) * 2];
    char *iptr = clean_input;
    rstraddslashes(input, clean_input);
    output[index] = '"';
    index++;
    while (*iptr) {
        if (*iptr == '"') {
            output[index] = '\\';
            output++;
        } else if (*iptr == '\\' && *(iptr + 1) == 'n') {
            output[index] = '\\';
            output++;
            output[index] = 'n';
            output++;
            output[index] = '"';
            output++;
            output[index] = '\n';
            output++;
            output[index] = '"';
            output++;
            iptr++;
            iptr++;
            continue;
        }
        output[index] = *iptr;
        index++;
        iptr++;
    }
    if (output[index - 1] == '"' && output[index - 2] == '\n') {
        output[index - 1] = 0;
    } else if (output[index - 1] != '"') {
        output[index] = '"';
        output[index + 1] = 0;
    }
}

size_t rstrtokline(char *input, char *output, size_t offset, bool strip_nl) {

    size_t len = strlen(input);
    output[0] = 0;
    size_t new_offset = 0;
    size_t j;
    size_t index = 0;

    for (j = offset; j < len + offset; j++) {
        if (input[j] == 0) {
            index++;
            break;
        }
        index = j - offset;
        output[index] = input[j];

        if (output[index] == '\n') {
            index++;
            break;
        }
    }
    output[index] = 0;

    new_offset = index + offset;

    if (strip_nl) {
        if (output[index - 1] == '\n') {
            output[index - 1] = 0;
        }
    }
    return new_offset;
}

void rstrjoin(char **lines, size_t count, char *glue, char *output) {
    output[0] = 0;
    for (size_t i = 0; i < count; i++) {
        strcat(output, lines[i]);
        if (i != count - 1)
            strcat(output, glue);
    }
}

int rstrsplit(char *input, char **lines) {
    int index = 0;
    size_t offset = 0;
    char line[1024];
    while ((offset = rstrtokline(input, line, offset, false)) && *line) {
        if (!*line) {
            break;
        }
        lines[index] = (char *)malloc(strlen(line) + 1);
        strcpy(lines[index], line);
        index++;
    }
    return index;
}

bool rstartswithnumber(char *str) { return isdigit(str[0]); }

void rstrmove2(char *str, unsigned int start, size_t length,
               unsigned int new_pos) {
    size_t str_len = strlen(str);
    char new_str[str_len + 1];
    memset(new_str, 0, str_len);
    if (start < new_pos) {
        strncat(new_str, str + length, str_len - length - start);
        new_str[new_pos] = 0;
        strncat(new_str, str + start, length);
        strcat(new_str, str + strlen(new_str));
        memset(str, 0, str_len);
        strcpy(str, new_str);
    } else {
        strncat(new_str, str + start, length);
        strncat(new_str, str, start);
        strncat(new_str, str + start + length, str_len - start);
        memset(str, 0, str_len);
        strcpy(str, new_str);
    }
    new_str[str_len] = 0;
}

void rstrmove(char *str, unsigned int start, size_t length,
              unsigned int new_pos) {
    size_t str_len = strlen(str);
    if (start >= str_len || new_pos >= str_len || start + length > str_len) {
        return;
    }
    char temp[length + 1];
    strncpy(temp, str + start, length);
    temp[length] = 0;
    if (start < new_pos) {
        memmove(str + start, str + start + length, new_pos - start);
        strncpy(str + new_pos - length + 1, temp, length);
    } else {
        memmove(str + new_pos + length, str + new_pos, start - new_pos);
        strncpy(str + new_pos, temp, length);
    }
}

int cmp_line(const void *left, const void *right) {
    char *l = *(char **)left;
    char *r = *(char **)right;

    char lstripped[strlen(l) + 1];
    rstrip_whitespace(l, lstripped);
    char rstripped[strlen(r) + 1];
    rstrip_whitespace(r, rstripped);

    double d1, d2;
    bool found_d1 = rstrextractdouble(lstripped, &d1);
    bool found_d2 = rstrextractdouble(rstripped, &d2);

    if (found_d1 && found_d2) {
        double frac_part1;
        double int_part1;
        frac_part1 = modf(d1, &int_part1);
        double frac_part2;
        double int_part2;
        frac_part2 = modf(d2, &int_part2);
        if (d1 == d2) {
            return strcmp(lstripped, rstripped);
        } else if (frac_part1 && frac_part2) {
            return d1 > d2;
        } else if (frac_part1 && !frac_part2) {
            return 1;
        } else if (frac_part2 && !frac_part1) {
            return -1;
        } else if (!frac_part1 && !frac_part2) {
            return d1 > d2;
        }
    }
    return 0;
}

int rstrsort(char *input, char *output) {
    char **lines = (char **)malloc(strlen(input) * 10);
    int line_count = rstrsplit(input, lines);
    qsort(lines, line_count, sizeof(char *), cmp_line);
    rstrjoin(lines, line_count, "", output);
    free(lines);
    return line_count;
}

#endif
#ifndef RLIB_TERMINAL_H
#define RLIB_TERMINAL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *rfcaptured = NULL;

void rfcapture(FILE *f, char *buff, size_t size) {
    rfcaptured = buff;
    setvbuf(f, rfcaptured, _IOFBF, size);
}
void rfstopcapture(FILE *f) { setvbuf(f, 0, _IOFBF, 0); }

bool _r_disable_stdout_toggle = false;

FILE *_r_original_stdout = NULL;

bool rr_enable_stdout() {
    if (_r_disable_stdout_toggle)
        return false;
    if (!_r_original_stdout) {
        stdout = fopen("/dev/null", "rb");
        return false;
    }
    if (_r_original_stdout && _r_original_stdout != stdout) {
        fclose(stdout);
    }
    stdout = _r_original_stdout;
    return true;
}
bool rr_disable_stdout() {
    if (_r_disable_stdout_toggle) {
        return false;
    }
    if (_r_original_stdout == NULL) {
        _r_original_stdout = stdout;
    }
    if (stdout == _r_original_stdout) {
        stdout = fopen("/dev/null", "rb");
        return true;
    }
    return false;
}
bool rr_toggle_stdout() {
    if (!_r_original_stdout) {
        rr_disable_stdout();
        return true;
    } else if (stdout != _r_original_stdout) {
        rr_enable_stdout();
        return true;
    } else {
        rr_disable_stdout();
        return true;
    }
}

typedef struct rprogressbar_t {
    unsigned long current_value;
    unsigned long min_value;
    unsigned long max_value;
    unsigned int length;
    bool changed;
    double percentage;
    unsigned int width;
    unsigned long draws;
    FILE *fout;
} rprogressbar_t;

rprogressbar_t *rprogressbar_new(long min_value, long max_value,
                                 unsigned int width, FILE *fout) {
    rprogressbar_t *pbar = (rprogressbar_t *)malloc(sizeof(rprogressbar_t));
    pbar->min_value = min_value;
    pbar->max_value = max_value;
    pbar->current_value = min_value;
    pbar->width = width;
    pbar->draws = 0;
    pbar->length = 0;
    pbar->changed = false;
    pbar->fout = fout ? fout : stdout;
    return pbar;
}

void rprogressbar_free(rprogressbar_t *pbar) { free(pbar); }

void rprogressbar_draw(rprogressbar_t *pbar) {
    if (!pbar->changed) {
        return;
    } else {
        pbar->changed = false;
    }
    pbar->draws++;
    char draws_text[22];
    draws_text[0] = 0;
    sprintf(draws_text, "%ld", pbar->draws);
    char *draws_textp = draws_text;
    // bool draws_text_len = strlen(draws_text);
    char bar_begin_char = ' ';
    char bar_progress_char = ' ';
    char bar_empty_char = ' ';
    char bar_end_char = ' ';
    char content[4096] = {0};
    char bar_content[1024];
    char buff[2048] = {0};
    bar_content[0] = '\r';
    bar_content[1] = bar_begin_char;
    unsigned int index = 2;
    for (unsigned long i = 0; i < pbar->length; i++) {
        if (*draws_textp) {
            bar_content[index] = *draws_textp;
            draws_textp++;
        } else {
            bar_content[index] = bar_progress_char;
        }
        index++;
    }
    char infix[] = "\033[0m";
    for (unsigned long i = 0; i < strlen(infix); i++) {
        bar_content[index] = infix[i];
        index++;
    }
    for (unsigned long i = 0; i < pbar->width - pbar->length; i++) {
        bar_content[index] = bar_empty_char;
        index++;
    }
    bar_content[index] = bar_end_char;
    bar_content[index + 1] = '\0';
    sprintf(buff, "\033[43m%s\033[0m \033[33m%.2f%%\033[0m ", bar_content,
            pbar->percentage * 100);
    strcat(content, buff);
    if (pbar->width == pbar->length) {
        strcat(content, "\r");
        for (unsigned long i = 0; i < pbar->width + 10; i++) {
            strcat(content, " ");
        }
        strcat(content, "\r");
    }
    fprintf(pbar->fout, "%s", content);
    fflush(pbar->fout);
}

bool rprogressbar_update(rprogressbar_t *pbar, unsigned long value) {
    if (value == pbar->current_value) {
        return false;
    }
    pbar->current_value = value;
    pbar->percentage = (double)pbar->current_value /
                       (double)(pbar->max_value - pbar->min_value);
    unsigned long new_length = (unsigned long)(pbar->percentage * pbar->width);
    pbar->changed = new_length != pbar->length;
    if (pbar->changed) {
        pbar->length = new_length;
        rprogressbar_draw(pbar);
        return true;
    }
    return false;
}

size_t rreadline(char *data, size_t len, bool strip_ln) {
    __attribute__((unused)) char *unused = fgets(data, len, stdin);
    size_t length = strlen(data);
    if (length && strip_ln)
        data[length - 1] = 0;
    return length;
}

void rlib_test_progressbar() {
    rtest_banner("Progress bar");
    rprogressbar_t *pbar = rprogressbar_new(0, 1000, 10, stderr);
    rprogressbar_draw(pbar);
    // No draws executed, nothing to show
    rassert(pbar->draws == 0);
    rprogressbar_update(pbar, 500);
    rassert(pbar->percentage == 0.5);
    rprogressbar_update(pbar, 500);
    rprogressbar_update(pbar, 501);
    rprogressbar_update(pbar, 502);
    // Should only have drawn one time since value did change, but percentage
    // did not
    rassert(pbar->draws == 1);
    // Changed is false because update function calls draw
    rassert(pbar->changed == false);
    rprogressbar_update(pbar, 777);
    rassert(pbar->percentage == 0.777);
    rprogressbar_update(pbar, 1000);
    rassert(pbar->percentage == 1);
}

#endif
#ifndef RTERM_H
#define RTERM_H
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

typedef struct winsize winsize_t;

typedef struct rshell_keypress_t {
    bool pressed;
    bool ctrl;
    bool shift;
    bool escape;
    char c;
    int ms;
    int fd;
} rshell_keypress_t;

typedef struct rterm_t {
    bool show_cursor;
    bool show_footer;
    rshell_keypress_t key;
    void (*before_cursor_move)(struct rterm_t *);
    void (*after_cursor_move)(struct rterm_t *);
    void (*after_key_press)(struct rterm_t *);
    void (*before_key_press)(struct rterm_t *);
    void (*before_draw)(struct rterm_t *);
    void *session;
    unsigned long iterations;
    void (*tick)(struct rterm_t *);
    char *status_text;
    winsize_t size;
    struct {
        int x;
        int y;
        int pos;
        int available;
    } cursor;
} rterm_t;

typedef void (*rterm_event)(rterm_t *);

void rterm_init(rterm_t *rterm) {
    memset(rterm, 0, sizeof(rterm_t));
    rterm->show_cursor = true;
    rterm->show_cursor = true;
}

void rterm_getwinsize(winsize_t *w) {
    // Get the terminal size
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, w) == -1) {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }
}

// Terminal setup functions
void enableRawMode(struct termios *orig_termios) {
    struct termios raw = *orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echoing
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1; // Set timeout for read input

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode(struct termios *orig_termios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH,
              orig_termios); // Restore original terminal settings
}

void rterm_clear_screen() {
    printf("\x1b[2J"); // Clear the entire screen
    printf("\x1b[H");  // Move cursor to the home position (0,0)
}

void setBackgroundColor() {
    printf("\x1b[44m"); // Set background color to blue
}

void rterm_move_cursor(int x, int y) {

    printf("\x1b[%d;%dH", y + 1, x + 1); // Move cursor to (x, y)
}

void cursor_set(rterm_t *rt, int x, int y) {
    rt->cursor.x = x;
    rt->cursor.y = y;
    rt->cursor.pos = y * rt->size.ws_col + x;
    rterm_move_cursor(rt->cursor.x, rt->cursor.y);
}
void cursor_restore(rterm_t *rt) {
    rterm_move_cursor(rt->cursor.x, rt->cursor.y);
}

void rterm_print_status_bar(rterm_t *rt, char c, unsigned long i) {
    winsize_t ws = rt->size;
    rterm_move_cursor(0, ws.ws_row - 1);

    char output_str[1024];
    output_str[0] = 0;

    // strcat(output_str, "\x1b[48;5;240m");

    for (int i = 0; i < ws.ws_col; i++) {
        strcat(output_str, " ");
    }
    char content[500];
    content[0] = 0;
    if (!rt->status_text) {
        sprintf(content, "\rp:%d:%d | k:%c:%d | i:%ld ", rt->cursor.x + 1,
                rt->cursor.y + 1, c == 0 ? '0' : c, c, i);
    } else {
        sprintf(content, "\r%s", rt->status_text);
    }
    strcat(output_str, content);
    // strcat(output_str, "\x1b[0m");
    printf("%s", output_str);
    cursor_restore(rt);
}

void rterm_show_cursor() {
    printf("\x1b[?25h"); // Show the cursor
}

void rterm_hide_cursor() {
    printf("\x1b[?25l"); // Hide the cursor
}

rshell_keypress_t rshell_getkey() {
    static rshell_keypress_t press;
    press.c = 0;
    press.ctrl = false;
    press.shift = false;
    press.escape = false;
    press.pressed = rfd_wait(0, 100);
    if (press.pressed) {
        press.c = getchar();
    }
    char ch = press.c;
    if (ch == '\x1b') {
        // Get detail
        ch = getchar();

        if (ch == '[') {
            // non char key:
            press.escape = true;

            ch = getchar(); // is a number. 1 if shift + arrow
            press.c = ch;
            if (ch >= '0' && ch <= '9')
                ch = getchar();
            press.c = ch;
            if (ch == ';') {
                ch = getchar();
                press.c = ch;
                if (ch == '5') {
                    press.ctrl = true;
                    press.c = getchar(); // De arrow
                }
            }
        } else {
            press.c = ch;
        }
    }
    return press;
}

// Main function
void rterm_loop(rterm_t *rt) {
    struct termios orig_termios;
    tcgetattr(STDIN_FILENO, &orig_termios); // Get current terminal attributes
    enableRawMode(&orig_termios);

    int x = 0, y = 0; // Initial cursor position
    char ch = 0;
    ;
    while (1) {
        rterm_getwinsize(&rt->size);
        rt->cursor.available = rt->size.ws_col * rt->size.ws_row;
        if (rt->tick) {
            rt->tick(rt);
        }

        rterm_hide_cursor();
        // setBackgroundColor();
        rterm_clear_screen();
        if (rt->before_draw) {
            rt->before_draw(rt);
        }
        rterm_print_status_bar(rt, ch, rt->iterations);
        if (!rt->iterations || (x != rt->cursor.x || y != rt->cursor.y)) {
            if (y == rt->size.ws_row) {
                y--;
            }
            if (y < 0) {
                y = 0;
            }
            rt->cursor.x = x;
            rt->cursor.y = y;
            if (rt->before_cursor_move)
                rt->before_cursor_move(rt);
            cursor_set(rt, rt->cursor.x, rt->cursor.y);
            if (rt->after_cursor_move)
                rt->after_cursor_move(rt);
            x = rt->cursor.x;
            y = rt->cursor.y;
        }
        if (rt->show_cursor)
            rterm_show_cursor();
        fflush(stdout);

        rt->key = rshell_getkey();
        if (rt->key.pressed && rt->before_key_press) {
            rt->before_key_press(rt);
        }
        rshell_keypress_t key = rt->key;
        ch = key.c;
        if (ch == 'q')
            break; // Press 'q' to quit

        // Escape
        if (key.escape) {
            switch (key.c) {
            case 65: // Move up
                if (y > -1)
                    y--;
                break;
            case 66: // Move down
                if (y < rt->size.ws_row)
                    y++;
                break;
            case 68: // Move left
                if (x > 0)
                    x--;
                if (key.ctrl)
                    x -= 4;
                break;
            case 67: // Move right
                if (x < rt->size.ws_col) {
                    x++;
                }
                if (key.ctrl) {
                    x += 4;
                }
                break;
            }
        }
        if (rt->key.pressed && rt->after_key_press) {
            rt->after_key_press(rt);
        }
        rt->iterations++;

        //  usleep (1000);
    }

    // Cleanup
    printf("\x1b[0m"); // Reset colors
    rterm_clear_screen();
    disableRawMode(&orig_termios);
}
#endif
#ifndef RTREE_H
#define RTREE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct rtree_t {
    struct rtree_t *next;
    struct rtree_t *children;
    char c;
    void *data;
} rtree_t;

rtree_t *rtree_new() {
    rtree_t *b = (rtree_t *)rmalloc(sizeof(rtree_t));
    b->next = NULL;
    b->children = NULL;
    b->c = 0;
    b->data = NULL;
    return b;
}

rtree_t *rtree_set(rtree_t *b, char *c, void *data) {
    while (b) {
        if (b->c == 0) {
            b->c = *c;
            c++;
            if (*c == 0) {
                b->data = data;
                // printf("SET1 %c\n", b->c);
                return b;
            }
        } else if (b->c == *c) {
            c++;
            if (*c == 0) {
                b->data = data;
                return b;
            }
            if (b->children) {
                b = b->children;
            } else {
                b->children = rtree_new();
                b = b->children;
            }
        } else if (b->next) {
            b = b->next;
        } else {
            b->next = rtree_new();
            b = b->next;
            b->c = *c;
            c++;
            if (*c == 0) {
                b->data = data;
                return b;
            } else {
                b->children = rtree_new();
                b = b->children;
            }
        }
    }
    return NULL;
}

rtree_t *rtree_find(rtree_t *b, char *c) {
    while (b) {
        if (b->c == *c) {
            c++;
            if (*c == 0) {
                return b;
            }
            b = b->children;
            continue;
        }
        b = b->next;
    }
    return NULL;
}

void rtree_free(rtree_t *b) {
    if (!b)
        return;
    rtree_free(b->children);
    rtree_free(b->next);
    rfree(b);
}

void *rtree_get(rtree_t *b, char *c) {
    rtree_t *t = rtree_find(b, c);
    if (t) {
        return t->data;
    }
    return NULL;
}
#endif
#ifndef RLEXER_H
#define RLEXER_H
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define RTOKEN_VALUE_SIZE 1024

typedef enum rtoken_type_t {
    RT_UNKNOWN = 0,
    RT_SYMBOL,
    RT_NUMBER,
    RT_STRING,
    RT_PUNCT,
    RT_OPERATOR,
    RT_EOF = 10,
    RT_BRACE_OPEN,
    RT_CURLY_BRACE_OPEN,
    RT_BRACKET_OPEN,
    RT_BRACE_CLOSE,
    RT_CURLY_BRACE_CLOSE,
    RT_BRACKET_CLOSE
} rtoken_type_t;

typedef struct rtoken_t {
    rtoken_type_t type;
    char value[RTOKEN_VALUE_SIZE];
    unsigned int line;
    unsigned int col;
} rtoken_t;

static char *_content;
static unsigned int _content_ptr;
static unsigned int _content_line;
static unsigned int _content_col;

static int isgroupingchar(char c) {
    return (c == '{' || c == '}' || c == '(' || c == ')' || c == '[' ||
            c == ']' || c == '"' || c == '\'');
}

static int isoperator(char c) {
    return (c == '+' || c == '-' || c == '/' || c == '*' || c == '=' ||
            c == '>' || c == '<' || c == '|' || c == '&');
}

static rtoken_t rtoken_new() {
    rtoken_t token;
    memset(&token, 0, sizeof(token));
    token.type = RT_UNKNOWN;
    return token;
}

rtoken_t rlex_number() {
    rtoken_t token = rtoken_new();
    token.col = _content_col;
    token.line = _content_line;
    bool first_char = true;
    int dot_count = 0;
    char c;
    while (isdigit(c = _content[_content_ptr]) ||
           (first_char && _content[_content_ptr] == '-') ||
           (dot_count == 0 && _content[_content_ptr] == '.')) {
        if (c == '.')
            dot_count++;
        first_char = false;
        char chars[] = {c, 0};
        strcat(token.value, chars);
        _content_ptr++;
        _content_col++;
    }
    token.type = RT_NUMBER;
    return token;
}

static rtoken_t rlex_symbol() {
    rtoken_t token = rtoken_new();

    token.col = _content_col;
    token.line = _content_line;
    char c;
    while (isalpha(_content[_content_ptr]) || _content[_content_ptr] == '_') {
        c = _content[_content_ptr];
        char chars[] = {c, 0};
        strcat(token.value, chars);
        _content_ptr++;
        _content_col++;
    }
    token.type = RT_SYMBOL;
    return token;
}

static rtoken_t rlex_operator() {

    rtoken_t token = rtoken_new();

    token.col = _content_col;
    token.line = _content_line;
    char c;
    bool is_first = true;
    while (isoperator(_content[_content_ptr])) {
        if (!is_first) {
            if (_content[_content_ptr - 1] == '=' &&
                _content[_content_ptr] == '-') {
                break;
            }
        }
        c = _content[_content_ptr];
        char chars[] = {c, 0};
        strcat(token.value, chars);
        _content_ptr++;
        _content_col++;
        is_first = false;
    }
    token.type = RT_OPERATOR;
    return token;
}

static rtoken_t rlex_punct() {

    rtoken_t token = rtoken_new();

    token.col = _content_col;
    token.line = _content_line;
    char c;
    bool is_first = true;
    while (ispunct(_content[_content_ptr])) {
        if (!is_first) {
            if (_content[_content_ptr] == '"') {
                break;
            }
            if (_content[_content_ptr] == '\'') {
                break;
            }
            if (isgroupingchar(_content[_content_ptr])) {
                break;
            }
            if (isoperator(_content[_content_ptr])) {
                break;
            }
        }
        c = _content[_content_ptr];
        char chars[] = {c, 0};
        strcat(token.value, chars);
        _content_ptr++;
        _content_col++;
        is_first = false;
    }
    token.type = RT_PUNCT;
    return token;
}

static rtoken_t rlex_string() {
    rtoken_t token = rtoken_new();
    char c;
    token.col = _content_col;
    token.line = _content_line;
    char str_chr = _content[_content_ptr];
    _content_ptr++;
    while (_content[_content_ptr] != str_chr) {
        c = _content[_content_ptr];
        if (c == '\\') {
            _content_ptr++;
            c = _content[_content_ptr];
            if (c == 'n') {
                c = '\n';
            } else if (c == 'r') {
                c = '\r';
            } else if (c == 't') {
                c = '\t';
            } else if (c == str_chr) {
                c = str_chr;
            }

            _content_col++;
        }
        char chars[] = {c, 0};
        strcat(token.value, chars);
        _content_ptr++;
        _content_col++;
    }
    _content_ptr++;
    token.type = RT_STRING;
    return token;
}

void rlex(char *content) {
    _content = content;
    _content_ptr = 0;
    _content_col = 1;
    _content_line = 1;
}

static void rlex_repeat_str(char *dest, char *src, unsigned int times) {
    for (size_t i = 0; i < times; i++) {
        strcat(dest, src);
    }
}

rtoken_t rtoken_create(rtoken_type_t type, char *value) {
    rtoken_t token = rtoken_new();
    token.type = type;
    token.col = _content_col;
    token.line = _content_line;
    strcpy(token.value, value);
    return token;
}

rtoken_t rlex_next() {
    while (true) {

        _content_col++;

        if (_content[_content_ptr] == 0) {
            return rtoken_create(RT_EOF, "eof");
        } else if (_content[_content_ptr] == '\n') {
            _content_line++;
            _content_col = 1;
            _content_ptr++;
        } else if (isspace(_content[_content_ptr])) {
            _content_ptr++;
        } else if (isdigit(_content[_content_ptr]) ||
                   (_content[_content_ptr] == '-' &&
                    isdigit(_content[_content_ptr + 1]))) {
            return rlex_number();
        } else if (isalpha(_content[_content_ptr]) ||
                   _content[_content_ptr] == '_') {
            return rlex_symbol();
        } else if (_content[_content_ptr] == '"' ||
                   _content[_content_ptr] == '\'') {
            return rlex_string();
        } else if (isoperator(_content[_content_ptr])) {
            return rlex_operator();
        } else if (ispunct(_content[_content_ptr])) {
            if (_content[_content_ptr] == '{') {

                _content_ptr++;
                return rtoken_create(RT_CURLY_BRACE_OPEN, "{");
            }
            if (_content[_content_ptr] == '}') {

                _content_ptr++;
                return rtoken_create(RT_CURLY_BRACE_CLOSE, "}");
            }
            if (_content[_content_ptr] == '(') {

                _content_ptr++;
                return rtoken_create(RT_BRACE_OPEN, "(");
            }
            if (_content[_content_ptr] == ')') {

                _content_ptr++;
                return rtoken_create(RT_BRACE_CLOSE, ")");
            }
            if (_content[_content_ptr] == '[') {

                _content_ptr++;
                return rtoken_create(RT_BRACKET_OPEN, "[");
            }
            if (_content[_content_ptr] == ']') {

                _content_ptr++;
                return rtoken_create(RT_BRACKET_CLOSE, "]");
            }
            return rlex_punct();
        }
    }
}

char *rlex_format(char *content) {
    rlex(content);
    char *result = (char *)malloc(strlen(content) + 4096);
    result[0] = 0;
    unsigned int tab_index = 0;
    char *tab_chars = "    ";
    unsigned int col = 0;
    rtoken_t token_previous;
    token_previous.value[0] = 0;
    token_previous.type = RT_UNKNOWN;
    while (true) {
        rtoken_t token = rlex_next();
        if (token.type == RT_EOF) {
            break;
        }

        // col = strlen(token.value);

        if (col == 0) {
            rlex_repeat_str(result, tab_chars, tab_index);
            // col = strlen(token.value);// strlen(tab_chars) * tab_index;
        }

        if (token.type == RT_STRING) {
            strcat(result, "\"");

            char string_with_slashes[strlen(token.value) * 2 + 1];
            rstraddslashes(token.value, string_with_slashes);
            strcat(result, string_with_slashes);

            strcat(result, "\"");
            // col+= strlen(token.value) + 2;
            // printf("\n");
            // printf("<<<%s>>>\n",token.value);

            memcpy(&token_previous, &token, sizeof(token));
            continue;
        }
        if (!(strcmp(token.value, "{"))) {
            if (col != 0) {
                strcat(result, "\n");
                rlex_repeat_str(result, "    ", tab_index);
            }
            strcat(result, token.value);

            tab_index++;

            strcat(result, "\n");

            col = 0;

            memcpy(&token_previous, &token, sizeof(token));
            continue;
        } else if (!(strcmp(token.value, "}"))) {
            unsigned int tab_indexed = 0;
            if (tab_index)
                tab_index--;
            strcat(result, "\n");

            rlex_repeat_str(result, tab_chars, tab_index);
            tab_indexed++;

            strcat(result, token.value);
            strcat(result, "\n");
            col = 0;

            memcpy(&token_previous, &token, sizeof(token));
            continue;
        }
        if ((token_previous.type == RT_SYMBOL && token.type == RT_NUMBER) ||
            (token_previous.type == RT_NUMBER && token.type == RT_SYMBOL) ||
            (token_previous.type == RT_PUNCT && token.type == RT_SYMBOL) ||
            (token_previous.type == RT_BRACE_CLOSE &&
             token.type == RT_SYMBOL) ||
            (token_previous.type == RT_SYMBOL && token.type == RT_SYMBOL)) {
            if (token_previous.value[0] != ',' &&
                token_previous.value[0] != '.') {
                if (token.type != RT_OPERATOR && token.value[0] != '.') {
                    strcat(result, "\n");
                    rlex_repeat_str(result, tab_chars, tab_index);
                }
            }
        }

        if (token.type == RT_OPERATOR) {
            strcat(result, " ");
        }
        if (token.type == RT_STRING) {
            strcat(result, "\"");
        }
        strcat(result, token.value);
        if (token.type == RT_STRING) {
            strcat(result, "\"");
        }

        if (token.type == RT_OPERATOR) {
            strcat(result, " ");
        }
        if (!strcmp(token.value, ",")) {
            strcat(result, " ");
        }
        col += strlen(token.value);
        memcpy(&token_previous, &token, sizeof(token));
    }
    return result;
}
#endif
#ifndef RBENCH_H
#define RBENCH_H

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#define RBENCH(times, action)                                                  \
    {                                                                          \
        unsigned long utimes = (unsigned long)times;                           \
        nsecs_t start = nsecs();                                               \
        for (unsigned long i = 0; i < utimes; i++) {                           \
            { action; }                                                        \
        }                                                                      \
        nsecs_t end = nsecs();                                                 \
        printf("%s\n", format_time(end - start));                              \
    }

#define RBENCHP(times, action)                                                 \
    {                                                                          \
        printf("\n");                                                          \
        nsecs_t start = nsecs();                                               \
        unsigned int prev_percentage = 0;                                      \
        unsigned long utimes = (unsigned long)times;                           \
        for (unsigned long i = 0; i < utimes; i++) {                           \
            unsigned int percentage =                                          \
                ((long double)i / (long double)times) * 100;                   \
            int percentage_changed = percentage != prev_percentage;            \
            __attribute__((unused)) int first = i == 0;                        \
            __attribute__((unused)) int last = i == utimes - 1;                \
            { action; };                                                       \
            if (percentage_changed) {                                          \
                printf("\r%d%%", percentage);                                  \
                fflush(stdout);                                                \
                                                                               \
                prev_percentage = percentage;                                  \
            }                                                                  \
        }                                                                      \
        nsecs_t end = nsecs();                                                 \
        printf("\r%s\n", format_time(end - start));                            \
    }

struct rbench_t;

typedef struct rbench_function_t {
#ifdef __cplusplus
    void (*call)();
#else
    void(*call);
#endif
    char name[256];
    char group[256];
    void *arg;
    void *data;
    bool first;
    bool last;
    int argc;
    unsigned long times_executed;

    nsecs_t average_execution_time;
    nsecs_t total_execution_time;
} rbench_function_t;

typedef struct rbench_t {
    unsigned int function_count;
    rbench_function_t functions[100];
    rbench_function_t *current;
    rprogressbar_t *progress_bar;
    bool show_progress;
    int winner;
    bool stdout;
    unsigned long times;
    bool silent;
    nsecs_t execution_time;
#ifdef __cplusplus
    void (*add_function)(struct rbench_t *r, const char *name,
                         const char *group, void (*)());
#else
    void (*add_function)(struct rbench_t *r, const char *name,
                         const char *group, void *);
#endif
    void (*rbench_reset)(struct rbench_t *r);
    struct rbench_t *(*execute)(struct rbench_t *r, long times);
    struct rbench_t *(*execute1)(struct rbench_t *r, long times, void *arg1);
    struct rbench_t *(*execute2)(struct rbench_t *r, long times, void *arg1,
                                 void *arg2);
    struct rbench_t *(*execute3)(struct rbench_t *r, long times, void *arg1,
                                 void *arg2, void *arg3);

} rbench_t;

FILE *_rbench_stdout = NULL;
FILE *_rbench_stdnull = NULL;

void rbench_toggle_stdout(rbench_t *r) {
    if (!r->stdout) {
        if (_rbench_stdout == NULL) {
            _rbench_stdout = stdout;
        }
        if (_rbench_stdnull == NULL) {
            _rbench_stdnull = fopen("/dev/null", "wb");
        }
        if (stdout == _rbench_stdout) {
            stdout = _rbench_stdnull;
        } else {
            stdout = _rbench_stdout;
        }
    }
}
void rbench_restore_stdout(rbench_t *r) {
    if (r->stdout)
        return;
    if (_rbench_stdout) {
        stdout = _rbench_stdout;
    }
    if (_rbench_stdnull) {
        fclose(_rbench_stdnull);
        _rbench_stdnull = NULL;
    }
}

rbench_t *rbench_new();

rbench_t *_rbench = NULL;
rbench_function_t *rbf;
rbench_t *rbench() {
    if (_rbench == NULL) {
        _rbench = rbench_new();
    }
    return _rbench;
}

typedef void *(*rbench_call)();
typedef void *(*rbench_call1)(void *);
typedef void *(*rbench_call2)(void *, void *);
typedef void *(*rbench_call3)(void *, void *, void *);

#ifdef __cplusplus
void rbench_add_function(rbench_t *rp, const char *name, const char *group,
                         void (*call)()) {
#else
void rbench_add_function(rbench_t *rp, const char *name, const char *group,
                         void *call) {
#endif
    rbench_function_t *f = &rp->functions[rp->function_count];
    rp->function_count++;
    f->average_execution_time = 0;
    f->total_execution_time = 0;
    f->times_executed = 0;
    f->call = call;
    strcpy(f->name, name);
    strcpy(f->group, group);
}

void rbench_reset_function(rbench_function_t *f) {
    f->average_execution_time = 0;
    f->times_executed = 0;
    f->total_execution_time = 0;
}

void rbench_reset(rbench_t *rp) {
    for (unsigned int i = 0; i < rp->function_count; i++) {
        rbench_reset_function(&rp->functions[i]);
    }
}
int rbench_get_winner_index(rbench_t *r) {
    int winner = 0;
    nsecs_t time = 0;
    for (unsigned int i = 0; i < r->function_count; i++) {
        if (time == 0 || r->functions[i].total_execution_time < time) {
            winner = i;
            time = r->functions[i].total_execution_time;
        }
    }
    return winner;
}
bool rbench_was_last_function(rbench_t *r) {
    for (unsigned int i = 0; i < r->function_count; i++) {
        if (i == r->function_count - 1 && r->current == &r->functions[i])
            return true;
    }
    return false;
}

rbench_function_t *rbench_execute_prepare(rbench_t *r, int findex, long times,
                                          int argc) {
    rbench_toggle_stdout(r);
    if (findex == 0) {
        r->execution_time = 0;
    }
    rbench_function_t *rf = &r->functions[findex];
    rf->argc = argc;
    rbf = rf;
    r->current = rf;
    if (r->show_progress)
        r->progress_bar = rprogressbar_new(0, times, 20, stderr);
    r->times = times;
    // printf("   %s:%s gets executed for %ld times with %d
    // arguments.\n",rf->group, rf->name, times,argc);
    rbench_reset_function(rf);

    return rf;
}
void rbench_execute_finish(rbench_t *r) {
    rbench_toggle_stdout(r);
    if (r->progress_bar) {
        free(r->progress_bar);
        r->progress_bar = NULL;
    }
    r->current->average_execution_time =
        r->current->total_execution_time / r->current->times_executed;
    ;
    // printf("   %s:%s finished executing in
    // %s\n",r->current->group,r->current->name,
    // format_time(r->current->total_execution_time));
    // rbench_show_results_function(r->current);
    if (rbench_was_last_function(r)) {
        rbench_restore_stdout(r);
        unsigned int winner_index = rbench_get_winner_index(r);
        r->winner = winner_index + 1;
        if (!r->silent)
            rprintgf(stderr, "Benchmark results:\n");
        nsecs_t total_time = 0;

        for (unsigned int i = 0; i < r->function_count; i++) {
            rbf = &r->functions[i];
            total_time += rbf->total_execution_time;
            bool is_winner = winner_index == i;
            if (is_winner) {
                if (!r->silent)
                    rprintyf(stderr, " > %s:%s:%s\n",
                             format_time(rbf->total_execution_time), rbf->group,
                             rbf->name);
            } else {
                if (!r->silent)
                    rprintbf(stderr, "   %s:%s:%s\n",
                             format_time(rbf->total_execution_time), rbf->group,
                             rbf->name);
            }
        }
        if (!r->silent)
            rprintgf(stderr, "Total execution time: %s\n",
                     format_time(total_time));
    }
    rbench_restore_stdout(r);
    rbf = NULL;
    r->current = NULL;
}
struct rbench_t *rbench_execute(rbench_t *r, long times) {

    for (unsigned int i = 0; i < r->function_count; i++) {

        rbench_function_t *f = rbench_execute_prepare(r, i, times, 0);
        rbench_call c = (rbench_call)f->call;
        nsecs_t start = nsecs();
        f->first = true;
        c();
        f->first = false;
        f->last = false;
        f->times_executed++;
        for (int j = 1; j < times; j++) {
            c();
            f->times_executed++;
            f->last = f->times_executed == r->times - 1;
            if (r->progress_bar) {
                rprogressbar_update(r->progress_bar, f->times_executed);
            }
        }
        f->total_execution_time = nsecs() - start;
        r->execution_time += f->total_execution_time;
        rbench_execute_finish(r);
    }
    return r;
}

struct rbench_t *rbench_execute1(rbench_t *r, long times, void *arg1) {

    for (unsigned int i = 0; i < r->function_count; i++) {
        rbench_function_t *f = rbench_execute_prepare(r, i, times, 1);
        rbench_call1 c = (rbench_call1)f->call;
        nsecs_t start = nsecs();
        f->first = true;
        c(arg1);
        f->first = false;
        f->last = false;
        f->times_executed++;
        for (int j = 1; j < times; j++) {
            c(arg1);
            f->times_executed++;
            f->last = f->times_executed == r->times - 1;
            if (r->progress_bar) {
                rprogressbar_update(r->progress_bar, f->times_executed);
            }
        }
        f->total_execution_time = nsecs() - start;
        r->execution_time += f->total_execution_time;
        rbench_execute_finish(r);
    }
    return r;
}

struct rbench_t *rbench_execute2(rbench_t *r, long times, void *arg1,
                                 void *arg2) {

    for (unsigned int i = 0; i < r->function_count; i++) {
        rbench_function_t *f = rbench_execute_prepare(r, i, times, 2);
        rbench_call2 c = (rbench_call2)f->call;
        nsecs_t start = nsecs();
        f->first = true;
        c(arg1, arg2);
        f->first = false;
        f->last = false;
        f->times_executed++;
        for (int j = 1; j < times; j++) {
            c(arg1, arg2);
            f->times_executed++;
            f->last = f->times_executed == r->times - 1;
            if (r->progress_bar) {
                rprogressbar_update(r->progress_bar, f->times_executed);
            }
        }
        f->total_execution_time = nsecs() - start;
        r->execution_time += f->total_execution_time;
        rbench_execute_finish(r);
    }
    return r;
}

struct rbench_t *rbench_execute3(rbench_t *r, long times, void *arg1,
                                 void *arg2, void *arg3) {

    for (unsigned int i = 0; i < r->function_count; i++) {
        rbench_function_t *f = rbench_execute_prepare(r, i, times, 3);

        rbench_call3 c = (rbench_call3)f->call;
        nsecs_t start = nsecs();
        f->first = true;
        c(arg1, arg2, arg3);
        f->first = false;
        f->last = false;
        f->times_executed++;
        for (int j = 1; j < times; j++) {
            c(arg1, arg2, arg3);
            f->times_executed++;
            f->last = f->times_executed == r->times - 1;
            if (r->progress_bar) {
                rprogressbar_update(r->progress_bar, f->times_executed);
            }
        }
        f->total_execution_time = nsecs() - start;
        rbench_execute_finish(r);
    }
    return r;
}

rbench_t *rbench_new() {

    rbench_t *r = (rbench_t *)malloc(sizeof(rbench_t));
    memset(r, 0, sizeof(rbench_t));
    r->add_function = rbench_add_function;
    r->rbench_reset = rbench_reset;
    r->execute1 = rbench_execute1;
    r->execute2 = rbench_execute2;
    r->execute3 = rbench_execute3;
    r->execute = rbench_execute;
    r->stdout = true;
    r->silent = false;
    r->winner = 0;
    r->show_progress = true;
    return r;
}
void rbench_free(rbench_t *r) { free(r); }

#endif
// END OF RLIB
#endif


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


#ifndef RREX3_H
#define RREX3_H
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
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
        //if(*rrex3->expr &&rrex3->functions[(int)*rrex3->expr] ==
        //rrex3_cmp_literal && !rrex3->inside_brackets &&
        //!rrex3_is_function(*rrex3->expr)){ rrex3_cmp_literal(rrex3);
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
        rrex3->matches = (char **)realloc(rrex3->matches, rrex3->match_capacity *
                                                             sizeof(char *));
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
    //assert(rrex3(rrex,"char pony() { }","\\b\\w+(\\s+\\*+)?\\s+\\w+\\s*\\([^)]*\\)\s*\\{[^{}]*\\}"));

    rrex3_free(rrex);
}
#endif
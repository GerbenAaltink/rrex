#ifndef RREX4_H
#define RREX4_H
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define R4_DEBUG_a

#ifdef R4_DEBUG
#define DEBUG_VALIDATE_FUNCTION                                                \
    printf("DEBUG: %s v(%d) <%s> \"%s\"\n", __func__, r4->valid, r4->expr,     \
           r4->str);
#else
#define DEBUG_VALIDATE_FUNCTION
#endif

struct r4_t;

typedef bool (*r4_function)(struct r4_t *);

typedef struct r4_t {
    bool valid;
    bool in_block;
    bool in_range;
    unsigned int match_count;
    unsigned int validation_count;
    bool (*functions[254])(struct r4_t *);
    bool (*slash_functions[254])(struct r4_t *);
    char *_str;
    char *_expr;
    char *str;
    char *expr;
    char *str_previous;
    char *expr_previous;
    char **matches;
} r4_t;

static void r4_free_matches(r4_t *r) {
    if (!r)
        return;
    if (!r->match_count) {
        return;
    }
    for (unsigned i = 0; i < r->match_count; i++) {
        free(r->matches[i]);
    }
    free(r->matches);
    r->match_count = 0;
    r->matches = NULL;
}

static void r4_free(r4_t *r) {
    if (!r)
        return;
    r4_free_matches(r);
    free(r);
}

static bool r4_validate(r4_t *r4);
static void r4_match_add(r4_t *r4, char *extracted);

static bool r4_validate_literal(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    if (!r4->valid)
        return false;
    if (*r4->str != *r4->expr) {
        r4->valid = false;
    } else {
        r4->str++;
    }
    r4->expr++;
    if (r4->in_range || r4->in_block) {
        return r4->valid;
    }
    return r4_validate(r4);
}
static bool r4_validate_question_mark(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    r4->valid = true;
    r4->expr++;
    return r4_validate(r4);
}

static bool r4_validate_plus(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    if (r4->str == r4->str_previous)
        return false;
    r4->expr++;
    if (r4->valid == false) {
        return false;
    }
    char *str_start = r4->str;
    char *expr_left = r4->expr_previous;
    char *expr_right = r4->expr;
    char *return_expr = NULL;
    if (*expr_right == ')') {
        return_expr = expr_right;
        expr_right++;
    }
    r4->expr = expr_right;
    bool right_valid = r4_validate(r4);
    if (right_valid) {
        if (return_expr) {
            r4->str = str_start;
            r4->expr = return_expr;
        }
        return right_valid;
    }
    r4->str = str_start;
    r4->str_previous = str_start;
    r4->expr = expr_left;
    r4->valid = true;
    return r4_validate(r4);
}

static bool r4_validate_dollar(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    r4->expr++;
    return *r4->str == 0;
}

static bool r4_validate_roof(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    if (r4->str != r4->_str) {
        return false;
    }
    r4->expr++;
    return r4_validate(r4);
}

static bool r4_validate_dot(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    if (*r4->str == 0) {
        return false;
    }
    r4->expr++;
    r4->valid = *r4->str != '\n';
    r4->str++;

    if (r4->in_range || r4->in_block) {
        return r4->valid;
    }
    return r4_validate(r4);
}

static bool r4_validate_asterisk(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    if (r4->str == r4->str_previous)
        return false;
    r4->expr++;
    if (r4->valid == false) {
        r4->valid = true;
        return r4_validate(r4);
    }
    char *str_start = r4->str;
    char *expr_left = r4->expr_previous;
    char *expr_right = r4->expr;
    char *return_expr = NULL;
    if (*expr_right == ')') {
        return_expr = expr_right;
        expr_right++;
    }
    r4->expr = expr_right;
    bool right_valid = r4_validate(r4);
    if (right_valid) {
        if (return_expr) {
            r4->str = str_start;
            r4->expr = return_expr;
        }
        return right_valid;
    }
    r4->str = str_start;
    r4->str_previous = str_start;
    r4->expr = expr_left;
    r4->valid = true;
    return r4_validate(r4);
}

static bool r4_validate_pipe(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    r4->expr++;
    if (r4->valid == true) {
        return true;
    } else {
        r4->valid = true;
    }
    return r4_validate(r4);
}

static bool r4_validate_digit(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    if (!isdigit(*r4->str)) {
        r4->valid = false;
    } else {
        r4->str++;
    }
    r4->expr++;
    if (r4->in_block) {
        return r4->valid;
    }
    if (r4->in_range) {
        return r4->valid;
    }
    return r4_validate(r4);
}
static bool r4_validate_not_digit(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    if (isdigit(*r4->str)) {
        r4->valid = false;
    } else {
        r4->str++;
    }
    r4->expr++;

    if (r4->in_block) {
        return r4->valid;
    }

    if (r4->in_range) {
        return r4->valid;
    }
    return r4_validate(r4);
}
static bool r4_validate_word(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    if (!isalpha(*r4->str)) {
        r4->valid = false;
    } else {
        r4->str++;
    }
    r4->expr++;

    if (r4->in_block) {
        return r4->valid;
    }

    if (r4->in_range) {
        return r4->valid;
    }
    return r4_validate(r4);
}
static bool r4_validate_not_word(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    if (isalpha(*r4->str)) {
        r4->valid = false;
    } else {
        r4->str++;
    }
    r4->expr++;

    if (r4->in_block) {
        return r4->valid;
    }

    if (r4->in_range) {
        return r4->valid;
    }
    return r4_validate(r4);
}

static bool r4_isrange(char *s) {
    if (!isalnum(*s)) {
        return false;
    }
    if (*(s + 1) != '-') {
        return false;
    }
    return isalnum(*(s + 2));
}

static bool r4_validate_block_close(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    r4->in_block = false;
    return r4->valid;
}
static bool r4_validate_block_open(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    if (r4->valid == false) {
        return false;
    }
    char *expr_self = r4->expr;
    r4->expr++;
    bool reversed = *r4->expr == '^';
    if (reversed) {
        r4->expr++;
    }

    bool valid_once = false;
    r4->in_block = true;
    while (*r4->expr != ']') {
        if (r4_isrange(r4->expr)) {
            char s = *r4->expr;
            char e = *(r4->expr + 2);
            r4->expr += 2;
            if (s > e) {
                char tempc = s;
                s = e;
                e = tempc;
            }
            if (*r4->str >= s && *r4->str <= e) {
                if (!reversed) {
                    r4->str++;
                }
                valid_once = true;
                break;
            } else {
                r4->expr++;
            }
        } else if (r4_validate(r4)) {
            valid_once = true;
            if (reversed)
                r4->str--;
            break;
        } else if (*r4->expr == *r4->str) {
            if (!reversed)
                r4->str++;
            valid_once = true;
            r4->expr++;
            break;
        } else {
            r4->expr++;
        }
    }
    char *expr_end = strchr(r4->expr, ']');
    r4->expr = expr_end ? expr_end : r4->expr;
    r4->in_block = false;
    r4->valid = expr_end && (!reversed ? valid_once : !valid_once);
    r4->expr++;
    r4->expr_previous = expr_self;

    if (r4->in_range) {
        return r4->valid;
    }
    return r4_validate(r4);
}

static bool r4_validate_whitespace(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    r4->valid = strchr("\r\t \n", *r4->str) != NULL;
    r4->expr++;
    if (r4->valid) {
        r4->str++;
    }
    if (r4->in_range || r4->in_block) {
        return r4->valid;
    }
    return r4_validate(r4);
}
static bool r4_validate_not_whitespace(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    r4->valid = strchr("\r\t \n", *r4->str) == NULL;
    r4->expr++;
    if (r4->valid) {
        r4->str++;
    }
    if (r4->in_range || r4->in_block) {
        return r4->valid;
    }
    return r4_validate(r4);
}

static bool r4_validate_range(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION;
    if (r4->valid == false) {
        r4->expr++;
        return false;
    }
    char *previous = r4->expr_previous;
    r4->in_range = true;
    r4->expr++;
    unsigned int start = 0;
    while (isdigit(*r4->expr)) {
        start = 10 * start;
        start += *r4->expr - '0';
        r4->expr++;
    }
    if (start != 0)
        start--;

    unsigned int end = 0;
    bool variable_end_range = false;
    if (*r4->expr == ',') {
        r4->expr++;
        if (!isdigit(*r4->expr)) {
            variable_end_range = true;
        }
    }
    while (isdigit(*r4->expr)) {
        end = end * 10;
        end += *r4->expr - '0';
        r4->expr++;
    }
    r4->expr++;

    bool valid = true;
    char *expr_right = r4->expr;
    for (unsigned int i = 0; i < start; i++) {
        r4->expr = previous;
        valid = r4_validate(r4);
        if (!*r4->str)
            break;
        if (!valid) {
            break;
        }
    }
    r4->expr = expr_right;
    r4->in_range = false;
    if (!r4->valid)
        return false;
    return r4_validate(r4);

    for (unsigned int i = start; i < end; i++) {
        r4->expr = previous;
        valid = r4_validate(r4);
        if (!valid) {
            break;
        }
    }

    while (variable_end_range) {
        r4->in_range = false;
        valid = r4_validate(r4);
        r4->in_range = true;
        if (valid) {
            break;
        }
        r4->in_range = true;
        valid = r4_validate(r4);
        r4->in_range = false;
        if (!valid) {
            break;
        }
    }
    r4->valid = valid;

    return r4_validate(r4);
}

static bool r4_validate_group_close(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    r4->in_block = false;
    r4->in_range = false;
    return r4->valid;
}

static bool r4_validate_group_open(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    r4->expr++;

    char *str_extract_start = r4->str;
    bool valid = r4_validate(r4);

    if (!valid || *r4->expr != ')') {
        // this is a valid case if not everything between () matches
        return false;
    }
    char *str_extract_end = r4->str;
    unsigned int extracted_length = str_extract_end - str_extract_start;
    char *str_extracted = (char *)calloc(sizeof(char), extracted_length + 1);
    strncpy(str_extracted, str_extract_start, extracted_length);
    r4_match_add(r4, str_extracted);
    r4->expr++;
    return r4_validate(r4);
}

static bool r4_validate_slash(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    // The handling code for handling slashes is implemented in r4_validate
    return r4_validate(r4);
}

static void r4_match_add(r4_t *r4, char *extracted) {
    r4->matches =
        (char **)realloc(r4->matches, (r4->match_count + 1) * sizeof(char *));
    r4->matches[r4->match_count] = extracted;
    r4->match_count++;
}

void r4_init(r4_t *r4) {
    for (__uint8_t i = 0; i < 254; i++) {
        r4->functions[i] = r4_validate_literal;
        r4->slash_functions[i] = r4_validate_literal;
    }
    r4->valid = true;
    r4->validation_count = 0;
    r4->match_count = 0;
    r4->matches = NULL;
    r4->functions['*'] = r4_validate_asterisk;
    r4->functions['?'] = r4_validate_question_mark;
    r4->functions['+'] = r4_validate_plus;
    r4->functions['$'] = r4_validate_dollar;
    r4->functions['^'] = r4_validate_roof;
    r4->functions['.'] = r4_validate_dot;
    r4->functions['|'] = r4_validate_pipe;
    r4->functions['\\'] = r4_validate_slash;
    r4->functions['['] = r4_validate_block_open;
    r4->functions[']'] = r4_validate_block_close;
    r4->functions['{'] = r4_validate_range;
    r4->functions['('] = r4_validate_group_open;
    r4->functions[')'] = r4_validate_group_close;
    r4->slash_functions['d'] = r4_validate_digit;
    r4->slash_functions['w'] = r4_validate_word;
    r4->slash_functions['D'] = r4_validate_not_digit;
    r4->slash_functions['W'] = r4_validate_not_word;
    r4->slash_functions['s'] = r4_validate_whitespace;
    r4->slash_functions['S'] = r4_validate_not_whitespace;
}

static bool r4_looks_behind(char c) { return strchr("?*+{", c) != NULL; }

r4_t *r4_new() {
    r4_t *r4 = (r4_t *)malloc(sizeof(r4_t));

    r4_init(r4);

    return r4;
}

static bool r4_pipe_next(r4_t *r4) {
    char *expr = r4->expr;
    while (*expr) {
        if (*expr == '|') {
            r4->expr = expr + 1;
            r4->valid = true;
            return true;
        }
        expr++;
    }
    return false;
}

static bool r4_validate(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    if (!r4_looks_behind(*r4->expr)) {
        r4->expr_previous = r4->expr;
    } else if (r4->_str == r4->str) {
        // Regex may not start with a look behind ufnction
        return false;
    }
    r4->validation_count++;
    char c_val = *r4->expr;
    if (c_val == 0)
        return r4->valid;

    if (!r4->valid && !r4_looks_behind(*r4->expr)) {
        if (!r4_pipe_next(r4)) {
            return false;
        }
    }
    r4_function f;
    if (c_val == '\\') {
        r4->expr++;
        c_val = *r4->expr;
        f = r4->slash_functions[(int)c_val];
    } else {
        f = r4->functions[(int)c_val];
    }
    r4->valid = f(r4);
    return r4->valid;
}

static bool r4_search(r4_t *r) {
    bool valid = true;
    char *str_next = r->str;
    while (*r->str) {
        if (!(valid = r4_validate(r))) {
            // Move next until we find a match
            str_next++;
            r->str = str_next;
            r->expr = r->_expr;
            r->valid = true;
        } else {
            break;
        }
    }
    r->valid = valid;
    return r->valid;
}

r4_t *r4(const char *str, const char *expr) {
    r4_t *r = r4_new();
    r->_str = (char *)str;
    r->_expr = (char *)expr;
    r->str = r->_str;
    r->expr = r->_expr;
    r->str_previous = r->_str;
    r->expr_previous = r->expr;
    r->in_block = false;
    r->in_range = false;
    r4_search(r);
    return r;
}

r4_t *r4_next(r4_t *r, char *expr) {
    if (expr) {
        r->_expr = expr;
    }
    r->expr = r->_expr;
    r4_free_matches(r);
    r4_search(r);
    return r;
}

bool r4_match(char *str, char *expr) {
    r4_t *r = r4(str, expr);
    bool result = r->valid;
    printf("%d:(%s)<%s>\n", r->validation_count, r->_str, r->_expr);
    r4_free(r);
    return result;
}
#endif
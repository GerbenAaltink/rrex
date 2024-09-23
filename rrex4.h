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
#define _R4_DEBUG 1
static int _r4_debug = 1;
#else
#define _R4_DEBUG 0
static int _r4_debug = 0;
#endif

static char *_format_function_name(const char *name) {
    static char result[100];
    result[0] = 0;

    char *new_name = (char *)name;
    new_name += 11;
    if (new_name[0] == '_')
        new_name += 1;
    if (strlen(new_name) == 0) {
        return " -";
    }
    strcpy(result, new_name);
    return result;
}

#define DEBUG_VALIDATE_FUNCTION                                                \
    \  
    if (_r4_debug || r4->debug)                                                \
        printf("DEBUG: %s %s <%s> \"%s\"\n", _format_function_name(__func__),  \
               r4->valid ? "valid" : "INVALID", r4->expr, r4->str);

struct r4_t;


void r4_enable_debug() { _r4_debug = true; }
void r4_disable_debug() { _r4_debug = false; }

typedef bool (*r4_function)(struct r4_t *);

typedef struct r4_t {
    bool debug;
    bool valid;
    bool in_block;
    bool in_range;
    unsigned int backtracking;
    unsigned int loop_count;
    unsigned int in_group;
    unsigned int match_count;
    unsigned int validation_count;
    unsigned int start;
    unsigned int end;
    unsigned int length;
    bool (*functions[254])(struct r4_t *);
    bool (*slash_functions[254])(struct r4_t *);
    char *_str;
    char *_expr;
    char * match;
    char *str;
    char *expr;
    char *str_previous;
    char *expr_previous;
    char **matches;
} r4_t;

static bool v4_initiated = false;
typedef bool (*v4_function_map)(r4_t *);
v4_function_map v4_function_map_global[256];
v4_function_map v4_function_map_slash[256];
v4_function_map v4_function_map_block[256];

static void r4_free_matches(r4_t *r) {
    if (!r)
        return;
    if(r->match){
        free(r->match);
        r->match = NULL;
    }
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

static bool r4_backtrack(r4_t *r4);
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
    r4->expr++;
    if (r4->valid == false) {
        return r4_validate(r4);
    }
    char *expr_left = r4->expr_previous;
    char *expr_right = r4->expr;
    char *str = r4->str;
    char *return_expr = NULL;
    if (*expr_right == ')') {
        return_expr = expr_right;
        expr_right++;
    }
    r4->in_block = true;
    r4->expr = expr_left;
    while (r4->valid) {
        if (*expr_right) {
            r4->expr = expr_right;
            r4->in_block = false;
            if (r4_backtrack(r4)) {

                if (return_expr) {
                    r4->str = str;
                    r4->expr = return_expr;
                }
                return r4_validate(r4);
            } else {
                r4->in_block = true;
            }
        }
        r4->valid = true;
        r4->expr = expr_left;
        r4->str = str;
        r4_validate(r4);
        str = r4->str;
    }
    r4->in_block = false;
    r4->valid = true;
    r4->expr = return_expr ? return_expr : expr_right;
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
    r4->expr++;
    if (r4->valid == false) {
        r4->valid = true;
        return r4->valid;
        //return r4_validate(r4);
    }
    char *expr_left = r4->expr_previous;
    char *expr_right = r4->expr;
    char *str = r4->str;
    char *return_expr = NULL;
    if (*expr_right == ')') {
        return_expr = expr_right;
        expr_right++;
    }
    r4->in_block = true;
    r4->expr = expr_left;
    while (r4->valid) {
        if (*expr_right) {
            r4->expr = expr_right;
            r4->in_block = false;
            if (r4_backtrack(r4)) {

                if (return_expr) {
                    r4->str = str;
                    r4->expr = return_expr;
                }
                return r4_validate(r4);
            } else {
                r4->in_block = true;
            }
        }
        r4->valid = true;
        r4->expr = expr_left;
        r4->str = str;
        r4_validate(r4);
        str = r4->str;
    }
    r4->in_block = false;
    r4->valid = true;
    r4->expr = return_expr ? return_expr : expr_right;
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
        r4->valid = true;
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
    return r4->valid;
}

static bool r4_validate_group_open(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    char *expr_previous = r4->expr_previous;
    r4->expr++;
    bool save_match = r4->in_group == 0;
    r4->in_group++;
    char *str_extract_start = r4->str;
    bool valid = r4_validate(r4);

    if (!valid || *r4->expr != ')') {
        // this is a valid case if not everything between () matches
        r4->in_group--;
        if(save_match == false){
            r4->valid = true;
        }
       
        // Not direct return? Not sure
        return r4_validate(r4);
    }
    if (save_match) {
        char *str_extract_end = r4->str;
        unsigned int extracted_length =
            strlen(str_extract_start) - strlen(str_extract_end);
        char *str_extracted =
            (char *)calloc(sizeof(char), extracted_length + 1);
        strncpy(str_extracted, str_extract_start, extracted_length);
        r4_match_add(r4, str_extracted);
    }
    assert(*r4->expr == ')');
    r4->expr++;
    r4->in_group--;
    r4->expr_previous = expr_previous;
    return r4_validate(r4);
}

static bool r4_validate_slash(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    // The handling code for handling slashes is implemented in r4_validate
    r4->expr++;
    r4_function f = v4_function_map_slash[(int)*r4->expr];
    
    return f(r4);
}

static void r4_match_add(r4_t *r4, char *extracted) {
    r4->matches =
        (char **)realloc(r4->matches, (r4->match_count + 1) * sizeof(char *));
    r4->matches[r4->match_count] = extracted;
    r4->match_count++;
}

static void r4_validate_word_boundary_start(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    r4->expr++;
    if (!r4->valid) {
        return r4->valid;
    }
    r4->valid =
        isalpha(*r4->str) && (r4->str == r4->_str || !isalpha(*(r4->str - 1)));
    printf("<<%d>>\n", r4->valid);
    if (r4->in_range || r4->in_block) {
        return r4->valid;
    }
    return r4_validate(r4);
}
static void r4_validate_word_boundary_end(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    r4->expr++;
    if (!r4->valid) {
        return r4->valid;
    }
    r4->valid =
        isalpha(*r4->str) && (*(r4->str + 1) == 0 || !isalpha(*(r4->str + 1)));
    if (r4->in_range || r4->in_block) {
        return r4->valid;
    }
    return r4_validate(r4);
}

static void v4_init_function_maps() {
    if (v4_initiated)
        return;
    v4_initiated = true;
    for (__uint8_t i = 0; i < 255; i++) {
        v4_function_map_global[i] = r4_validate_literal;
        v4_function_map_slash[i] = r4_validate_literal;
        v4_function_map_block[i] = r4_validate_literal;
    }
    v4_function_map_global['*'] = r4_validate_asterisk;
    v4_function_map_global['?'] = r4_validate_question_mark;
    v4_function_map_global['+'] = r4_validate_plus;
    v4_function_map_global['$'] = r4_validate_dollar;
    v4_function_map_global['^'] = r4_validate_roof;
    v4_function_map_global['.'] = r4_validate_dot;
    v4_function_map_global['|'] = r4_validate_pipe;
    v4_function_map_global['\\'] = r4_validate_slash;
    v4_function_map_global['['] = r4_validate_block_open;
    v4_function_map_global[']'] = r4_validate_block_close;
    v4_function_map_global['{'] = r4_validate_range;
    v4_function_map_global['('] = r4_validate_group_open;
    v4_function_map_global[')'] = r4_validate_group_close;
    v4_function_map_slash['b'] = r4_validate_word_boundary_start;
    v4_function_map_slash['B'] = r4_validate_word_boundary_end;
    v4_function_map_slash['d'] = r4_validate_digit;
    v4_function_map_slash['w'] = r4_validate_word;
    v4_function_map_slash['D'] = r4_validate_not_digit;
    v4_function_map_slash['W'] = r4_validate_not_word;
    v4_function_map_slash['s'] = r4_validate_whitespace;
    v4_function_map_slash['S'] = r4_validate_not_whitespace;
    v4_function_map_block['\\'] = r4_validate_slash;

    v4_function_map_block['{'] = r4_validate_range;

    v4_function_map_block[']'] = r4_validate_block_close;
}

void r4_init(r4_t *r4) {
    v4_init_function_maps();
    if (r4 == NULL)
        return;
    r4->debug = _R4_DEBUG;
    r4->valid = true;
    r4->validation_count = 0;
    r4->match_count = 0;
    r4->start = 0;
    r4->end = 0;
    r4->length = 0;
    r4->matches = NULL;
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

static bool r4_backtrack(r4_t *r4){
    if(_r4_debug)
        printf("\033[36mDEBUG: backtrack start (%d)\n",r4->backtracking);
    r4->backtracking++;
    char * str = r4->str;
    char * expr = r4->expr;
    bool result = r4_validate(r4);
    r4->backtracking--;
    if(result == false){
        r4->expr = expr;
        r4->str = str;
    }
    if(_r4_debug)
        printf("DEBUG: backtrack end (%d) result: %d %s\n", r4->backtracking, result, r4->backtracking == 0 ? "\033[0m" : "");
    return result;
}

static bool r4_validate(r4_t *r4) {
    DEBUG_VALIDATE_FUNCTION
    r4->validation_count++;
    char c_val = *r4->expr;
    if (c_val == 0)
    {
        printf("HIEROOO\n");
        return r4->valid;
    }
    if (!r4_looks_behind(c_val)) {
        r4->expr_previous = r4->expr;
    } else if (r4->expr == r4->_expr) {
        // Regex may not start with a look behind ufnction
        printf("HIEROO\n");
        return false;
    }

    if (!r4->valid && !r4_looks_behind(*r4->expr)) {
        if (!r4_pipe_next(r4)) {
            return false;
        }
    }
    r4_function f;
    f = v4_function_map_global[(int)c_val];

    r4->valid = f(r4);
    return r4->valid;
}

char * r4_get_match(r4_t *r) {
    char * match = (char *)malloc(r->length + 1);
    strncpy(match, r->_str + r->start, r->length);
    match[r->length] = 0;
    return match;
}

static bool r4_search(r4_t *r) {
    bool valid = true;
    char *str_next = r->str;
    while (*r->str) {
        if (!(valid = r4_validate(r))) {
            // Move next until we find a match
            if(!r->backtracking){
                r->start++;
            }
            str_next++;
            r->str = str_next;
            r->expr = r->_expr;
            r->valid = true;
        } else {
            /// HIGH DOUBT
            if(!r->backtracking){
            //r->start = 0;
            }
            break;
        }
    }
    r->valid = valid;
    if (r->valid) {
        r->end = strlen(r->_str) - strlen(r->str);
        r->length = r->end - r->start;
        r->match = r4_get_match(r);
    }
    return r->valid;
}

r4_t *r4(const char *str, const char *expr) {
    r4_t *r = r4_new();
    r->_str = (char *)str;
    r->_expr = (char *)expr;
    r->match = NULL;
    r->str = r->_str;
    r->expr = r->_expr;
    r->str_previous = r->_str;
    r->expr_previous = r->expr;
    r->in_block = false;
    r->in_group = 0;
    r->loop_count = 0;
    r->backtracking = 0;
    r->in_range = false;
    r4_search(r);
    return r;
}

r4_t *r4_next(r4_t *r, char *expr) {
    if (expr) {
        r->_expr = expr;
    }
    r->backtracking = 0;
    r->expr = r->_expr;
    r4_free_matches(r);
    r4_search(r);
    return r;
}

bool r4_match(char *str, char *expr) {
    r4_t *r = r4(str, expr);
    bool result = r->valid;
    r4_free(r);
    return result;
}
#endif
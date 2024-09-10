#include <assert.h>
#include <regex.h>
#include <rlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct r4_t;

typedef bool (*r4_function)(struct r4_t *);

bool r4_next_is_function(struct r4_t *);

typedef struct r4_t {
    bool (*functions[254])(struct r4_t *);
    bool (*slash_functions[254])(struct r4_t *);
    char *_str;
    char *_expr;
    char *str;
    char *expr;
    char *str_mem;
    char *expr_mem;
    char *previous;
    bool valid;
    bool in_block;
    bool in_range;
    unsigned int validation_count;
} r4_t;

void r4_free(r4_t *r) {
    free(r->_str);
    free(r->_expr);
    free(r);
}

bool r4_validate(r4_t *r4);

bool r4_validate_literal(r4_t *r4) {
    if (*r4->str != *r4->expr) {
        // r4->str++;
        r4->valid = false;
        ;
    } else {
        r4->str++;
    }
    r4->expr++;
    // if(r4->in_block){
    //     return r4->valid;
    // }
    if(r4->in_range || r4->in_block){
        return r4->valid;
    }
    return r4_validate(r4);
    // return false;
}
bool r4_validate_question_mark(r4_t *r4) {
    r4->valid = true;
    r4->expr++;
    return r4_validate(r4);
}

bool r4_validate_plus(r4_t *r4) {
    if (r4->str == r4->str_mem)
        return false;
    if (r4->valid == false) {
        return false;
    }
    char *previous = r4->previous;
    char *str = r4->str;
    char *expr = r4->expr;
    r4->expr++;
    bool right_valid = r4_validate(r4);
    if (right_valid) {

        return right_valid;
    } else {
        r4->str = str;
        r4->str_mem = str;
        r4->expr = previous;
    }
    r4->valid = true;
    return r4_validate(r4);
}

bool r4_validate_dollar(r4_t *r4) {
    r4->expr++;
    return *r4->str == 0;
}

bool r4_validate_roof(r4_t *r4) {
    if (r4->str != r4->_str) {
        return false;
    }
    r4->expr++;
    return r4_validate(r4);
}

bool r4_validate_dot(r4_t *r4) {
    if (*r4->str == 0) {
        return false;
    }
    r4->expr++;
    r4->valid = *r4->str != '\n';
    r4->str++;

    if(r4->in_range || r4->in_block){
        return r4->valid;
    }
    return r4_validate(r4);
}

bool r4_validate_asterisk(r4_t *r4) {
    char *previous = r4->previous;
    if (r4->str == r4->str_mem)
        return false;
    if (r4->valid == false) {
        r4->valid = true;
        r4->expr++;
        return r4_validate(r4);
    }
    char *str = r4->str;
    char *expr = r4->expr;
    r4->expr++;
    if(*r4->expr == ')'){
        r4->expr++;
    }
    bool right_valid = r4_validate(r4);
    if (right_valid) {
            printf("RIGHT_VALIDG\n");
        return right_valid;
    } else {
        r4->str = str;
        r4->str_mem = str;
        r4->expr = previous;
    }
    r4->valid = true;
    return r4_validate(r4);
}
bool r4_validate_pipe(r4_t *r4) {
    r4->expr++;
    if (r4->valid == true) {
        return true;
        ;
    } else {
        r4->valid = true;
    }
    return r4_validate(r4);
}

bool r4_validate_digit(r4_t *r4) {
    if (!isdigit(*r4->str)) {
        r4->valid = false;
    }
    r4->str++;
    r4->expr++;
    if (r4->in_block) {
        return r4->valid;
    }

    if(r4->in_range){
        return r4->valid;
    }
    return r4_validate(r4);
}
bool r4_validate_not_digit(r4_t *r4) {
    if (isdigit(*r4->str)) {
        r4->valid = false;
    }
    r4->str++;
    r4->expr++;

    if (r4->in_block) {
        return r4->valid;
    }

    if(r4->in_range){
        return r4->valid;
    }
    return r4_validate(r4);
}
bool r4_validate_word(r4_t *r4) {
    if (!isalpha(*r4->str)) {
        r4->valid = false;
    }
    r4->str++;
    r4->expr++;

    if (r4->in_block) {
        return r4->valid;
    }

    if(r4->in_range){
        return r4->valid;
    }
    return r4_validate(r4);
}
bool r4_validate_not_word(r4_t *r4) {
    if (isalpha(*r4->str)) {
        r4->valid = false;
    }
    r4->str++;
    r4->expr++;

    if (r4->in_block) {
        return r4->valid;
    }

    if(r4->in_range){
        return r4->valid;
    }
    return r4_validate(r4);
}

static bool r4_isdigitrange(char *s) {
    if (!isdigit(*s)) {
        return false;
    }
    if (*(s + 1) != '-') {
        return false;
    }
    return isdigit(*(s + 2));
}

static bool r4_isalpharange(char *s) {
    if (!isalpha(*s)) {
        return false;
    }
    if (*(s + 1) != '-') {
        return false;
    }
    return isalpha(*(s + 2));
}

static bool r4_isrange(char *s) {
    return r4_isalpharange(s) || r4_isdigitrange(s);
}

bool r4_validate_block_close(r4_t *r4) { return r4->valid; }
bool r4_validate_block_open(r4_t *r4) {
    if (r4->valid == false) {
        return false;
    }
    char *previous = r4->expr;
    r4->expr++;
    bool reversed = *r4->expr == '^';
    if (reversed) {
        r4->expr++;
    }

    bool valid_once = false;
    char *expr = r4->expr;
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
        }  else if (*r4->expr == '/') {
            if(r4_validate(r4)){
                valid_once = true; 
                if(reversed)
                    r4->str--;
                break;
            }
        }  else  if (*r4->expr == *r4->str) {
            if (!reversed)
                r4->str++;
            valid_once = true;
            break;
        } else {
            r4->expr++;
        }
    }
    char *rex_new_pos = strchr(r4->expr, ']');
    r4->expr = rex_new_pos ? rex_new_pos : r4->expr;
    r4->in_block = false;
    r4->valid = rex_new_pos && (!reversed ? valid_once : !valid_once);
    r4->expr++;
    r4->previous = previous;

    if(r4->in_range){
        return r4->valid;
    }
    return r4_validate(r4);
}

bool r4_validate_whitespace(r4_t *r4){
    r4->valid = strchr("\r\t \n",*r4->str) != NULL;
    r4->expr++;
    if(r4->valid){
        r4->str++;
    }
    return r4->valid;
}
bool r4_validate_not_whitespace(r4_t *r4){
    r4->valid = strchr("\r\t \n",*r4->str) == NULL;
    r4->expr++;
    if(r4->valid){
        r4->str++;
    }
    return r4->valid;
}

bool r4_validate_range(r4_t *r4){
    if(r4->valid == false){
        r4->expr++;
        return false;
    }
    char * previous = r4->previous;
    r4->in_range = true;
    r4->expr++;
    unsigned int start = 0;
    while(isdigit(*r4->expr)){
        start = 10 * start;
        start += *r4->expr - '0';
        r4->expr++;
    }
    if(start != 0)
        start--;
    
    unsigned int end = 0;
    bool variable_end_range = false;
    if(*r4->expr == ','){
        r4->expr++;
        if(!isdigit(*r4->expr)){
            variable_end_range = true;
        }
    }

    while(isdigit(*r4->expr)){
        end = end * 10;
        end += *r4->expr - '0';
        r4->expr++;
    }
   
    
    bool valid = true;
    r4->expr++;
    char * next = r4->expr;
    for(int i = 0; i < start ; i++){
        r4->expr = previous;
      
        valid = r4_validate(r4); 
        printf("%d\n",valid); 
        if(!*r4->str)
            break;
        if(!valid){
            break;
        }
    }
      
    
    for(int i = start; i < end; i++){
        r4->expr = previous;
        valid = r4_validate(r4);
        if(!valid){
            break;
        }
    }
    
    while(variable_end_range){
        r4->in_range = false; 
        valid  = r4_validate(r4);
        r4->in_range = true;
        if(valid){
            break;
        }
        r4->in_range = true;
        valid = r4_validate(r4);
        r4->in_range = false;
        if(!valid){
            break;
        }
    } 
    r4->valid = valid;
     // }

    return r4_validate(r4);
}

bool r4_validate_group_close(r4_t *r4){
    return r4->valid;
}

bool r4_validate_group_open(r4_t *r4){

    r4->expr++;
    char data[3000] = {0};
    char * dp = data; 

    char * expr = r4->expr;
    char * next = r4->expr;
    while(*next != ')'){
        next++;
    }
    next++;
    char * left_str = r4->str;
    char * right_str = r4->str;
    char * group_start = r4->str;
    char * group_end = r4->str;
    while(*r4->expr != ')'){
        r4->expr = next;
        r4->valid = true;
        bool valid_right = r4_validate(r4);
        if(valid_right){
            break;
        }
        r4->str = left_str;
        r4->valid = true;
        r4->expr = expr;
        printf("%s\n",left_str);
        r4->in_block = true;
        r4_validate(r4);
        r4->expr++;
        printf("<%s>\n",r4->str);
        expr = r4->expr;
        left_str = r4->str;
    }
    group_end = r4->str;
    char * new_str = (char *)malloc(strlen(r4->_str) + 1);
    strcpy(new_str,group_start);
    new_str[group_end - group_start] = 0;
    
    printf("FOUND!! ((%s))\n",new_str);
    r4->expr++;
    group_end = r4->str;
    
    return r4->valid;
}

bool r4_validate_slash(r4_t *r4) { return r4_validate(r4); }

void r4_init(r4_t *r4) {
    for (__uint8_t i = 0; i < 254; i++) {
        r4->functions[i] = r4_validate_literal;
        // r4->slash_functions[i] = r4_cmp_literal;
    }
    r4->valid = true;
    r4->validation_count = 0;
    r4->functions['*'] = r4_validate_asterisk;

    r4->functions['?'] = r4_validate_question_mark;
    r4->functions['+'] = r4_validate_plus;
    r4->functions['$'] = r4_validate_dollar;
    r4->functions['^'] = r4_validate_roof;
    ;
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
    ;
    /*

    r4->functions['|'] = r4_cmp_pipe;
    r4->functions['\\'] = r4_cmp_slash;
    r4->functions['{'] = r4_cmp_range;
    r4->functions['['] = r4_cmp_brackets;
    r4->functions['('] = r4_cmp_parentheses;
    r4->slash_functions['w'] = r4_cmp_w;
    r4->slash_functions['W'] = r4_cmp_w_upper;
    r4->slash_functions['d'] = r4_cmp_d;
    r4->slash_functions['D'] = r4_cmp_d_upper;
    r4->slash_functions['s'] = r4_cmp_whitespace;
    r4->slash_functions['S'] = r4_cmp_whitespace_upper;
    r4->slash_functions['b'] = r4_cmp_word_start_or_end;
    r4->slash_functions['B'] = r4_cmp_word_not_start_or_end;
    r4->match_count = 0;
    r4->match_capacity = 0;
    r4->matches = NULL;
    r4->compiled = NULL;

    r4_reset(r4);*/
}

bool r4_next_is_function(r4_t *r4) {
    char c_next = *(r4->expr + 1);
    ;
    if (c_next == '?' || c_next == '*' || c_next == '+' || c_next == '{') {
        return true;
    }
    return false;
}

r4_t *r4_new() {
    r4_t *r4 = (r4_t *)malloc(sizeof(r4_t));

    r4_init(r4);

    return r4;
}

bool r4_pipe_in_future(r4_t *r4) {
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

bool r4_validate(r4_t *r4) {
    if (!strchr("?+*{", *r4->expr)) {
        r4->previous = r4->expr;
    }
    r4->validation_count++;
    char c_val = *r4->expr;
    if (c_val == 0)
        return r4->valid;

    if (!r4->valid && !r4_next_is_function(r4)) {
        if (!r4_pipe_in_future(r4)) {
            return false;
        }
    }
    if (c_val == '\\') {
        r4->expr++;
        c_val = *r4->expr;
        r4_function sf = r4->slash_functions[c_val];
        r4->valid = sf(r4);
        return r4->valid;
    }
    r4_function f = r4->functions[c_val];
    r4->valid = f(r4);
    return r4->valid;
}

r4_t *r4(char *str, char *expr) {
    r4_t *r = r4_new();
    r->_str = strdup(str);
    r->_expr = strdup(expr);
    r->str = r->_str;
    r->expr = r->_expr;
    r->expr_mem = r->_expr;
    r->str_mem = r->_str;
    r->previous = r->expr;
    r->in_block = false;
    r->in_range = false;
    bool valid = true;
    while (*r->str) {
        if (!(valid = r4_validate(r))) {
            r->str++;
            r->expr = r->_expr;
            r->valid = true;
        } else {
            break;
        }
    }
    r->valid = valid;
    return r;
}

bool test_r4(char *str, char *expr) {
    r4_t *r = r4(str, expr);
    bool result = r->valid;
    printf("%d:(%s)<%s>\n", r->validation_count, r->_str, r->_expr);
    r4_free(r);
    return result;
}

bool bench_r4(unsigned int *times, char *str, char *expr) {
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

bool bench_c(unsigned int times, char *str, char *expr) {
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

int main() {

    unsigned int times = 20;

    assert(bench(times, "suvw",
                 "[abcdefghijklmnopqrstuvw][abcdefghijklmnopqrstuvw]["
                 "abcdefghijklmnopqrstuvw][abcdefghijklmnopqrstuvw]"));
    assert(bench(times, "ponyyy", "^p+o.*yyy$$$$"));
    assert(bench(times, "                   ponyyzd", "p+o.*yyzd$$$$"));
    assert(bench(times, "abc", "def|gek|abc"));
    assert(bench(times, "abc", "def|a?b?c|def"));

    RBENCH(1, {
        assert(test_r4("ponyyy", "^p+o.*yyy$$$$"));
        assert(!test_r4("ponyyy", "p%+o.*yyy$$$$"));
        assert(!test_r4("ponyyyd", "^p+o.*yyz$$$$"));
        assert(test_r4("123", "[0-2][2-2][1-3]$"));
        
        assert(test_r4("abcde","ab(cd)e"));
        assert(test_r4("abcdefg","ab(.*)efg"));
        
        assert(test_r4("ppppony","p*pppony"));
        //assert(test_r4("aa","a{2}$"));
        assert(test_r4("A23", "[0-2A-z][2-2][1-3]$"));
        assert(test_r4("z23", "[0-2A-z][2-2][1-3]$"));
        assert(test_r4("r23", "[0-2Ar][2-2][1-3]$"));
        assert(test_r4("test", "\\w\\w\\w\\w$"));
        assert(!test_r4("test", "\\W\\w\\w\\w$"));
        assert(test_r4("1est", "\\W\\w\\w\\w$"));
        assert(test_r4("1est", "\\d\\w\\w\\w$"));
        assert(test_r4("Aest", "\\D\\w\\w\\w$"));
        assert(test_r4("abc", "[ab]+"));
        assert(!test_r4("abc", "[ab]+$"));
        assert(test_r4("abc", "[abc]+$"));
        assert(!test_r4("a", "[^ba]"));
        assert(!test_r4("a", "[^ab]"));
        assert(test_r4("                     ponyyzd", "p+o.*yyzd$$$$"));
        assert(test_r4("abc", "def|gek|abc"));
        assert(!test_r4("abc", "def|gek|abd"));
        assert(test_r4("abc", "def|abc|def"));
        assert(test_r4("suwv",
                       "[abcdesfghijklmnopqrtuvw][abcdefghijklmnopqrstuvw]["
                       "abcdefghijklmnopqrstuvw][abcdefghijklmnopqrstuvw]"));
    });

    return 0;
}
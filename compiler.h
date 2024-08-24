#include "rlib.h"
#include "rrex.h"
#include <assert.h>

typedef struct rrex_compiler_t {
    int previous_method;
    char *previous_method_start;
    char *bdata;
    char *rdata;
} rrex_compiler_t;

void compile_one(rrex_compiler_t *compiler, char **content, char **compiled,
                 int *indexp);
void rrex_compile(char *content, char *compiled);
int test_compiler();
int convert_bt(size_t i);
char *format_bc(char *code);
void print_bc(char *code);
int test_compile(char *s, char *r);

void rexx_init_compiler(rrex_compiler_t *c, char *rdata, char *bdata) {
    memset(c, 0, sizeof(rrex_compiler_t));
    c->rdata = rdata;
    c->bdata = bdata;
    c->previous_method = 0;
    c->previous_method_start = rdata;
}

typedef enum reg_new_t {
    RN_LITERAL = 1,
    RN_DRANGE = 2,
    RN_ARANGE = 3,
    RN_IGNORE = 4,
    RN_REPEAT = 5,
    RN_FUNCTION = 6,
    RN_DOT,
    RN_ROOF,
    RN_CHOICE_START,
    RN_CHOICE_END,
    RN_WHITESPACE,
    RN_SLASH_CD,
    RN_SLASH_CW,
    RN_PLUS,
    RN_DOLLAR,
    RN_ASTERISK,
    RN_GROUP_START,
    RN_GROUP_END,
    RN_PIPE,
    RN_QUESTION,
    RN_DIGIT,
    RN_ALPHA
} reg_new_t;

int convert_bt(size_t i) {
    char chars[] = "lRRirf.^[]wDW+$*()|?da";
    if (i < strlen(chars) + 1) // Index starts at 1
        return chars[i - 1];
    return i;
}
char *format_bc(char *code) {
    static char result[50000];
    result[0] = 0;
    char value;
    int type = 0;
    for (size_t i = 0; i < strlen(code); i++) {
        type = 0;
        value = code[i];
        if (i && (code[i - 1] == 1)) {
            type = 1; // no byte
        } else if (code[i - 1] == RN_REPEAT) {
            type = 2; // int
        } else {
            type = 0; // byte
        }
        char chunk[10];
        chunk[0] = 0;
        if (type == 0) {
            sprintf(chunk, "%c", convert_bt(value));
        } else if (type == 2)
            sprintf(chunk, "%d", value);
        else
            sprintf(chunk, "(%c)", value);
        strcat(result, chunk);
    }
    return result;
}
void print_bc(char *code) {
    char *human_readable = format_bc(code);
    printf("%s", human_readable);
}

void compile_one(rrex_compiler_t *compiler, char **content, char **compiled,
                 int *indexp) {
    char *r = *content;
    char *c = *compiled;
    int index = *indexp;
    if (*r == '*') {
        compiler->previous_method_start = r;

        c[index] = RN_ASTERISK;
        index++;
        r++;
    } else if (*r == '\\') {
        r++;
        if (*r == 'd') {
            compiler->previous_method_start = r - 1;

            c[index] = RN_DIGIT;
            index++;
            r++;
        } else if (*r == 'w') {
            compiler->previous_method_start = r - 1;

            c[index] = RN_ALPHA;
            index++;
            r++;
        } else if (*r == 's') {
            compiler->previous_method_start = r - 1;

            c[index] = RN_WHITESPACE;
            index++;
            r++;
        } else if (*r == 'D') {
            compiler->previous_method_start = r - 1;

            c[index] = RN_SLASH_CD;
            index++;
            r++;
        } else if (*r == 'W') {
            compiler->previous_method_start = r - 1;

            c[index] = RN_SLASH_CW;
            index++;
            r++;
        } else {
            compiler->previous_method_start = r - 1;
            c[index] = *r;
            index++;
            r++;
        }
    } else if (*r == '$') {
        compiler->previous_method_start = r;
        c[index] = RN_DOLLAR;
        index++;
        r++;
    } else if (*r == '(') {
        char *choice_start = r;
        r++;
        c[index] = RN_GROUP_START;
        index++;
        while (*r != ')') {
            compile_one(compiler, &r, &c, &index);
        }
        compiler->previous_method_start = choice_start;
        c[index] = RN_GROUP_END;
        index++;
        r++;
    } else if (*r == '|') {
        compiler->previous_method_start = r;
        c[index] = RN_PIPE;
        index++;
        r++;
    } else if (*r == '?') {
        r++;
        if (index) {
            char buff_r[1024] = {0};
            char *br = buff_r;
            char *br_start = br;
            char *first_position = compiler->previous_method_start;
            char *rindex = first_position;
            while (rindex != r - 1) {
                *br = *rindex;
                br++;
                *br = 0;
                rindex++;
            }
            br = br_start;
            char buff_b[1024] = {0};
            char *bc = buff_b;
            char *bc_start = buff_b;
            int indexb = 0;
            compile_one(compiler, &br, &bc, &indexb);
            bc = bc_start;
            index -= strlen(bc);
            c[index] = RN_QUESTION;
            index++;
            while (*bc) {
                c[index] = *bc;
                index++;
                bc++;
            }
            compiler->previous_method_start = r - 1;
        }
    } else if (isalpharange(r) || isdigitrange(r)) {
        compiler->previous_method_start = r;

        c[index] = isalpha(*r) ? RN_ARANGE : RN_DRANGE;
        index++;
        c[index] = *r;
        index++;
        r += 2;
        c[index] = *r;
        index++;
        r++;
    } else if (*r == '.') {
        compiler->previous_method_start = r;

        c[index] = RN_DOT;
        index++;
        r++;
    } else if (*r == '^') {
        compiler->previous_method_start = r;

        c[index] = RN_ROOF;
        index++;
        r++;
    } else if (*r == '[') {
        char *choice_start = r;
        r++;
        c[index] = RN_CHOICE_START;
        index++;
        while (*r != ']') {
            compile_one(compiler, &r, &c, &index);
        }
        compiler->previous_method_start = choice_start;
        c[index] = RN_CHOICE_END;
        index++;
        r++;
    } else if (*r == '+') {
        compiler->previous_method_start = r;

        r++;
        c[index] = RN_PLUS;
        index++;
    } else if (*r == '{') {

        r++;
        char *to_repeat = compiler->previous_method_start; // r - 2;
        compiler->previous_method_start = r;
        char *to_repeat_end = r - 2;
        if (isgrouping(to_repeat)) {
            char begin_chr = groupcreverse(*(r - 2));
            while (*to_repeat != begin_chr)
                to_repeat--;
            to_repeat--;
        } else {
            to_repeat--;
        }
        int times = *r - '0';
        r++;
        while (isdigit(*r)) {
            times *= 10;
            times += *r - '0';
            r++;
        }
        for (int i = 0; i < times - 1; i++) {
            char *repeat_index = to_repeat + 1;
            while (repeat_index <= to_repeat_end) {
                compile_one(compiler, &repeat_index, &c, &index);
            }
        }
        r++;
    } else {
        compiler->previous_method_start = r;
        c[index] = *r;
        index++;
        r++;
    }
    c[index] = 0;
    *indexp = index;
    *content = r;
    *compiled = c;
}

void rrex_compile(char *content, char *compiled) {
    rrex_compiler_t compiler;
    rexx_init_compiler(&compiler, content, compiled);

    char *r = content;
    int index = 0;
    while (*r) {
        compile_one(&compiler, &r, &compiled, &index);
    }

    compiled[index] = 0;
}

int test_compile(char *s, char *r) {
    char compiled[50000];
    memset(compiled, 0, sizeof(compiled));
    rrex_compile(s, compiled);
    char *human_format = format_bc(compiled);

    bool result = !strcmp(r, human_format);
    rassert(result);
    return result;
}

void rrex_compiler_tests() {
    rtest_banner("rrex compiler");

    test_compile("\\W\\w\\d\\D", "WadD");
    test_compile("0-9", "R09");
    test_compile("a-z", "Raz");
    test_compile("0-9a-z", "R09Raz");
    test_compile("0-9A-Z", "R09RAZ");
    test_compile("^12^3", "^12^3");
    test_compile("3{1}", "3");
    test_compile("3{2}", "33");
    test_compile("[123]{1}", "[123]");
    test_compile("[123]{2}", "[123][123]");
    test_compile("[123]{3}$", "[123][123][123]$");
    test_compile("(123){3}$", "(123)(123)(123)$");
}
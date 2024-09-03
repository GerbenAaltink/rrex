#include "rlib.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool latleast(char *s, unsigned int l) {
    if (!l)
        return true;
    unsigned int i = 0;
    while (s[i] != 0) {
        i++;
        if (i == l)
            return true;
    }
    return false;
}

bool long_enough(char *s, char *n) {
    while (++(*n)) {
        if (!(++(*s)))
            return true;
    }
    return false;
}

int swith(char *s1, char *s2) {

    while (*s1 == *s2) {
        if (!*s2)
            return true;
        s1++;
        s2++;
    }
    return (*s1 && !*s2);
}

int substr(char *s, int start, int len, char *rdata) {

    for (int i = 0; i < len; i++) {
        if (s[i + start] == 0)
            return false;
        rdata[i] = s[i + start];
    }
    rdata[len] = '\0';
    return strlen(rdata);
}

char groupcreverse(char c) {
    if (c == '{')
        return '}';
    if (c == '}')
        return '{';
    if (c == '(')
        return ')';
    if (c == ')')
        return '(';
    if (c == '[')
        return ']';
    if (c == ']')
        return '[';
    return 0;
}

char isgroupingc(char c) { return groupcreverse(c) != 0; }

bool isgrouping(char *s) { return isgroupingc(s[0]) > 0; }

void test_isgrouping() {
    rassert(isgrouping("{"));
    rassert(isgrouping("{test"));
    rassert(isgrouping("}"));
    rassert(isgrouping("("));
    rassert(isgrouping(")"));
    rassert(isgrouping("["));
    rassert(isgrouping("]"));
    rassert(!isgrouping("!"));
}

int sextract(char *s, char *s_open, char *s_close, char *rdata) {
    unsigned int indent = 0;
    char *s_original = s;
    char *sptr = s;
    int start = -1;
    int pos = 0;
    unsigned int s_open_len = strlen(s_open);
    while (*sptr) {
        pos = sptr - s_original;
        if (start == -1 && !swith(sptr, s_open)) {
            break;
        } else if (start == -1) {
            start = s_open_len;
            indent++;
        } else if (swith(sptr, s_open)) {
            indent++;
        } else if (swith(sptr, s_close)) {
            indent--;
            if (indent == 0) {
                if (substr(s_original, start, pos - start, rdata))
                    return pos;
                else
                    return false;
            }
        }
        sptr++;
    }
    rdata[0] = 0;
    return -1;
}

// expr rex
int exprtok(char *expr, char *ex) {
    if (*expr == '\\' && *(expr + 1) != 0) {
        *ex = *expr;
        *(ex++) = *(expr + 1);
        *(ex + 2) = 0;
        return 2;
    }
    char close_chr = groupcreverse(*expr);
    if (close_chr) {

        int length = 0;
        char open_chr = *expr;

        int indent = 0;
        while (*expr) {
            length++;
            char c = *expr;
            if (c == open_chr) {
                indent++;
            } else if (c == close_chr) {
                indent--;
            }
            *ex = c;

            if (indent == 0) {
                break;
            }

            ex++;
            expr++;
        }
        (*ex++) = 0;
        return indent == 0 ? length : 0;
    } else if (isalpharange(expr) || isdigitrange(expr)) {
        for (int i = 0; i < 3; i++) {
            ex[i] = expr[i];
        }
        ex[3] = 0;
        return 3;
    }
    // printf("%s\n",expr);
    if (*expr) {
        *ex = *expr;
        *(ex++) = 0;
        return 1;
    }
    return 0;
}

void _test_exprtok(char *expr, char *texpected, int len) {
    char tok[4096];
    int toklen = exprtok(expr, tok);
    if (toklen != len) {
        printf("%d:%d\n", toklen, len);
        printf("Assert error length of expected token %s\n", texpected);
        rassert(toklen == len);
    }
    if (strncmp(expr, texpected, len)) {
        printf(
            "Compare error of exprtok with expected token %s does not starts "
            "with %s\n",
            expr, texpected);
        rassert(false);
    }
}
typedef struct rreg_token_t {
    char content[4096];
    int len;
} rreg_token_t;

void test_exprtok() {
    _test_exprtok("[abc]def", "[abc]", 5);
    _test_exprtok("0-9", "0-9", 3);
    _test_exprtok("a-z", "a-z", 3);
    _test_exprtok("A-Z", "A-Z", 3);
    _test_exprtok("\\w", "\\w", 2);
    _test_exprtok("\\", "\\", 1);
    _test_exprtok("a", "a", 1);
}

int sexpand(char *s, char *rdata) {
    int times = 0;
    while (isgrouping(s)) {
        char c_open[2] = {s[0], '\0'};
        char c_close[2] = {groupcreverse(c_open[0]), '\0'};
        if (sextract(s, c_open, c_close, rdata) > 0)
            times++;
        s = rdata;
    }
    return times;
}

void test_sexpand() {

    char rdata[1024];
    rassert(sexpand("[a]", rdata) == 1);
    rassert(!strcmp(rdata, "a"));
    rassert(sexpand("(a)", rdata) == 1);
    rassert(!strcmp(rdata, "a"));
    rassert(sexpand("[a)", rdata) == 0);
    rassert(!strcmp(rdata, ""));
    rassert(sexpand("(a]", rdata) == 0);
    rassert(!strcmp(rdata, ""));
    rassert(sexpand("[{(a)}]", rdata) == 3);
    rassert(!strcmp(rdata, "a"));
}

void test_isalpharange() {
    rassert(isalpharange("a-z"));
    rassert(isalpharange("a-a"));
    rassert(isalpharange("z-z"));
    rassert(isalpharange("a-Z"));
    rassert(isalpharange("Z-a"));
    rassert(isalpharange("Z-Z"));
    rassert(!isalpharange("-a"));
    rassert(!isalpharange("a-"));
    rassert(!isalpharange("a"));
    rassert(!isalpharange("-"));
    rassert(!isalpharange("z"));
    rassert(!isalpharange("-A"));
    rassert(!isalpharange("A-"));
    rassert(!isalpharange("A"));
    rassert(!isalpharange("-"));
    rassert(!isalpharange("Z"));
    rassert(!isalpharange("0-9"));
}

void test_isdigitrange() {
    rassert(isdigitrange("0-9"));
    rassert(isdigitrange("0-0"));
    rassert(isdigitrange("9-9"));
    rassert(!isdigitrange("-0"));
    rassert(!isdigitrange("0-"));
    rassert(!isdigitrange("0"));
    rassert(!isdigitrange("-"));
    rassert(!isdigitrange("9"));
    rassert(!isdigitrange("a-a"));
}

void test_swith() {
    rassert(swith("r", "r"));
    rassert(!swith("r", "re"));
    rassert(swith("retoor", "r"));
    rassert(swith("retoor", "re"));
    rassert(swith("retoor", "retoor"));
    rassert(!swith("retoor", "retoori"));
    rassert(!swith("retoor", "retoorii"));
    rassert(!swith("<value>", "<v>"));
    rassert(!swith("<v>", "<value>"));
}

void test_substr() {
    int r;
    char str[1024];
    r = substr("[-]", 1, 1, str);
    rassert(r == 1);
    rassert(!strcmp(str, "-"));

    r = substr("[-]", 0, 1, str);
    rassert(r == 1);
    rassert(!strcmp(str, "["));

    r = substr("[-]", 2, 1, str);
    rassert(r == 1);
    rassert(!strcmp(str, "]"));

    r = substr("[-]", 0, 3, str);
    rassert(r == 3);
    rassert(!strcmp(str, "[-]"));

    r = substr("[-]", 0, 2, str);
    rassert(r == 2);
    rassert(!strcmp(str, "[-"));
}

void test_sextract() {
    char rdata[1024];
    int pos = 0;
    rassert((pos = sextract("(valid)", "(", ")", rdata)) == 6);
    rassert(!strcmp("valid", rdata));
    rassert((pos = sextract("{valid}", "{", "}", rdata)) == 6);
    rassert(!strcmp("valid", rdata));
    rassert((pos = sextract("{{valid}", "{", "}", rdata)) == -1);
    rassert(!strcmp("", rdata));
    rassert((pos = sextract("{valid}}", "{", "}", rdata)) == 6);
    rassert(!strcmp("valid", rdata));
    rassert((pos = sextract("{{valid}}", "{", "}", rdata)) == 8);
    rassert(!strcmp("{valid}", rdata));
    rassert((pos = sextract("{[({valid}}", "{", "}", rdata)) == 10);
    rassert(!strcmp("[({valid}", rdata));
    rassert((pos = sextract("/*valid*/", "/*", "*/", rdata)) > 0);
    rassert(!strcmp("valid", rdata));
    rassert((pos = sextract("/**valid*/", "/**", "*/", rdata)) > 0);
    rassert(!strcmp("valid", rdata));
    rassert((pos = sextract("/*valid**/", "/*", "**/", rdata)) > 0);
    rassert(!strcmp("valid", rdata));
    rassert((pos = sextract("<test>valid</test>", "<test>", "</test>", rdata)) >
            0);
    rassert(!strcmp("valid", rdata));
    rassert((pos = sextract("<t>valid</t>", "<test>", "</test>", rdata)) == -1);
    rassert(!strcmp("", rdata));
}

void test_latleast() {

    rassert(latleast("", 0));
    rassert(latleast("a", 1));
    rassert(latleast("aa", 1));
    rassert(latleast("aaa", 1));
    rassert(latleast("aa", 2));
    rassert(latleast("aaa", 2));
    rassert(!latleast("a", 2));
}
bool iswhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}
void rrex_functions_test() {
    rtest_banner("rrex functions") test_isalpharange();
    test_isdigitrange();
    test_swith();
    test_substr();
    test_sextract();
    test_isgrouping();
    test_sexpand();
    test_latleast();
    test_exprtok();
}

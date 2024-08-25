#include "compiler.h"
#include "rlib.h"
#include <regex.h>

#define ifwhile(cond, action)                                                  \
    bool _did_doit;                                                            \
    _did_doit = false;                                                         \
    while (cond) {                                                             \
        action                                                                 \
    };                                                                         \
    if (_did_doit)

// bool is_valid = expr ? 1 : 0;
//  repeat:

// bool valid = expr != NULL *expr > 0;

// bool _expr_true = false;
// if(res){
//    _expr_true = true;
//}
// bool ifwhile(bool res){
//
//}
struct rrex_executor_t;

typedef bool (*rrex_function)(struct rrex_executor_t *);

typedef struct rrex_executor_t {
    char *previous_position;
    char previous;
    char *bdata;
    char *_bdata;
    char *sdata;
    char *_sdata;

    long current;
    bool valid;
    rrex_function functions[30];

} rrex_executor_t;

bool rrex_match(char *sdata, char *bdata);
bool rrex_execute_one(rrex_executor_t *t);
bool rrex(char *s, char *r);

bool rrex(char *s, char *r) {
    char b[4096];
    rrex_compile(r, b);
    return rrex_match(s, b);
}

bool rrex_match_sol(rrex_executor_t *executor) {
    executor->previous = RN_ROOF;
    executor->previous_position = executor->bdata;
    bool valid = executor->sdata == executor->_sdata;
    if (valid) {
        executor->bdata++;
    }
    return valid;
}
bool rrex_match_dot(rrex_executor_t *executor) {
    executor->previous = RN_DOT;
    executor->previous_position = executor->bdata;
    if ((executor->sdata)[0] != '\n') {
        executor->sdata++;
        executor->bdata++;
        return true;
    }
    return false;
}

bool rrex_match_digit(rrex_executor_t *executor) {
    if (isdigit(*executor->sdata)) {
        executor->sdata++;
        executor->bdata++;
        return true;
    }
    return false;
}

bool rrex_match_whitespace(rrex_executor_t *executor) {
    if (*executor->sdata == ' ' || *executor->sdata == '\t' ||
        *executor->sdata == '\n' || *executor->sdata == '\r') {
        executor->sdata++;
        executor->bdata++;
        return true;
    }
    return false;
}

bool rrex_match_word(rrex_executor_t *executor) {
    if (isalpha((executor->sdata)[0]) || (executor->sdata)[0] == '_') {
        executor->sdata++;
        executor->bdata++;
        return true;
    }
    return false;
}

bool rrex_match_not_word(rrex_executor_t *executor) {
    if (!(isalpha(*executor->sdata) || *executor->sdata == '_')) {
        executor->sdata++;
        executor->bdata++;
        return true;
    }
    return false;
}

bool rrex_match_not_digit(rrex_executor_t *executor) {
    if (!(isdigit(*executor->sdata))) {
        executor->sdata++;
        executor->bdata++;
        return true;
    }
    return false;
}
bool rrex_match_dollar(rrex_executor_t *executor) {
    if (*executor->sdata == '\0') {
        executor->bdata++;
        return true;
    }
    return false;
}

bool rrex_match_literal(rrex_executor_t *executor) {
    if (*executor->bdata == *executor->sdata) {
        executor->bdata++;
        executor->sdata++;
        return true;
    }
    return false;
    /*
    executor->bdata++;
    if (*executor->bdata == *executor->sdata)
    {
        executor->bdata++;

        executor->sdata++;
        return true;
    }

       // executor->bdata++;
       // executor->sdata++;
    return false;*/
}

bool rrex_match_group(rrex_executor_t *executor) {
    bool v = true;
    executor->bdata++;
    while (v && *executor->bdata != RN_GROUP_END) {
        v = rrex_execute_one(executor);
        if (!v) {
            while (*executor->bdata != RN_GROUP_END) {
                if (*executor->bdata == RN_PIPE) {
                    v = true;
                    executor->bdata++;
                    break;
                }
                executor->bdata++;
            }
        } else if (*executor->bdata == RN_PIPE) {
            break;
        }
    }
    while (*executor->bdata != RN_GROUP_END) {
        executor->bdata++;
    }
    executor->bdata++;
    return v;
}

bool rrex_match_choice(rrex_executor_t *executor) {
    bool v;

    executor->bdata++;
    bool reverse = *executor->bdata == RN_ROOF;
    if (reverse)
        executor->bdata++;

    while (*executor->bdata != RN_CHOICE_END) {
        v = rrex_execute_one(executor);
        if (reverse) {

            v = !v;
            executor->sdata++;
        }
        if (v) {
            break;
        } else {

            if (!reverse)
                executor->bdata++;
        }
    }
    while (*executor->bdata != RN_CHOICE_END) {
        executor->bdata++;
    }

    executor->bdata++;

    return v;
}

bool rrex_match_optional(rrex_executor_t *executor) {
    executor->bdata++;
    char *optional_start = executor->bdata;
    bool v = rrex_execute_one(executor);
    if (!v) {
        executor->bdata = optional_start;
        char closer = 0;
        if (*executor->bdata == RN_CHOICE_START) {
            closer = RN_CHOICE_END;
        }
        if (*executor->bdata == RN_GROUP_START) {
            closer = RN_GROUP_END;
        }
        if (closer) {
            while (*executor->bdata != closer) {
                executor->bdata++;
            }
        }
        executor->bdata++;
    }
    return true;
}

bool rrex_match_at_least_one(rrex_executor_t *executor) {
    bool v = true;
    bool once_valid;
    executor->bdata++;
    char *method_position = executor->previous_position;
    char *next = executor->bdata;
    while (v) {
        executor->bdata = method_position;
        v = rrex_execute_one(executor);

        if (v)
            once_valid = true;

        executor->bdata = next;
        bool v_right = rrex_execute_one(executor);
        if (v_right) {
            once_valid = true;
            break;
        }
    }
    return once_valid;
}

bool rrex_match_range(rrex_executor_t *executor) {
    // Go to first parameter and remember
    executor->bdata++;
    char char_start = *executor->bdata;
    // Go to second parameter and remember
    executor->bdata++;
    char char_end = *executor->bdata;
    // Swap parameters if first one is higher than second one
    if (char_start > char_end) {
        char temp = char_end;
        char_end = char_start;
        char_start = temp;
    }
    // Compare if current char in sdata is between parameters
    if (*executor->sdata >= char_start && *executor->sdata <= char_end) {
        executor->bdata++;
        executor->sdata++;
        return true;
    }
    // Set pointer before parameters. Back to R.
    executor->bdata--;
    executor->bdata--;
    return false;
}

bool rrex_match_plus(rrex_executor_t *executor) {
    char *plus_position = executor->bdata;
    char *to_repeat = executor->previous_position;
    // Return value
    bool valid = true;
    bool matched_once = false;
    char *sdata_before_fail;
    while (valid && executor->bdata == plus_position) {
        // Check if EOF is reached
        if (!*executor->sdata) {
            executor->bdata++;
            break;
        }
        executor->bdata = to_repeat;
        sdata_before_fail = executor->sdata;
        valid = rrex_execute_one(executor);
        if (valid) {
            matched_once = true;
        } else {
            executor->sdata = sdata_before_fail;
        }
    }
    // Move pointer to RN_PLUS sign.
    executor->bdata = plus_position;
    if (matched_once) {
        // Move pointer to after RN_PLUS sign.
        executor->bdata++;
    }
    return matched_once;
}

bool rrex_execute_one(rrex_executor_t *executor) {
    bool valid;
    executor->current = *executor->bdata;
    int previous = executor->current;

    char *previous_position = executor->bdata;
    if (executor->current > 31)
        executor->current = RN_LITERAL;
    valid = executor->functions[executor->current](executor);
    // executor->current = *executor->bdata;
    executor->previous = previous;
    executor->previous_position = previous_position;
    return valid;
}

bool rrex_match(char *sdata, char *bdata) {
    rrex_executor_t executor;
    executor.bdata = bdata;
    executor._bdata = bdata;
    executor.sdata = sdata;
    executor._sdata = sdata;
    executor.previous_position = executor.bdata;
    executor.functions[RN_ARANGE] = rrex_match_range;
    executor.functions[RN_CHOICE_START] = rrex_match_choice;
    executor.functions[RN_DOLLAR] = rrex_match_dollar;
    executor.functions[RN_DOT] = rrex_match_dot;
    executor.functions[RN_DRANGE] = rrex_match_range;
    executor.functions[RN_LITERAL] = rrex_match_literal;
    executor.functions[RN_SLASH_CD] = rrex_match_not_digit;
    executor.functions[RN_SLASH_CW] = rrex_match_not_word;
    executor.functions[RN_PLUS] = rrex_match_plus;
    executor.functions[RN_ASTERISK] = rrex_match_at_least_one;
    executor.functions[RN_WHITESPACE] = rrex_match_whitespace;
    executor.functions[RN_GROUP_START] = rrex_match_group;
    executor.functions[RN_QUESTION] = rrex_match_optional;
    executor.functions[RN_ROOF] = rrex_match_sol;
    executor.functions[RN_DIGIT] = rrex_match_digit;
    executor.functions[RN_ALPHA] = rrex_match_word;
    rrex_executor_t *ex = &executor;
    char *s_padding = ex->sdata;
    bool valid = true;
    while (valid && *ex->bdata) {
        valid = rrex_execute_one(&executor);
        if (!valid && *ex->sdata) {
            if (*ex->_bdata == RN_ROOF) {
                break;
            }
            s_padding++;
            ex->sdata = s_padding;
            ex->bdata = ex->_bdata;
            if (*ex->bdata && *ex->sdata)
                valid = true;
        }
    }
    return valid;
}

void rrex_executor_tests() {
    rtest_banner("rrex regular expressions");

    rassert(rrex(" a ", "\\sa\\s"));
    rassert(!rrex("a", "\\s"));
    rassert(rrex("abc", "ab[def]?c"));
    rassert(rrex("abc", "ab(d|e|f)?c"));
    rassert(rrex("1990-01-13",
                 "^(19|20)\\d\\d-(0[1-9]|1[0-2])-(0[1-9]|[12]\\d|3[01])$"));
    rassert(rrex("1990-01-13", "(19|20)\\d\\d-[0?1]\\d-[0123]\\d"));
    //rassert(rrex("1990-1-3", "(19|20)\\d\\d-[0?1]\\d-[0123]\\d"));
    //rassert(rrex("1990-1-3", "(19|20)\\d\\d-[01]?\\d-[0123]\\d"));
    rassert(rrex("1990-13-25", "(19|20)\\d\\d-([01]\\d?||\\d)-([0123]\\d|\\d)$"));
    rassert(!rrex("1990-13-45", "(19|20)\\d\\d-([01]\\d?||\\d)-([0123]\\d|\\d)$"))
    //(19|20)\d\d-[01]?\d-[0123]\d
    rassert(rrex("a", "[zsa]"));
    rassert(rrex("abcdefg", "abcd?efg"));
    rassert(rrex("abcefg", "abcd?efg"));
    rassert(rrex("ce", "(a|b|c|d)e"));
    rassert(rrex("A", "A-Z"));
    rassert(rrex("a", "a-Z"));
    rassert(rrex("abcab", "[abc][acb]{4}$"));
    rassert(rrex("aa", "\\w{2}$"));

    rassert(rrex("a", "[ca]"));
    rassert(rrex("1-4", "1\\-4"));

    rassert(rrex("a", "[ba]"));
    rassert(rrex("5", "4-9"));
    rassert(rrex("4", "4-9"));
    rassert(rrex("9", "4-9"));
    rassert(rrex("123A", "1-41-41-4A"));
    rassert(!rrex("123B", "1-41-41-4A"));
    rassert(!rrex("1", "4-9"));
    rassert(rrex("abca", "[abc][abc][abc]a$"));

    rassert(rrex("abca", "[a-z][abc][abc]a"));
    rassert(rrex("abca", "[\\w][abc][abc]a"));
    rassert(rrex("a5a5g!a", "a0-9a-z\\d\\D\\Wa"));
    rassert(!rrex("1", "\\D"));
    rassert(!rrex("a", "\\W"));
    rassert(!rrex("1", "\\w"));
    rassert(!rrex("a", "\\d"));
    rassert(!rrex("\n", "."));
    rassert(rrex("a", "a$"));
    rassert(rrex("a1ba1ba1b", "[a-z\\db]{3}"));
    rassert(rrex("abbc", "a{1}[a-z]{2}c{1}"));
    rassert(rrex("aA", "[a-zA-Z]{2}"));

    rassert(!rrex("123", "\\d+a"));
    rassert(rrex("123a", "[123]+a"));
    rassert(rrex("123", "[123]+"));
    rassert(!rrex("123b", "[123]+a"));
    // rassert(!rrex("123", "[123]+b")); NOT READY YET

    rassert(rrex("abababa", "^(ab)+a$"));
    rassert(rrex("abababc", "^(ab)+c$"));
    rassert(!rrex("abababb", "^(ab)+a$"));
    rassert(!rrex("abababa", "^(ab)+b$"));
    rassert(!rrex("abdabdabda", "^(abc)+a$"));
    rassert(!rrex("abababa", "^(abc)+a$"));

    rassert(rrex("123a33", "\\d+a\\d+"));
    rassert(!rrex("123ab", "\\d+$"));

    rassert(rrex("567", "[^1234]"));
    rassert(rrex("400", "[^5]"));
    rassert(!rrex("132213gh", ".*gd"));
    rassert(!rrex("132213gd", ".*gh"));
    rassert(rrex("#include \"test.h\"x", "#include *\"t*es*t.h\"x"));

    //rassert(rrex("#include \"test.h\"x", "#include.*\".*\"x"));
    rassert(!rrex("#include \"test.h\"y", ".*#include.*\".*\"x"));

    rassert(rrex("123test", "^123"));
    rassert(rrex("test123", "123"));
    rassert(!rrex("test123", "^123"));
    rassert(rrex("test123", "123$"));
    rassert(rrex("test123test", "123"));
    rassert(!rrex("test123test", "123$"));
}
